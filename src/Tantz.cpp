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

	// Sequencer state
	int currentStep = 0;
	float lastClock = 0.0f;
	float gateTimers[RhythmData::NUM_DRUMS] = {};
	float resetGateTimer = 0.0f;

	// Pulsewidth limits (seconds)
	float minGateSec = 0.005f; // 5ms
	float maxGateSec = 0.1f; // 1-0ms

	bool runState = true;        // current on/off state
	bool lastRunButton = false;   // last frame button value
	bool lastRunCV = false;       // last frame CV value

	// Convert PW knob 0–1 to 5–50ms gate length (seconds)
	float getGateLength(float pw) {
		return minGateSec + pw * (maxGateSec - minGateSec);
	}

void process(const ProcessArgs& args) override {
    float dt = args.sampleTime;

    // --- Run toggle (button OR CV) ---
	bool runButton = params[RUN_PARAM].getValue() > 0.5f;
	bool runCV = false;
	if (inputs[RUNCVIN_INPUT].isConnected()) {
	    float v = inputs[RUNCVIN_INPUT].getVoltage();
	    runCV = v > 0.0f;  // positive voltage counts as "pressed"
	}
	if ((runButton && !lastRunButton) || (runCV && !lastRunCV))
	    runState = !runState;
	lastRunButton = runButton;
	lastRunCV = runCV;
	lights[RUNLED_LIGHT].setBrightnessSmooth(runState ? 1.0f : 0.0f, args.sampleTime);


    if (!runState) {
        currentStep = 0;
        for (int d = 0; d < RhythmData::NUM_DRUMS; d++) gateTimers[d] = 0.0f;
        resetGateTimer = 0.0f;
        for (int d = 0; d < RhythmData::NUM_DRUMS; d++)
            lights[drumLightIds[d]].setBrightness(0.0f);
        lights[RESETLED_LIGHT].setBrightness(0.0f);
        return;
    }

    // --- Pulsewidth ---
    float pwKnob = params[PW_PARAM].getValue() * 5.f;
    float pwCV = inputs[PWCVIN_INPUT].isConnected() ? inputs[PWCVIN_INPUT].getVoltage() : 0.0f;
    float pwFinal = clamp((pwKnob + pwCV) / 5.f, 0.0f, 1.0f);

    // --- Swing ---
    float swingKnob = params[SWING_PARAM].getValue();
    float swingCV = inputs[SWINGCVIN_INPUT].isConnected() ? inputs[SWINGCVIN_INPUT].getVoltage() / 5.0f : 0.0f;
    float swingAmount = clamp(swingKnob + swingCV, 0.0f, 1.0f) * 0.5f;

	// --- Style (with CV modulation) ---
	float styleKnob = params[STYLE_PARAM].getValue(); // 0–4
	float styleCV = inputs[STYLECVIN_INPUT].isConnected() ? inputs[STYLECVIN_INPUT].getVoltage() : 0.0f;
	float styleValue = clamp(styleKnob + rescale(styleCV, -5.f, 5.f, -2.5f, 2.5f), 0.f, 5.f);
	int style = (int)round(styleValue);
	int seqLength = rhythmData.sequenceLengths[style];

    // --- Clock handling ---
    float clock = inputs[CLOCKIN_INPUT].isConnected() ? inputs[CLOCKIN_INPUT].getVoltage() : 0.0f;
    static float lastClock = 0.0f;
    static float currentStepTime = 0.0f;
    static float stepInterval = 0.0f;
    static bool stepPending = false;

    currentStepTime += dt;
    if (clock > 1.0f && lastClock <= 1.0f) {
        stepInterval = currentStepTime;
        currentStepTime = 0.0f;
        stepPending = true;
    }
    lastClock = clock;

    if (stepPending) {
        float delay = (currentStep % 2 == 1) ? swingAmount * stepInterval : 0.0f;

        if (currentStepTime >= delay) {
            // --- Step triggers per channel ---
            for (int d = 0; d < RhythmData::NUM_DRUMS; d++) {
                float knobVal = params[drumParamIds[d]].getValue();
                float cvVal = 0.0f;
                if (inputs[drumCVInputs[d]].isConnected()) {
                    cvVal = clamp(inputs[drumCVInputs[d]].getVoltage() / 5.0f, -1.0f, 1.0f);
                }
                float sumVal = clamp(knobVal + cvVal * 7.0f, 0.0f, 7.0f);
                int pattern = (int)round(sumVal);

                if (rhythmData.rhythms[style][d][pattern][currentStep])
                    gateTimers[d] = getGateLength(pwFinal);
            }

            // Reset pulse on final step
            if (currentStep == seqLength - 1)
                resetGateTimer = getGateLength(pwFinal);

            // --- Increment step after triggering ---
            currentStep++;
            if (currentStep >= seqLength)
                currentStep = 0;

            stepPending = false;
        }
    }

    // --- Write outputs and LEDs with final mute ---
    for (int d = 0; d < RhythmData::NUM_DRUMS; d++) {
        bool muted = params[drumMuteIds[d]].getValue() <= 0.5f;

        if (gateTimers[d] > 0.0f && !muted) {
            outputs[drumOutputIds[d]].setVoltage(5.0f);
            lights[drumLightIds[d]].setBrightnessSmooth(1.0f, dt);
            gateTimers[d] -= dt;
        } else {
            outputs[drumOutputIds[d]].setVoltage(0.0f);
            lights[drumLightIds[d]].setBrightnessSmooth(0.0f, dt);
        }
    }

    // --- Reset output ---
    if (resetGateTimer > 0.0f) {
        outputs[RESET_OUTPUT].setVoltage(5.0f);
        lights[RESETLED_LIGHT].setBrightnessSmooth(1.0f, dt);
        resetGateTimer -= dt;
    } else {
        outputs[RESET_OUTPUT].setVoltage(0.0f);
        lights[RESETLED_LIGHT].setBrightnessSmooth(0.0f, dt);
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