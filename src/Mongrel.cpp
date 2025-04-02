#include "plugin.hpp"

struct Mongrel : Module {
	enum ParamId {
		TAIL_PARAM,
		YIP_PARAM,
		SNARL__PARAM,
		GROWL_PARAM,
		YAP_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BANG_CV_INPUT,
		TAIL_CV_INPUT,
		YIP_CV_INPUT,
		GROWL_CV_INPUT,
		SNARL_CV_INPUT,
		YAP_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MONGREL_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_LIGHT,
		LIGHTS_LEN
	};

	float yipPhase = 0.0f;
	float yapPhase = 0.0f;
	float yipFrequency = 20.0f;    // Default frequency for Yip
	float yapFrequency = 20.0f;    // Default frequency for Yap
	float growlAmount = 0.5f;
	float sampleRate = 44100.0f;
	const float MIN_FREQUENCY = 20.0f;
	const float MAX_FREQUENCY = 300.0f;

	float sineWave(float phase) {
		return sinf(phase * 2.0f * M_PI);  // 2π for a full wave
	}

	float mapToRange(float value, float oldMin, float oldMax, float newMin, float newMax) {
		return newMin + (newMax - newMin) * ((value - oldMin) / (oldMax - oldMin));
	}

	Mongrel() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TAIL_PARAM, 0.f, 1.f, 0.f, "Tail", " ms", 60.f, 5.f); // Decay Time find real value 
		configParam(YIP_PARAM, 0.f, 1.f, 0.f, "Yip", " hz", 15.f, 20.f); // Freq 1 find real value 
		configSwitch(SNARL__PARAM, 0.f, 1.f, 0.f, "Waveshape", {"Triangle", "Square"}); // Waveshape
		configParam(GROWL_PARAM, 0.f, 1.f, 0.f, "Ringmod", " %", 0.f, 100.f ); // Ring mod 
		configParam(YAP_PARAM, 0.f, 1.f, 0.f, "Yap", " hz", 15.f, 20.f); // Freq 2 find real vaule 
		configInput(BANG_CV_INPUT, "Bang! Gate");
		configInput(TAIL_CV_INPUT, "Tail CV");
		configInput(YIP_CV_INPUT, "Yip CV");
		configInput(GROWL_CV_INPUT, "Growl CV");
		configInput(SNARL_CV_INPUT, "Snarl Gate");
		configInput(YAP_CV_INPUT, "Yap CV");
		configOutput(MONGREL_OUTPUT, "Mongrel Audio");
	}


float squareWave(float phase) {
    return (phase < 0.5f) ? 1.0f : -1.0f;  
}

void process(const ProcessArgs& args) override {
    sampleRate = args.sampleRate;

    float cvInput = inputs[YIP_CV_INPUT].getVoltage();
    float normalizedCV = (cvInput + 5.0f) / 10.0f;
    float knob1Param = params[YIP_PARAM].getValue() + 0.05;
    float knob1Value = knob1Param + (normalizedCV - 0.5f);
    float controlValue = std::clamp(knob1Value, 0.0f, 1.0f);
    yipFrequency = mapToRange(controlValue, 0.0f, 1.0f, MIN_FREQUENCY, MAX_FREQUENCY);

    float cvInput2 = inputs[YAP_CV_INPUT].getVoltage();
    float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f;
    float knob2Param = params[YAP_PARAM].getValue() + 0.05;
    float knob2Value = knob2Param + (normalizedCV2 - 0.5f);
    float controlValue2 = std::clamp(knob2Value, 0.0f, 1.0f);
    yapFrequency = mapToRange(controlValue2, 0.0f, 1.0f, MIN_FREQUENCY, MAX_FREQUENCY);

    float cvInput3 = inputs[GROWL_CV_INPUT].getVoltage();
    float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f;
    float knob3Param = params[GROWL_PARAM].getValue() + 0.05;
    float knob3Value = knob3Param + (normalizedCV3 - 0.5f);
    float growlAmount = std::clamp(knob3Value, 0.0f, 1.0f);

	float cvInput4 = inputs[TAIL_CV_INPUT].getVoltage();
    float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f;
    float knob4Param = params[TAIL_PARAM].getValue() + 0.05;
    float knob4Value = knob4Param + (normalizedCV4 - 0.5f);
    float tailValue = std::clamp(knob4Value, 0.0f, 1.0f);

    static bool lastBangState = false;
    bool currentBangState = inputs[BANG_CV_INPUT].getVoltage() > 0.5f;
    bool bangRisingEdge = !lastBangState && currentBangState;
    lastBangState = currentBangState;

    float decayTime = 5.0f + (tailValue * 295.0f);
    float decayAlpha = exp(-1.0f / (args.sampleRate * (decayTime / 1000.0f)));

    static bool pulseTriggered = false;
    static float envelopeValue = 0.0f;

    if (bangRisingEdge) {
        pulseTriggered = true;
        envelopeValue = 1.0f;
    }

    if (pulseTriggered) {
        envelopeValue *= decayAlpha;

        if (envelopeValue < 0.01f) {
            envelopeValue = 0.0f;
            pulseTriggered = false;
        }
    } else {
        envelopeValue = 0.0f;
    }

    yipPhase += yipFrequency / sampleRate;
    yapPhase += yapFrequency / sampleRate;
    if (yipPhase >= 1.0f) yipPhase -= 1.0f;
    if (yapPhase >= 1.0f) yapPhase -= 1.0f;

    bool switchState = params[SNARL__PARAM].getValue() > 0.5f;
    bool gateState = inputs[SNARL_CV_INPUT].getVoltage() > 0.5f;

    float yipWave, yapWave;
    if (switchState || gateState) {
        yipWave = (squareWave(yipPhase)) * 0.75f;
        yapWave = (squareWave(yapPhase)) * 0.75f;
    } else {
        yipWave = sineWave(yipPhase);
        yapWave = sineWave(yapPhase);
    }

    yipWave *= 2.5f;
    yapWave *= 2.5f;

    float ringModulatedSignal = (yipWave * yapWave) * 0.75f;
    float drySignal = yipWave + yapWave;
    float modulatedSignal = (1 - growlAmount) * drySignal + growlAmount * ringModulatedSignal;

    modulatedSignal *= envelopeValue;
    modulatedSignal = std::clamp(modulatedSignal, -5.0f, 5.0f);

    outputs[MONGREL_OUTPUT].setVoltage(modulatedSignal);

    float ledBrightness = envelopeValue;
    lights[LED_LIGHT].setBrightness(ledBrightness);
}
};


struct MongrelWidget : ModuleWidget {
	MongrelWidget(Mongrel* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Mongrel.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSSHorizontal>(mm2px(Vec(31.163, 46.546)), module, Mongrel::SNARL__PARAM));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(13.299, 26.368)), module, Mongrel::TAIL_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(49.138, 26.314)), module, Mongrel::YIP_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(13.304, 61.674)), module, Mongrel::GROWL_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(49.116, 61.737)), module, Mongrel::YAP_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.3, 95.604)), module, Mongrel::BANG_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.194, 95.487)), module, Mongrel::TAIL_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.863, 95.487)), module, Mongrel::YIP_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.155, 108.731)), module, Mongrel::GROWL_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.198, 108.957)), module, Mongrel::SNARL_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.822, 108.669)), module, Mongrel::YAP_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.807, 101.759)), module, Mongrel::MONGREL_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.867, 22.525)), module, Mongrel::LED_LIGHT));
	}
};


Model* modelMongrel = createModel<Mongrel, MongrelWidget>("Mongrel");