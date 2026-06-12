#include "plugin.hpp"
#include "../res/wavetables/GMO_tables.hh"
#include "../res/wavetables/banks/banks_manifest.hpp"
#include <atomic>
#include <algorithm>
#include <string>
#ifdef METAMODULE
// Banks are embedded in the plugin's rodata (one shared copy for all instances)
// and indexed in place — no file I/O, no per-instance buffer.
#include "../res/wavetables/banks/gmo_banks_data.hpp"
#else
#include <cstdio> // desktop loads bank .bin files from disk
#endif

using namespace GMOTables; // brings wavetable..wavetable8 and WAVETABLE_SIZE into scope

#define GMO_MAX_WAVETABLES 32

struct GMO : Module {
	enum ParamId {
		SPECIMEN_PARAM,
		SPEED_PARAM,
		LOOP_PARAM,
		HEAD_PARAM,
		TAIL_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BANG_INPUT,
		SPECIMEN_CV_INPUT,
		SPEED_CV_INPUT,
		LOOP_CV_INPUT,
		HEAD_CV_INPUT,
		TAIL_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		GMO_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_LIGHT,
		LIGHTS_LEN
	};

	// --- Bank state ---
	int bankIndex = -1;          // -1 = original firmware bank (compiled-in wavetables)
	int bankNumWavetables = 8;
	float bankSpeedMin = 0.05f;
	float bankSpeedMax = 3.0f;
	std::atomic<int> pendingBankIndex{-2}; // -2 = no change pending; set by UI, applied in process()
#ifndef METAMODULE
	// Desktop only: the selected bank is read from disk into this buffer.
	// On MetaModule the bank data is embedded rodata and indexed directly (no copy).
	int8_t bankData[GMO_MAX_WAVETABLES][WAVETABLE_SIZE] = {};
#endif

