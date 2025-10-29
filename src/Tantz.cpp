#include "plugin.hpp"
#include "../res/sequences/TantzSequences.hpp"

struct Tantz : Module {
	enum ParamId {
		RUN_PARAM,
		STYLE_PARAM,
		KICK_PARAM,
		SNARE_PARAM,
		HHCLOSED_PARAM,
		KICKMUTE_PARAM,
		SNAREMUTE_PARAM,
		HHCLOSEDMUTE_PARAM,
		HHOPEN_PARAM,
		PERC1_PARAM,
		PERC2_PARAM,
		SWING_PARAM,
		PW_PARAM,
		HHOPENMUTE_PARAM,
		PERC1MUTE_PARAM,
		PERC2MUTE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RUNCVIN_INPUT,
		STYLECVIN_INPUT,
		SWINGCVIN_INPUT,
		PWCVIN_INPUT,
		KICKCVIN_INPUT,
		SNARECVIN_INPUT,
		HHCLOSEDCVIN_INPUT,
		CLOCKIN_INPUT,
		HHOPENCVIN_INPUT,
		PERC1CVIN_INPUT,
		PERC2CVIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RESET_OUTPUT,
		KICKOUT_OUTPUT,
		SNAREOUT_OUTPUT,
		HHCLOSEDOUT_OUTPUT,
		HHOPENOUT_OUTPUT,
		PERC1OUT_OUTPUT,
		PERC2OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		KICKLED_LIGHT,
		SNARELED_LIGHT,
		HHCLOSEDLED_LIGHT,
		HHOPENLED_LIGHT,
		PERC1LED_LIGHT,
		PERC2LED_LIGHT,
		RESETLED_LIGHT,
		RUNLED_LIGHT,
		LIGHTS_LEN
	};

