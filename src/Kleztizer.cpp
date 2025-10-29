#include "plugin.hpp"

struct Kleztizer : Module {
	enum ParamId {
		KEY_PARAM,
		MODE_PARAM,
		CHORDBUTTON1_PARAM,
		CHORDBUTTON2_PARAM,
		CHORDBUTTON3_PARAM,
		CHORDBUTTON4_PARAM,
		CHORDBUTTON5_PARAM,
		CHORDBUTTON6_PARAM,
		CHORDBUTTON7_PARAM,
		CHORDVOICING_PARAM,
		INVERSION_PARAM,
		LEADCV1_PARAM,
		LEADOCTAVE1_PARAM,
		LEADCV2_PARAM,
		LEADOCTAVE2_PARAM,
		LEADOFFSET1_PARAM,
		LEADGATE1_PARAM,
		LEADOFFSET2_PARAM,
		LEADGATE2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		KEYCV_INPUT,
		MODECV_INPUT,
		CHORDVOICINGCV_INPUT,
		CHORDINVERSIONCV_INPUT,
		CHORDSELECTCV_INPUT,
		LEADCV1_INPUT,
		LEADOCTAVECV1_INPUT,
		LEADCV2_INPUT,
		LEADOCTAVECV2_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		LEADGATEOUT1_OUTPUT,
		LEADGATEOUT2_OUTPUT,
		PEDALOUT_OUTPUT,
		CHORDROOTOUT_OUTPUT,
		CHORDOUT1_OUTPUT,
		CHORDOUT2_OUTPUT,
		CHORDOUT3_OUTPUT,
		LEADOUT1_OUTPUT,
		LEADOUT2_OUTPUT,
		CHORDBUTTONGATEOUT1_OUTPUT,
		CHORDBUTTONGATEOUT2_OUTPUT,
		CHORDBUTTONGATEOUT3_OUTPUT,
		CHORDBUTTONGATEOUT4_OUTPUT,
		CHORDBUTTONGATEOUT5_OUTPUT,
		CHORDBUTTONGATEOUT6_OUTPUT,
		CHORDBUTTONGATEOUT7_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LEADLED1_LIGHT,
		LEADLED_LIGHT,
		CHORDBUTTON1LED_LIGHT,
		CHORDBUTTON2LED_LIGHT,
		CHORDBUTTON3LED_LIGHT,
		CHORDBUTTON4LED_LIGHT,
		CHORDBUTTON5LED_LIGHT,
		CHORDBUTTON6LED_LIGHT,
		CHORDBUTTON7LED_LIGHT,
		LIGHTS_LEN
	};