	GMO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPECIMEN_PARAM, 1.f, 8.f, 1.f, "Sample Select");
		paramQuantities[SPECIMEN_PARAM]->snapEnabled = true;

		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Pitch", " %", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configParam(HEAD_PARAM, 0.f, 1.f, 0.f, "Start Time", " %", 0.f, 100.f);
		configParam(TAIL_PARAM, 0.f, 1.f, 1.f, "End Time", " %", 0.f, 100.f);
		configInput(BANG_INPUT, "Bang! Gate");
		configInput(SPECIMEN_CV_INPUT, "Specimen CV");
		configInput(SPEED_CV_INPUT, "Speed CV");
		configInput(LOOP_CV_INPUT, "Loop Gate");
		configInput(HEAD_CV_INPUT, "Head CV");
		configInput(TAIL_CV_INPUT, "Tail CV");
		configOutput(GMO_OUTPUT, "GMO Audio");
	}

	float speed = 1.0f;
	float wavetableIndex = 0.0f;
	float outputSignal = 0.0f;
	int8_t sample;
	int headIndex = 0;
	int tailIndex = 0;
	float finalOutput = 0;

	bool previousBangState = false;

	float previousInput = 0.0f;
	float previousOutput = 0.0f;

	bool isPlaying = false;
	bool isLooping = false;
	bool isBanged = false;

	// Select a bank. On MetaModule this is instant (the data is embedded rodata, so we
	// only record which bank + its params). On desktop it reads the .bin into bankData.
	void selectBank(int idx) {
		if (idx < -1 || idx >= GMO_NUM_BANKS) return;

		if (idx == -1) { // firmware bank
			bankIndex = -1;
			bankNumWavetables = 8;
			bankSpeedMin = 0.05f;
			bankSpeedMax = 3.0f;
			return;
		}

#ifndef METAMODULE
		std::string path = asset::plugin(pluginInstance,
			"res/wavetables/banks/" + std::string(GMO_BANKS[idx].filename) + ".bin");
		FILE* f = fopen(path.c_str(), "rb");
		if (!f) return;
		uint8_t numWt = 0;
		fread(&numWt, 1, 1, f);
		int n = std::min((int)numWt, GMO_MAX_WAVETABLES);
		for (int i = 0; i < n; i++)
			fread(bankData[i], WAVETABLE_SIZE, 1, f);
		fclose(f);
		bankNumWavetables = n;
#else
		bankNumWavetables = GMO_BANKS[idx].numWavetables;
#endif
		bankIndex = idx;
		bankSpeedMin = GMO_BANKS[idx].speedMin;
		bankSpeedMax = GMO_BANKS[idx].speedMax;
	}

	// Clamp the Specimen param to the active bank's table count and snap its max.
	void applyBankParams() {
		wavetableIndex = 0.f;
		float specimenVal = params[SPECIMEN_PARAM].getValue();
		if (specimenVal > (float)bankNumWavetables)
			params[SPECIMEN_PARAM].setValue((float)bankNumWavetables);
		paramQuantities[SPECIMEN_PARAM]->maxValue = (float)bankNumWavetables;
	}

	void process(const ProcessArgs &args) override {
		const int GAP = 200; // gap between HEAD and TAIL

		// --- Apply pending bank change (set by context menu / dataFromJson) ---
		// Instant on MetaModule (just selects embedded data); a quick disk read on desktop.
		int pending = pendingBankIndex.exchange(-2);
		if (pending != -2) {
			selectBank(pending);
			applyBankParams();
		}

		//Speed — scaled to the active bank's speed range
		float cvInput = inputs[SPEED_CV_INPUT].getVoltage();
		float normalizedCV = (cvInput + 5.0f) / 10.0f;
		float knob1Param = params[SPEED_PARAM].getValue() + 0.05;
		float knob1Value = knob1Param + (normalizedCV - 0.5f);
		float controlValue = std::clamp(knob1Value, 0.0f, 1.0f);
		speed = controlValue * (bankSpeedMax - bankSpeedMin) + bankSpeedMin;

		//Specimen — dynamically scaled to bankNumWavetables
		float cvInput2 = inputs[SPECIMEN_CV_INPUT].getVoltage();
		float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f;
		float knob2Param = params[SPECIMEN_PARAM].getValue() / (float)bankNumWavetables;
		float knob2Value = knob2Param + (normalizedCV2 - 0.5f);
		float controlValue2 = std::clamp(knob2Value, 0.0f, 1.0f);
		int sampleSelect = std::clamp((int)(controlValue2 * (bankNumWavetables - 1)) + 1, 1, bankNumWavetables);

		//Head
		float cvInput3 = inputs[HEAD_CV_INPUT].getVoltage();
		float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f;
		float knob3Param = params[HEAD_PARAM].getValue();
		float knob3Value = knob3Param + (normalizedCV3 - 0.5f);
		float controlValue3 = std::clamp(knob3Value, 0.0f, 1.0f);

		//Tail
		float cvInput4 = inputs[TAIL_CV_INPUT].getVoltage();
		float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f;
		float knob4Param = params[TAIL_PARAM].getValue();
		float knob4Value = knob4Param + (normalizedCV4 - 0.5f);
		float controlValue4 = std::clamp(knob4Value, 0.0f, 1.0f);

		// LOOP SWITCH
		bool switchState = params[LOOP_PARAM].getValue() > 0.5f;
		bool gateState = inputs[LOOP_CV_INPUT].getVoltage() > 0.5f;

		// One shot states
		bool bangState = inputs[BANG_INPUT].getVoltage() < 0.5f;

		if (switchState || gateState) { // LOOPING MODE
			isLooping = true;
			headIndex = (int)(controlValue3 * WAVETABLE_SIZE);
			tailIndex = (int)(controlValue4 * WAVETABLE_SIZE);
			// Don't let knobs cross
			if (tailIndex <= headIndex + GAP) {
				tailIndex = headIndex + GAP;
			}
			if (headIndex >= tailIndex) {
				headIndex = tailIndex - GAP;
			}

			if (wavetableIndex >= WAVETABLE_SIZE) {
				wavetableIndex -= WAVETABLE_SIZE;
			}
			wavetableIndex = std::clamp(wavetableIndex, (float)headIndex, (float)tailIndex);
			if (wavetableIndex >= tailIndex) {
				wavetableIndex = (float)headIndex; // Reset to HEAD index when TAIL is reached
			}
		}
		else {
			isLooping = false;
		}

		if (bangState < previousBangState) {
			isBanged = true;
			headIndex = (int)(controlValue3 * WAVETABLE_SIZE);
			tailIndex = (int)(controlValue4 * WAVETABLE_SIZE);
			// Don't let knobs cross
			if (tailIndex <= headIndex + GAP) {
				tailIndex = headIndex + GAP;
			}
			if (headIndex >= tailIndex) {
				headIndex = tailIndex - GAP;
			}
			wavetableIndex = headIndex;
		}

		previousBangState = bangState;

		wavetableIndex = std::clamp(wavetableIndex, (float)headIndex, (float)tailIndex);
		wavetableIndex += speed * args.sampleTime * WAVETABLE_SIZE;

		// --- Sample lookup ---
		int idx = std::clamp((int)wavetableIndex, 0, (int)WAVETABLE_SIZE - 1);
		if (bankIndex >= 0) {
#ifdef METAMODULE
			// Index directly into the embedded bank rodata.
			sample = GMO_BANK_DATA[bankIndex][(sampleSelect - 1) * (int)WAVETABLE_SIZE + idx];
#else
			sample = bankData[sampleSelect - 1][idx];
#endif
		} else {
			// Original firmware bank (compiled-in wavetables)
			switch (sampleSelect) {
				case 1:  sample = wavetable[idx];  break;
				case 2:  sample = wavetable2[idx]; break;
				case 3:  sample = wavetable3[idx]; break;
				case 4:  sample = wavetable4[idx]; break;
				case 5:  sample = wavetable5[idx]; break;
				case 6:  sample = wavetable6[idx]; break;
				case 7:  sample = wavetable7[idx]; break;
				default: sample = wavetable8[idx]; break;
			}
		}

		if (wavetableIndex >= tailIndex) {
			isBanged = false;
		}

		outputSignal = (float)sample / 128.0f;
		float boost = outputSignal * 20.0f; // Boost sample up to audible level

		// Highpass
		float RC = 1.0f / (2.0f * M_PI * 200.f); // Last number is cutoff freq
		float alpha = RC / (RC + args.sampleTime);
		float finalOutput = alpha * (previousOutput + boost - previousInput);

		// Update previous input and output for next iteration
		previousInput = boost;
		previousOutput = finalOutput;
		finalOutput = std::clamp(finalOutput, -5.0f, 5.0f);

		if (isLooping || isBanged == true) {
			outputs[GMO_OUTPUT].setVoltage(finalOutput);
			lights[LED_LIGHT].setBrightness(((finalOutput + 5.0f) * 0.1f) - 0.5f);
		}
		else {
			outputs[GMO_OUTPUT].setVoltage(0);
			lights[LED_LIGHT].setBrightness(0);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_object_set_new(rootJ, "bankIndex", json_integer(bankIndex));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* bankJ = json_object_get(rootJ, "bankIndex");
		if (bankJ)
			pendingBankIndex.store(json_integer_value(bankJ)); // applied in process()
	}
};