	Tantz() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(KICK_PARAM, 0.f, 7.f, 0.f, "Kick", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		configSwitch(SNARE_PARAM, 0.f, 7.f, 0.f, "Snare", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		configSwitch(HHCLOSED_PARAM, 0.f, 7.f, 0.f, "Hi Hat Closed", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		configSwitch(HHOPEN_PARAM, 0.f, 7.f, 0.f, "Hi Hat Open", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		configSwitch(PERC1_PARAM, 0.f, 7.f, 0.f, "Percussion 1", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		configSwitch(PERC2_PARAM, 0.f, 7.f, 0.f, "Percussion 2", {"Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4", "Pattern 5", "Pattern 6", "Pattern 7", "Pattern 8"});
		
		configSwitch(RUN_PARAM, 0.f, 1.f, 0.f, "Run", {"Start", "Stop"});
		configParam(SWING_PARAM, 0.f, 1.f, 0.f, "Swing", "%", 0.f, 100.f);
		configParam(PW_PARAM, 0.f, 1.f, 0.5f, "Pulsewidth", "%", 0.f, 100.f);
		configSwitch(STYLE_PARAM, 0.f, 5.f, 0.f, "Rhythm Style", {"Bulgar #1", "Bulgar #2", "Araber", "Terkisher", "Hora/Zhok", "In Zibn"});
		
		configInput(SWINGCVIN_INPUT, "Swing CV");
		configInput(PWCVIN_INPUT, "Pulsewidth CV");
		configInput(CLOCKIN_INPUT, "Clock");
		configInput(STYLECVIN_INPUT, "Style CV");
		configOutput(KICKOUT_OUTPUT, "Kick Gate");
		configOutput(SNAREOUT_OUTPUT, "Snare Gate");
		configOutput(HHCLOSEDOUT_OUTPUT, "Hi Hat Closed Gate");
		configOutput(HHOPENOUT_OUTPUT, "Hi Hat Open Gate");
		configOutput(PERC1OUT_OUTPUT, "Percussion 1 Gate");
		configOutput(PERC2OUT_OUTPUT, "Percusison 2 Gate");
		configOutput(RESET_OUTPUT, "Reset Gate");

		configSwitch(KICKMUTE_PARAM, 0.f, 1.f, 1.f, "Kick", {"Muted", "Unmuted"});
		configSwitch(SNAREMUTE_PARAM, 0.f, 1.f, 1.f, "Snare", {"Muted", "Unmuted"});
		configSwitch(HHCLOSEDMUTE_PARAM, 0.f, 1.f, 1.f, "Hi Hat Closed", {"Muted", "Unmuted"});
		configSwitch(HHOPENMUTE_PARAM, 0.f, 1.f, 1.f, "Hi Hat Open", {"Muted", "Unmuted"});
		configSwitch(PERC1MUTE_PARAM, 0.f, 1.f, 1.f, "Percussion 1", {"Muted", "Unmuted"});
		configSwitch(PERC2MUTE_PARAM, 0.f, 1.f, 1.f, "Percussion 2", {"Muted", "Unmuted"});

		configInput(RUNCVIN_INPUT, "Run CV");
		configInput(KICKCVIN_INPUT, "Kick CV");
		configInput(SNARECVIN_INPUT, "Snare CV");
		configInput(HHCLOSEDCVIN_INPUT, "Hi Hat Closed CV");
		configInput(HHOPENCVIN_INPUT, "Hi Hat Open CV");
		configInput(PERC1CVIN_INPUT, "Perc 1 CV");
		configInput(PERC2CVIN_INPUT, "Perc 2 CV");
	}

// Rhythm storage
	RhythmData rhythmData;

// --- Drum channel mappings ---
InputId drumCVInputs[RhythmData::NUM_DRUMS] = {
    KICKCVIN_INPUT,
    SNARECVIN_INPUT,
    HHCLOSEDCVIN_INPUT,
    HHOPENCVIN_INPUT,
    PERC1CVIN_INPUT,
    PERC2CVIN_INPUT
};

ParamId drumParamIds[RhythmData::NUM_DRUMS] = {
    KICK_PARAM,
    SNARE_PARAM,
    HHCLOSED_PARAM,
    HHOPEN_PARAM,
    PERC1_PARAM,
    PERC2_PARAM
};

ParamId drumMuteIds[RhythmData::NUM_DRUMS] = {
    KICKMUTE_PARAM,
    SNAREMUTE_PARAM,
    HHCLOSEDMUTE_PARAM,
    HHOPENMUTE_PARAM,
    PERC1MUTE_PARAM,
    PERC2MUTE_PARAM
};

OutputId drumOutputIds[RhythmData::NUM_DRUMS] = {
    KICKOUT_OUTPUT,
    SNAREOUT_OUTPUT,
    HHCLOSEDOUT_OUTPUT,
    HHOPENOUT_OUTPUT,
    PERC1OUT_OUTPUT,
    PERC2OUT_OUTPUT
};

LightId drumLightIds[RhythmData::NUM_DRUMS] = {
    KICKLED_LIGHT,
    SNARELED_LIGHT,
    HHCLOSEDLED_LIGHT,
    HHOPENLED_LIGHT,
    PERC1LED_LIGHT,
    PERC2LED_LIGHT
};

// --- Sequencer state ---
int currentStep = 0;
float lastClock = 0.0f;
float currentStepTime = 0.0f;
float stepInterval = 0.0f;
bool stepPending = false;

float gateTimers[RhythmData::NUM_DRUMS] = {};
float resetGateTimer = 0.0f;

float minGateSec = 0.005f;
float maxGateSec = 0.1f;

bool runState = true;          // start in running state
bool lastRunButton = false;
bool lastRunCV = false;

// Cached params / CVs for change detection
float lastPwKnob = -1.f, lastPwCV = -1.f, lastPwFinal = -1.f;
float lastSwingKnob = -1.f, lastSwingCV = -1.f, lastSwingAmt = -1.f;
float lastStyleKnob = -1.f, lastStyleCV = -1.f;
int   lastStyle = 0; // default to first style

float getGateLength(float pw) {
	return minGateSec + pw * (maxGateSec - minGateSec);
}

void process(const ProcessArgs& args) override {
	float dt = args.sampleTime;

	// --- Run toggle (button or CV) ---
	bool runButton = params[RUN_PARAM].getValue() > 0.5f;
	bool runCV = inputs[RUNCVIN_INPUT].isConnected() && inputs[RUNCVIN_INPUT].getVoltage() > 0.f;

	if ((runButton && !lastRunButton) || (runCV && !lastRunCV))
		runState = !runState;

	lastRunButton = runButton;
	lastRunCV = runCV;

	// Light reflects current run state (starts on)
	lights[RUNLED_LIGHT].setBrightnessSmooth(runState ? 1.f : 0.f, dt);

	// --- If stopped, mute all outputs and lights ---
	if (!runState) {
		std::fill(std::begin(gateTimers), std::end(gateTimers), 0.f);
		resetGateTimer = 0.f;
		for (int d = 0; d < RhythmData::NUM_DRUMS; d++) {
			lights[drumLightIds[d]].setBrightness(0.f);
			outputs[drumOutputIds[d]].setVoltage(0.f);
		}
		lights[RESETLED_LIGHT].setBrightness(0.f);
		outputs[RESET_OUTPUT].setVoltage(0.f);
		return;
	}

	// --- Initialize defaults once at startup ---
	static bool initialized = false;
	if (!initialized) {
		lastPwKnob = params[PW_PARAM].getValue() * 5.f;
		lastPwCV = inputs[PWCVIN_INPUT].isConnected() ? inputs[PWCVIN_INPUT].getVoltage() : 0.f;
		lastPwFinal = std::clamp((lastPwKnob + lastPwCV) / 5.f, 0.f, 1.f);

		lastSwingKnob = params[SWING_PARAM].getValue();
		lastSwingCV = inputs[SWINGCVIN_INPUT].isConnected() ? inputs[SWINGCVIN_INPUT].getVoltage() / 5.f : 0.f;
		lastSwingAmt = std::clamp(lastSwingKnob + lastSwingCV, 0.f, 1.f) * 0.5f;

		lastStyleKnob = params[STYLE_PARAM].getValue();
		lastStyleCV = inputs[STYLECVIN_INPUT].isConnected() ? inputs[STYLECVIN_INPUT].getVoltage() : 0.f;
		lastStyle = (int)round(std::clamp(lastStyleKnob + rescale(lastStyleCV, -5.f, 5.f, -2.5f, 2.5f), 0.f, 5.f));

		// Unmute all drums
		for (int d = 0; d < RhythmData::NUM_DRUMS; d++)
			params[drumMuteIds[d]].setValue(1.f);

		initialized = true;
	}

	// --- Parameter + CV change detection ---

	// Pulsewidth
	float pwKnob = params[PW_PARAM].getValue() * 5.f;
	float pwCV = inputs[PWCVIN_INPUT].isConnected() ? inputs[PWCVIN_INPUT].getVoltage() : 0.f;
	float pwFinal = std::clamp((pwKnob + pwCV) / 5.f, 0.f, 1.f);
	if (pwKnob != lastPwKnob || pwCV != lastPwCV) {
		lastPwKnob = pwKnob;
		lastPwCV = pwCV;
		lastPwFinal = pwFinal;
	}

	// Swing
	float swingKnob = params[SWING_PARAM].getValue();
	float swingCV = inputs[SWINGCVIN_INPUT].isConnected() ? inputs[SWINGCVIN_INPUT].getVoltage() / 5.f : 0.f;
	float swingAmount = std::clamp(swingKnob + swingCV, 0.f, 1.f) * 0.5f;
	if (swingKnob != lastSwingKnob || swingCV != lastSwingCV) {
		lastSwingKnob = swingKnob;
		lastSwingCV = swingCV;
		lastSwingAmt = swingAmount;
	}

	// Style
	float styleKnob = params[STYLE_PARAM].getValue();
	float styleCV = inputs[STYLECVIN_INPUT].isConnected() ? inputs[STYLECVIN_INPUT].getVoltage() : 0.f;
	float styleValue = std::clamp(styleKnob + rescale(styleCV, -5.f, 5.f, -2.5f, 2.5f), 0.f, 5.f);
	int style = (int)round(styleValue);
	int seqLength = rhythmData.sequenceLengths[style];
	if (styleKnob != lastStyleKnob || styleCV != lastStyleCV) {
		lastStyleKnob = styleKnob;
		lastStyleCV = styleCV;
		lastStyle = style;
	}

	// --- Clock edge detect ---
	float clock = inputs[CLOCKIN_INPUT].isConnected() ? inputs[CLOCKIN_INPUT].getVoltage() : 0.f;
	currentStepTime += dt;

	if (clock > 1.0f && lastClock <= 1.0f) {
		stepInterval = currentStepTime;
		currentStepTime = 0.f;
		stepPending = true;
	}
	lastClock = clock;

	// --- Step advance ---
	if (stepPending) {
		float delay = (currentStep % 2 == 1) ? lastSwingAmt * stepInterval : 0.f;

		if (currentStepTime >= delay) {
			for (int d = 0; d < RhythmData::NUM_DRUMS; d++) {
				float knobVal = params[drumParamIds[d]].getValue();
				float cvVal = inputs[drumCVInputs[d]].isConnected()
					? std::clamp(inputs[drumCVInputs[d]].getVoltage() / 5.f, -1.f, 1.f)
					: 0.f;
				float sumVal = std::clamp(knobVal + cvVal * 7.f, 0.f, 7.f);
				int pattern = (int)round(sumVal);

				if (rhythmData.rhythms[lastStyle][d][pattern][currentStep])
					gateTimers[d] = getGateLength(lastPwFinal);
			}

			if (currentStep == seqLength - 1)
				resetGateTimer = getGateLength(lastPwFinal);

			currentStep = (currentStep + 1) % seqLength;
			stepPending = false;
		}
	}

	// --- Outputs ---
	for (int d = 0; d < RhythmData::NUM_DRUMS; d++) {
		bool muted = params[drumMuteIds[d]].getValue() <= 0.5f;

		if (gateTimers[d] > 0.f && !muted) {
			outputs[drumOutputIds[d]].setVoltage(5.f);
			lights[drumLightIds[d]].setBrightnessSmooth(1.f, dt);
			gateTimers[d] -= dt;
		} else {
			outputs[drumOutputIds[d]].setVoltage(0.f);
			lights[drumLightIds[d]].setBrightnessSmooth(0.f, dt);
		}
	}

	// --- Reset output ---
	if (resetGateTimer > 0.f) {
		outputs[RESET_OUTPUT].setVoltage(5.f);
		lights[RESETLED_LIGHT].setBrightnessSmooth(1.f, dt);
		resetGateTimer -= dt;
	} else {
		outputs[RESET_OUTPUT].setVoltage(0.f);
		lights[RESETLED_LIGHT].setBrightnessSmooth(0.f, dt);
	}
}
};


struct TantzWidget : ModuleWidget {
	TantzWidget(Tantz* module) {
		setModule(module);
setPanel(createPanel(
		asset::plugin(pluginInstance, "res/panels/Tantz.svg")));
        
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

   	    addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(75.202, 25.408)), module, Tantz::RUN_PARAM, Tantz::RUNLED_LIGHT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(91.647, 25.408)), module, Tantz::STYLE_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(91.647, 59.179)), module, Tantz::PW_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(75.682, 59.179)), module, Tantz::SWING_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.235, 30.086)), module, Tantz::KICK_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(34.665, 30.088)), module, Tantz::SNARE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.199, 30.091)), module, Tantz::HHCLOSED_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.235, 64.67)), module, Tantz::HHOPEN_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(34.665, 64.67)), module, Tantz::PERC1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.824, 64.523)), module, Tantz::PERC2_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(10.235, 40.625)), module, Tantz::KICKMUTE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(10.235, 75.208)), module, Tantz::HHOPENMUTE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(34.665, 75.208)), module, Tantz::PERC1MUTE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.824, 75.061)), module, Tantz::PERC2MUTE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(34.665, 40.626)), module, Tantz::SNAREMUTE_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.199, 40.63)), module, Tantz::HHCLOSEDMUTE_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(75.202, 37.408)), module, Tantz::RUNCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(91.647, 37.408)), module, Tantz::STYLECVIN_INPUT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(75.682, 71.179)), module, Tantz::SWINGCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(91.647, 71.179)), module, Tantz::PWCVIN_INPUT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.94, 100.566)), module, Tantz::KICKCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(37.074, 100.566)), module, Tantz::SNARECVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(51.161, 100.566)), module, Tantz::HHCLOSEDCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.235, 114.234)), module, Tantz::CLOCKIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(23.94, 114.234)), module, Tantz::HHOPENCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(37.074, 114.234)), module, Tantz::PERC1CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(51.161, 114.234)), module, Tantz::PERC2CVIN_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(10.235, 100.566)), module, Tantz::RESET_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(64.505, 100.566)), module, Tantz::KICKOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(77.64, 100.566)), module, Tantz::SNAREOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.726, 100.566)), module, Tantz::HHCLOSEDOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(64.505, 114.234)), module, Tantz::HHOPENOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(77.64, 114.234)), module, Tantz::PERC1OUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.726, 114.234)), module, Tantz::PERC2OUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.164, 21.565)), module, Tantz::KICKLED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(34.594, 21.566)), module, Tantz::SNARELED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.128, 21.57)), module, Tantz::HHCLOSEDLED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.164, 56.148)), module, Tantz::HHOPENLED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(34.594, 56.148)), module, Tantz::PERC1LED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.754, 56.001)), module, Tantz::PERC2LED_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.235, 93.103)), module, Tantz::RESETLED_LIGHT));
	}
};


Model* modelTantz = createModel<Tantz, TantzWidget>("Tantz");