	Kleztizer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(KEY_PARAM, 0.f, 11.f, 2.f, "Key", {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"});
		configSwitch(MODE_PARAM, 0.f, 4.f, 0.f, "Mode", {"Freygish", "Mi Sheberach", "Adonai Malakh", "Magein Avot", "Harmonic Minor"});
		configSwitch(CHORDBUTTON1_PARAM, 0.f, 1.f, 0.f, "Chord 1");
		configSwitch(CHORDBUTTON2_PARAM, 0.f, 1.f, 0.f, "Chord 2");
		configSwitch(CHORDBUTTON3_PARAM, 0.f, 1.f, 0.f, "Chord 3");
		configSwitch(CHORDBUTTON4_PARAM, 0.f, 1.f, 0.f, "Chord 4");
		configSwitch(CHORDBUTTON5_PARAM, 0.f, 1.f, 0.f, "Chord 5");
		configSwitch(CHORDBUTTON6_PARAM, 0.f, 1.f, 0.f, "Chord 6");
		configSwitch(CHORDBUTTON7_PARAM, 0.f, 1.f, 0.f, "Chord 7");
		configSwitch(CHORDVOICING_PARAM, 0.f, 3.f, 0.f, "Voicing", {"Close", "Drop 2", "Drop 3", "Open"});
		configSwitch(INVERSION_PARAM, 0.f, 2.f, 0.f, "Inversion", {"Root", "1st", "2nd"}); // only 3-note inversions
		configParam(LEADCV1_PARAM, 0.f, 1.f, 1.f, "Lead 1 CV Amount", "%", 0.f, 100.f);
		configSwitch(LEADOCTAVE1_PARAM, 0.f, 4.f, 2.f, "Lead 1 Octave", {"-2", "-1", "0", "+1", "+2"});
		configParam(LEADCV2_PARAM, 0.f, 1.f, 1.f, "Lead 2 CV Amount", "%", 0.f, 100.f);
		configSwitch(LEADOCTAVE2_PARAM, 0.f, 4.f, 2.f, "Lead 2 Octave", {"-2", "-1", "0", "+1", "+2"});
		configParam(LEADOFFSET1_PARAM, -2.f, 2.f, 0.f, "Lead 1 Note Offset", "v");
		configSwitch(LEADGATE1_PARAM, 0.f, 1.f, 0.f, "Lead 1", {"Gate", "Trig"});
		configParam(LEADOFFSET2_PARAM, -2.f, 2.f, 0.f, "Lead 2 Note Offset", "v");
		configSwitch(LEADGATE2_PARAM, 0.f, 1.f, 0.f, "Lead 2", {"Gate", "Trig"});
		configInput(KEYCV_INPUT, "Key CV");
		configInput(MODECV_INPUT, "Mode CV");
		configInput(CHORDVOICINGCV_INPUT, "Voicing CV");
		configInput(CHORDINVERSIONCV_INPUT, "Inversion CV");
		configInput(CHORDSELECTCV_INPUT, "Chord Select CV");
		configInput(LEADCV1_INPUT, "Lead 1 CV");
		configInput(LEADOCTAVECV1_INPUT, "Lead 1 Octave CV");
		configInput(LEADCV2_INPUT, "Lead 2 CV");
		configInput(LEADOCTAVECV2_INPUT, "Lead 2 Octave CV");
		configOutput(LEADGATEOUT1_OUTPUT, "Lead 1 Gate");
		configOutput(LEADGATEOUT2_OUTPUT, "Lead 2 Gate");
		configOutput(PEDALOUT_OUTPUT, "Pedal");
		configOutput(CHORDROOTOUT_OUTPUT, "Bass");
		configOutput(CHORDOUT1_OUTPUT, "Voice 1");
		configOutput(CHORDOUT2_OUTPUT, "Voice 2");
		configOutput(CHORDOUT3_OUTPUT, "Voice 3");
		configOutput(LEADOUT1_OUTPUT, "Lead 1");
		configOutput(LEADOUT2_OUTPUT, "Lead 2");
		configOutput(CHORDBUTTONGATEOUT1_OUTPUT, "Button 1 Gate");
		configOutput(CHORDBUTTONGATEOUT2_OUTPUT, "Button 2 Gate");
		configOutput(CHORDBUTTONGATEOUT3_OUTPUT, "Button 3 Gate");
		configOutput(CHORDBUTTONGATEOUT4_OUTPUT, "Button 4 Gate");
		configOutput(CHORDBUTTONGATEOUT5_OUTPUT, "Button 5 Gate");
		configOutput(CHORDBUTTONGATEOUT6_OUTPUT, "Button 6 Gate");
		configOutput(CHORDBUTTONGATEOUT7_OUTPUT, "Button 7 Gate");


	}

	bool chordButtonStates[7] = {true};
	dsp::SchmittTrigger chordButtonTriggers[7];
	int chordSelect = 1;

	const int chordButtonParams[7] = {
		CHORDBUTTON1_PARAM,
		CHORDBUTTON2_PARAM,
		CHORDBUTTON3_PARAM,
		CHORDBUTTON4_PARAM,
		CHORDBUTTON5_PARAM,
		CHORDBUTTON6_PARAM,
		CHORDBUTTON7_PARAM
	};

	const int chordButtonLights[7] = {
		CHORDBUTTON1LED_LIGHT,
		CHORDBUTTON2LED_LIGHT,
		CHORDBUTTON3LED_LIGHT,
		CHORDBUTTON4LED_LIGHT,
		CHORDBUTTON5LED_LIGHT,
		CHORDBUTTON6LED_LIGHT,
		CHORDBUTTON7LED_LIGHT
	};

	float tonicVoltage = 0.f;

	const int FREYGISH[7]       = {0, 1, 4, 5, 7, 8, 10};
	const int MI_SHEBERACH[7]   = {0, 2, 3, 6, 7, 9, 10};
	const int ADONAI_MALAKH[7]  = {0, 2, 4, 5, 7, 9, 10};
	const int MAGEIN_AVOT[7]    = {0, 2, 5, 6, 7, 9, 10};
	const int HARMONIC_MINOR[7] = {0, 2, 3, 5, 7, 8, 11};

	// --- Chords tables now have only 3 voices each ---
	const int FREYGISH_CHORDS[7][3] = {
		{0, 4, 7}, {1, 5, 8}, {4, 7, 10},
		{5, 8, 0}, {7, 10, 2}, {5, 0, 2}, {10, 1, 5}
	};
	const int MI_SHEBERACH_CHORDS[7][3] = {
		{0, 3, 7}, {2, 6, 9}, {3, 6, 10},
		{6, 9, 0}, {7, 10, 2}, {9, 0, 4}, {10, 2, 5}
	};
	const int ADONAI_MALAKH_CHORDS[7][3] = {
		{0, 4, 7}, {2, 5, 9}, {4, 8, 11},
		{5, 9, 12}, {7, 11, 14}, {9, 0, 4}, {10, 2, 5}
	};
	const int MAGEIN_AVOT_CHORDS[7][3] = {
		{0, 3, 7}, {2, 5, 9}, {5, 8, 0},
		{6, 9, 0}, {7, 11, 14}, {9, 0, 4}, {10, 2, 5}
	};
	const int HARMONIC_MINOR_CHORDS[7][3] = {
		{0, 3, 7}, {2, 5, 9}, {4, 8, 11},
		{5, 8, 12}, {7, 11, 14}, {9, 0, 4}, {11, 2, 5}
	};

	float leadGateTimer[2] = {0.f, 0.f};
	float leadLedTimer[2]  = {0.f, 0.f};
	int lastQuantizedNote[2] = {-999, -999};

	void process(const ProcessArgs& args) override {
		// --- Chord buttons + CV ---
		for (int i = 0; i < 7; i++) {
			if (chordButtonTriggers[i].process(params[chordButtonParams[i]].getValue())) {
				for (int j = 0; j < 7; j++) chordButtonStates[j] = (i == j);
				chordSelect = i + 1;
			}
		}
		if (inputs[CHORDSELECTCV_INPUT].isConnected()) {
			float cv = clamp(inputs[CHORDSELECTCV_INPUT].getVoltage(), -5.f, 5.f);
			int cvChord = int(roundf(((cv + 5.f) / 10.f) * 6.f)) + 1;
			chordSelect = clamp(cvChord, 1, 7);
			for (int j = 0; j < 7; j++)
				chordButtonStates[j] = (j == chordSelect - 1);
		}
		for (int i = 0; i < 7; i++) {
			lights[chordButtonLights[i]].setBrightnessSmooth(chordButtonStates[i] ? 1.f : 0.f, args.sampleTime);
		}

		for (int i = 0; i < 7; i++) {
 	   outputs[CHORDBUTTONGATEOUT1_OUTPUT + i].setVoltage(chordButtonStates[i] ? 5.f : 0.f);
	   }

		// --- Key & Mode ---
		float keyCV = inputs[KEYCV_INPUT].isConnected() ? inputs[KEYCV_INPUT].getVoltage() : 0.f;
		float keyNorm = params[KEY_PARAM].getValue() / 11.f + keyCV / 10.f;
		int keyIndex = clamp(int(roundf(keyNorm * 11.f)), 0, 11);
		tonicVoltage = keyIndex / 12.f;
		outputs[PEDALOUT_OUTPUT].setVoltage(tonicVoltage - 2.f);

		float modeCV = inputs[MODECV_INPUT].isConnected() ? inputs[MODECV_INPUT].getVoltage() : 0.f;
		float modeNorm = params[MODE_PARAM].getValue() / 4.f + modeCV / 10.f;
		int modeIndex = clamp(int(roundf(modeNorm * 4.f)), 0, 4);

		const int* currentScale = nullptr;
		switch(modeIndex) {
			case 0: currentScale = FREYGISH; break;
			case 1: currentScale = MI_SHEBERACH; break;
			case 2: currentScale = ADONAI_MALAKH; break;
			case 3: currentScale = MAGEIN_AVOT; break;
			case 4: currentScale = HARMONIC_MINOR; break;
		}

		const int (*chordTable)[3] = nullptr;
		switch (modeIndex) {
			case 0: chordTable = FREYGISH_CHORDS; break;
			case 1: chordTable = MI_SHEBERACH_CHORDS; break;
			case 2: chordTable = ADONAI_MALAKH_CHORDS; break;
			case 3: chordTable = MAGEIN_AVOT_CHORDS; break;
			case 4: chordTable = HARMONIC_MINOR_CHORDS; break;
		}
if (chordSelect > 0 && chordTable) {
    int chordIndex = chordSelect - 1;

    // --- Root (always first note of chord table, no inversion/voicing) ---
    float rootVoltage = tonicVoltage + chordTable[chordIndex][0] / 12.f - 2.f; // Two octaves below chord
	// --- Drop an additional octave if Chord 7 is selected ---
	if (chordSelect == 7) {
    rootVoltage -= 1.f; // minus 1V = 1 octave
	}
    outputs[CHORDROOTOUT_OUTPUT].setVoltage(rootVoltage);

    // --- Inversion knob + CV ---
    float invParam = params[INVERSION_PARAM].getValue();
    if (inputs[CHORDINVERSIONCV_INPUT].isConnected())
        invParam += (inputs[CHORDINVERSIONCV_INPUT].getVoltage() / 5.f) * 2.f;
    int inversion = clamp((int)roundf(invParam), 0, 2);

    // --- Voicing knob + CV ---
    float voiceParam = params[CHORDVOICING_PARAM].getValue();
    if (inputs[CHORDVOICINGCV_INPUT].isConnected())
        voiceParam += (inputs[CHORDVOICINGCV_INPUT].getVoltage() / 5.f) * 3.f;
    int voicing = clamp((int)roundf(voiceParam), 0, 3);

    // --- Apply inversion ---
    int chordVoices[3];
    for (int v = 0; v < 3; v++)
        chordVoices[v] = chordTable[chordIndex][(v + inversion) % 3];

    // --- Apply voicing ---
    int finalVoices[3];
    switch (voicing) {
        case 0: for (int v = 0; v < 3; v++) finalVoices[v] = chordVoices[v]; break;
        case 1: finalVoices[0] = chordVoices[1] - 12; finalVoices[1] = chordVoices[0]; finalVoices[2] = chordVoices[2]; break;
        case 2: finalVoices[0] = chordVoices[2] - 12; finalVoices[1] = chordVoices[0]; finalVoices[2] = chordVoices[1]; break;
        case 3: finalVoices[0] = chordVoices[0] - 12; finalVoices[1] = chordVoices[1]; finalVoices[2] = chordVoices[2] - 12; break;
    }

    // --- Output chord voices (mono) ---
    for (int v = 0; v < 3; v++)
        outputs[CHORDOUT1_OUTPUT + v].setVoltage(tonicVoltage + finalVoices[v] / 12.f);

    // --- CHORDOUT1 as polyphonic output: 3 voices only ---
    outputs[CHORDOUT1_OUTPUT].setChannels(3);
    for (int v = 0; v < 3; v++)
        outputs[CHORDOUT1_OUTPUT].setVoltage(tonicVoltage + finalVoices[v] / 12.f, v);

} else {
    outputs[CHORDROOTOUT_OUTPUT].setVoltage(0.f);
    for (int v = 0; v < 3; v++)
        outputs[CHORDOUT1_OUTPUT + v].setVoltage(0.f);

    outputs[CHORDOUT1_OUTPUT].setChannels(3);
    for (int i = 0; i < 3; i++)
        outputs[CHORDOUT1_OUTPUT].setVoltage(0.f, i);
}

		// --- Lead processing (unchanged) ---
		for (int lead = 0; lead < 2; lead++) {
			int LEADCV_INPUT_ID       = (lead == 0) ? LEADCV1_INPUT       : LEADCV2_INPUT;
			int LEADOCTAVECV_INPUT_ID = (lead == 0) ? LEADOCTAVECV1_INPUT : LEADOCTAVECV2_INPUT;
			int LEADCV_PARAM_ID       = (lead == 0) ? LEADCV1_PARAM       : LEADCV2_PARAM;
			int LEADOCTAVE_PARAM_ID   = (lead == 0) ? LEADOCTAVE1_PARAM   : LEADOCTAVE2_PARAM;
			int LEADOFFSET_PARAM_ID   = (lead == 0) ? LEADOFFSET1_PARAM   : LEADOFFSET2_PARAM;
			int LEADGATE_PARAM_ID     = (lead == 0) ? LEADGATE1_PARAM     : LEADGATE2_PARAM;
			int LEADOUT_OUTPUT_ID     = (lead == 0) ? LEADOUT1_OUTPUT     : LEADOUT2_OUTPUT;
			int LEADGATEOUT_OUTPUT_ID = (lead == 0) ? LEADGATEOUT1_OUTPUT : LEADGATEOUT2_OUTPUT;
			int LEADLED_LIGHT_ID      = (lead == 0) ? LEADLED1_LIGHT      : LEADLED_LIGHT;

			float rawCV = inputs[LEADCV_INPUT_ID].isConnected() ? inputs[LEADCV_INPUT_ID].getVoltage() : 0.f;
			float leadCVAtt = rawCV * params[LEADCV_PARAM_ID].getValue();
			float totalCV = leadCVAtt + params[LEADOFFSET_PARAM_ID].getValue();
			float semitoneOffset = totalCV * 12.f;

			int closestNote = 0;
			float minDiff = 9999.f;
			for (int octave = -2; octave <= 2; octave++) {
				for (int i = 0; i < 7; i++) {
					float candidate = currentScale[i] + octave * 12;
					float diff = fabs(semitoneOffset - candidate);
					if (diff < minDiff) {
						minDiff = diff;
						closestNote = candidate;
					}
				}
			}

			float octaveParam = params[LEADOCTAVE_PARAM_ID].getValue();
			if (inputs[LEADOCTAVECV_INPUT_ID].isConnected())
				octaveParam += (inputs[LEADOCTAVECV_INPUT_ID].getVoltage() / 5.f) * 4.f;
			octaveParam = clamp(octaveParam, 0.f, 4.f);
			int octaveShift = (int)roundf(octaveParam) - 2;
			closestNote += octaveShift * 12;

			outputs[LEADOUT_OUTPUT_ID].setVoltage(tonicVoltage + closestNote / 12.f);

			bool isTrigMode = (params[LEADGATE_PARAM_ID].getValue() > 0.5f);
			float trigDur  = 0.005f;
			float gateDur  = 0.5f;

			if (closestNote != lastQuantizedNote[lead]) {
				lastQuantizedNote[lead] = closestNote;
				leadGateTimer[lead] = isTrigMode ? trigDur : gateDur;
				leadLedTimer[lead]  = isTrigMode ? trigDur : gateDur;
			}

			if (leadGateTimer[lead] > 0.f) {
				leadGateTimer[lead] -= args.sampleTime;
				outputs[LEADGATEOUT_OUTPUT_ID].setVoltage(5.f);
			} else {
				outputs[LEADGATEOUT_OUTPUT_ID].setVoltage(0.f);
			}

			if (leadLedTimer[lead] > 0.f) {
				leadLedTimer[lead] -= args.sampleTime;
				lights[LEADLED_LIGHT_ID].setBrightnessSmooth(1.f, args.sampleTime);
			} else {
				lights[LEADLED_LIGHT_ID].setBrightnessSmooth(0.f, args.sampleTime);
			}
		}
	}
};

struct KleztizerWidget : ModuleWidget {
	KleztizerWidget(Kleztizer* module) {
		setModule(module);
	setPanel(createPanel(
		asset::plugin(pluginInstance, "res/panels/Kleztizer.svg")));
        
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(42.667, 12.888)), module, Kleztizer::KEY_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(68.488, 12.888)), module, Kleztizer::MODE_PARAM));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(14.26, 46.862)), module, Kleztizer::CHORDBUTTON1_PARAM));
   	    addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(14.26, 46.862)), module, Kleztizer::CHORDBUTTON1_PARAM,Kleztizer::CHORDBUTTON1LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(26.475, 46.862)), module, Kleztizer::CHORDBUTTON2_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(26.475, 46.862)), module, Kleztizer::CHORDBUTTON2_PARAM,Kleztizer::CHORDBUTTON2LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(38.663, 46.862)), module, Kleztizer::CHORDBUTTON3_PARAM));
   	    addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(38.663, 46.862)), module, Kleztizer::CHORDBUTTON3_PARAM,Kleztizer::CHORDBUTTON3LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(50.851, 46.862)), module, Kleztizer::CHORDBUTTON4_PARAM));
   	    addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(50.851, 46.862)), module, Kleztizer::CHORDBUTTON4_PARAM,Kleztizer::CHORDBUTTON4LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(63.039, 46.862)), module, Kleztizer::CHORDBUTTON5_PARAM));
   	    addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(63.039, 46.862)), module, Kleztizer::CHORDBUTTON5_PARAM,Kleztizer::CHORDBUTTON5LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(75.227, 46.862)), module, Kleztizer::CHORDBUTTON6_PARAM));
   	    addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(75.227, 46.862)), module, Kleztizer::CHORDBUTTON6_PARAM,Kleztizer::CHORDBUTTON6LED_LIGHT));

		addParam(createParamCentered<VCVBezel>(mm2px(Vec(87.415, 46.862)), module, Kleztizer::CHORDBUTTON7_PARAM));
		addParam(createLightParamCentered<VCVLightBezel<>>(mm2px(Vec(87.415, 46.862)), module, Kleztizer::CHORDBUTTON7_PARAM,Kleztizer::CHORDBUTTON7LED_LIGHT));


		addParam(createParamCentered<CKSS>(mm2px(Vec(38.49, 88.284)), module, Kleztizer::LEADGATE1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(89.465, 88.284)), module, Kleztizer::LEADGATE2_PARAM));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.67, 80.836)), module, Kleztizer::LEADCV1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.073, 80.836)), module, Kleztizer::LEADOCTAVE1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(69.644, 80.836)), module, Kleztizer::LEADCV2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(81.047, 80.836)), module, Kleztizer::LEADOCTAVE2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.867, 58.556)), module, Kleztizer::CHORDVOICING_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(81.708, 58.556)), module, Kleztizer::INVERSION_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8.182, 88.284)), module, Kleztizer::LEADOFFSET1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(59.156, 88.284)), module, Kleztizer::LEADOFFSET2_PARAM));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(32.235, 12.888)), module, Kleztizer::KEYCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(57.911, 12.888)), module, Kleztizer::MODECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(20.0, 58.556)), module, Kleztizer::CHORDVOICINGCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(92.502, 58.556)), module, Kleztizer::CHORDINVERSIONCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(51.443, 65.58)), module, Kleztizer::CHORDSELECTCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(18.67, 96.388)), module, Kleztizer::LEADCV1_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.073, 96.388)), module, Kleztizer::LEADOCTAVECV1_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(69.644, 96.388)), module, Kleztizer::LEADCV2_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(81.047, 96.388)), module, Kleztizer::LEADOCTAVECV2_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(46.25, 88.284)), module, Kleztizer::LEADGATEOUT1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(97.224, 88.284)), module, Kleztizer::LEADGATEOUT2_OUTPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(8.867, 117.006)), module, Kleztizer::PEDALOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(25.486, 117.006)), module, Kleztizer::CHORDROOTOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(37.595, 117.006)), module, Kleztizer::CHORDOUT1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(49.704, 117.006)), module, Kleztizer::CHORDOUT2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(61.813, 117.006)), module, Kleztizer::CHORDOUT3_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(79.682, 117.006)), module, Kleztizer::LEADOUT1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.791, 117.006)), module, Kleztizer::LEADOUT2_OUTPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(14.26, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT1_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(26.475, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT2_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(38.663, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT3_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(50.851, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT4_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(63.039, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT5_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(75.227, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT6_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(87.415, 37.16)), module, Kleztizer::CHORDBUTTONGATEOUT7_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(46.38, 80.836)), module, Kleztizer::LEADLED1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(97.354, 80.836)), module, Kleztizer::LEADLED_LIGHT));
	}
};


Model* modelKleztizer = createModel<Kleztizer, KleztizerWidget>("Kleztizer");