struct GMOWidget : ModuleWidget {
	GMOWidget(GMO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/GMO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSSHorizontal>(mm2px(Vec(30.519, 46.169)), module, GMO::LOOP_PARAM));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(12.479, 25.756)), module, GMO::SPECIMEN_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(48.715, 25.751)), module, GMO::SPEED_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(12.512, 61.331)), module, GMO::HEAD_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(48.79, 61.43)), module, GMO::TAIL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.566, 95.811)), module, GMO::BANG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.557, 95.904)), module, GMO::SPECIMEN_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.539, 95.895)), module, GMO::SPEED_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.596, 109.169)), module, GMO::LOOP_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.548, 109.124)), module, GMO::HEAD_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.584, 109.146)), module, GMO::TAIL_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.68, 103.12)), module, GMO::GMO_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.309, 21.827)), module, GMO::LED_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		GMO* module = dynamic_cast<GMO*>(this->module);
		if (!module) return;

		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Sample Bank"));

		menu->addChild(createCheckMenuItem("Original Firmware", "",
			[=]() { return module->bankIndex == -1; },
			[=]() { module->pendingBankIndex = -1; }
		));

		// Category order
		const char* categories[] = {
			"Drum Machines", "Percussion", "Artist Series", "Other"
		};

		for (const char* cat : categories) {
			// Check if any banks belong to this category
			bool hasAny = false;
			for (int i = 0; i < GMO_NUM_BANKS; i++) {
				if (std::string(GMO_BANKS[i].category) == cat) { hasAny = true; break; }
			}
			if (!hasAny) continue;

			menu->addChild(createSubmenuItem(cat, "", [=](Menu* submenu) {
				for (int i = 0; i < GMO_NUM_BANKS; i++) {
					if (std::string(GMO_BANKS[i].category) != cat) continue;
					int capturedI = i;
					submenu->addChild(createCheckMenuItem(GMO_BANKS[i].displayName, "",
						[=]() { return module->bankIndex == capturedI; },
						[=]() { module->pendingBankIndex = capturedI; }
					));
				}
			}));
		}
	}
};


Model* modelGMO = createModel<GMO, GMOWidget>("GMO");
