#include "plugin.hpp"

struct Moffenmix : Module {
	enum ParamId {
		GAIN_1_PARAM,
		GAIN_2_PARAM,
		GAIN_3_PARAM,
		GAIN_4_PARAM,
		VOLUME_1_PARAM,
		VOLUME_2_PARAM,
		VOLUME_3_PARAM,
		VOLUME_4_PARAM,
		RANGE_1_PARAM,
		RANGE_2_PARAM,
		RANGE_3_PARAM,
		RANGE_4_PARAM,
		MUTE_1_PARAM,
		MUTE_2_PARAM,
		MUTE_3_PARAM,
		MUTE_4_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CH_1_INPUT,
		CH_2_INPUT,
		CH_3_INPUT,
		CH_4_INPUT,
		LINK_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MIX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_1_LIGHT,
		LED_2_LIGHT,
		LED_3_LIGHT,
		LED_4_LIGHT,
		LIGHTS_LEN
	};

	Moffenmix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	
		// Set all Gain knobs to 100% (value 1.0)
		configParam(GAIN_1_PARAM, 0.f, 1.f, 1.f, "Gain 1", " %", 0.f, 100.f);
		configParam(GAIN_2_PARAM, 0.f, 1.f, 1.f, "Gain 2", " %", 0.f, 100.f);
		configParam(GAIN_3_PARAM, 0.f, 1.f, 1.f, "Gain 3", " %", 0.f, 100.f);
		configParam(GAIN_4_PARAM, 0.f, 1.f, 1.f, "Gain 4", " %", 0.f, 100.f);
	
		// Set all Volume knobs to 50% (value 0.5)
		configParam(VOLUME_1_PARAM, 0.f, 1.f, 0.5f, "Volume 1", " %", 0.f, 100.f);
		configParam(VOLUME_2_PARAM, 0.f, 1.f, 0.5f, "Volume 2", " %", 0.f, 100.f);
		configParam(VOLUME_3_PARAM, 0.f, 1.f, 0.5f, "Volume 3", " %", 0.f, 100.f);
		configParam(VOLUME_4_PARAM, 0.f, 1.f, 0.5f, "Volume 4", " %", 0.f, 100.f);
	
		// Set all Range switches to position 0 (value 0)
		configSwitch(RANGE_1_PARAM, 0.f, 2.f, 0.f, "Gain Range 1", {"1x", "5x", "100x"});
		configSwitch(RANGE_2_PARAM, 0.f, 2.f, 0.f, "Gain Range 2", {"1x", "5x", "100x"});
		configSwitch(RANGE_3_PARAM, 0.f, 2.f, 0.f, "Gain Range 3", {"1x", "5x", "100x"});
		configSwitch(RANGE_4_PARAM, 0.f, 2.f, 0.f, "Gain Range 4", {"1x", "5x", "100x"});
	
		// Set all Mute switches to engaged (value 0, muted)
		configSwitch(MUTE_1_PARAM, 0.f, 1.f, 1.f, "Mute 1", {"Mute", "Unmute"});
		configSwitch(MUTE_2_PARAM, 0.f, 1.f, 1.f, "Mute 2", {"Mute", "Unmute"});
		configSwitch(MUTE_3_PARAM, 0.f, 1.f, 1.f, "Mute 3", {"Mute", "Unmute"});
		configSwitch(MUTE_4_PARAM, 0.f, 1.f, 1.f, "Mute 4", {"Mute", "Unmute"});
	
		// Inputs and Outputs
		configInput(CH_1_INPUT, "Ch.1 Audio");
		configInput(CH_2_INPUT, "Ch.2 Audio");
		configInput(CH_3_INPUT, "Ch.3 Audio");
		configInput(CH_4_INPUT, "Ch.4 Audio");
		configInput(LINK_INPUT, "Link Audio");
		configOutput(MIX_OUTPUT, "Mix Audio");
	}
	
	void process(const ProcessArgs& args) override {
		// Get input signals for each channel
		float ch1 = inputs[CH_1_INPUT].getVoltage();
		float ch2 = inputs[CH_2_INPUT].getVoltage();
		float ch3 = inputs[CH_3_INPUT].getVoltage();
		float ch4 = inputs[CH_4_INPUT].getVoltage();
		
		// Get link input at unity gain (no modification)
		float linkInput = inputs[LINK_INPUT].getVoltage();

		// Process each channel (distortion, gain, clipping, etc.)
		float output1 = processChannel(ch1, RANGE_1_PARAM, GAIN_1_PARAM, MUTE_1_PARAM);
		float output2 = processChannel(ch2, RANGE_2_PARAM, GAIN_2_PARAM, MUTE_2_PARAM);
		float output3 = processChannel(ch3, RANGE_3_PARAM, GAIN_3_PARAM, MUTE_3_PARAM);
		float output4 = processChannel(ch4, RANGE_4_PARAM, GAIN_4_PARAM, MUTE_4_PARAM);

		// Apply volume attenuation to each channel's output AFTER distortion and clipping
		output1 = applyVolume(output1, VOLUME_1_PARAM);
		output2 = applyVolume(output2, VOLUME_2_PARAM);
		output3 = applyVolume(output3, VOLUME_3_PARAM);
		output4 = applyVolume(output4, VOLUME_4_PARAM);

		// Apply volume attenuation to the Link input (no distortion applied)
		linkInput = applyVolume(linkInput, 0); // No range/gain for link input

		// Update the LEDs based on the channel output (indicating audio level)
		lights[LED_1_LIGHT].setBrightnessSmooth((fabs(output1) / 5.0f), args.sampleTime); // Normalize to 5V max
		lights[LED_2_LIGHT].setBrightnessSmooth((fabs(output2) / 5.0f), args.sampleTime);  // Normalize to 5V max
		lights[LED_3_LIGHT].setBrightnessSmooth((fabs(output3) / 5.0f), args.sampleTime);  // Normalize to 5V max
		lights[LED_4_LIGHT].setBrightnessSmooth((fabs(output4) / 5.0f), args.sampleTime);  // Normalize to 5V max

		// Combine all channels (after volume attenuation) and include the Link input at unity gain
		float mixedSignal = output1 + output2 + output3 + output4 + linkInput;

		// Apply volume protection to prevent clipping
		mixedSignal = applyVolumeProtection(mixedSignal);

		// Output the final mixed signal
		outputs[MIX_OUTPUT].setVoltage(mixedSignal);
	}

	// Function to process each channel
	float processChannel(float input, int rangeParam, int gainParam, int muteParam) {
		// Check mute status
		if (params[muteParam].getValue() < 0.5f) {
			return 0.0f; // Muted channel
		}

		// Get range setting and apply gain curve
		int range = (int)params[rangeParam].getValue();
		float gain = params[gainParam].getValue();
		float gainMultiplier = getGainMultiplier(range, gain);

		// Apply gain and distortion (soft clipping) before mixing
		float processedSignal = input * gainMultiplier;
		processedSignal = applyDistortion(processedSignal); // Apply clipping/distortion here

		return processedSignal;
	}

	// Function to determine the gain multiplier based on the range setting
	float getGainMultiplier(int range, float gain) {
		switch (range) {
			case 0:
				return gain * 1.0f; // 0 to unity gain (no gain)
			case 1:
				return gain * 5.0f; // Unity to 5x gain
			case 2:
				return gain * 100.0f; // 5x to 100x gain
			default:
				return 1.0f;
		}
	}

	// Soft clipping/distortion function (simple example)
	float applyDistortion(float signal) {
		const float threshold = 5.0f; // Example threshold for clipping
		if (signal > threshold) {
			return threshold + (signal - threshold) / 2.0f; // Distortion beyond threshold
		} else if (signal < -threshold) {
			return -threshold + (signal + threshold) / 2.0f; // Distortion beyond threshold
		}
		return signal; // No clipping
	}

	// Function to apply volume attenuation after distortion/clipping
	float applyVolume(float signal, int volumeParam) {
		// Apply the volume attenuation to the signal after distortion and clipping
		if (volumeParam == 0) {
			return signal; // Link input doesn't use a volume control parameter
		}
		float volume = params[volumeParam].getValue();
		return signal * volume;
	}

	// Function to clamp the output to a safe volume range (±5V)
	float applyVolumeProtection(float signal) {
		const float maxVoltage = 5.0f; // Maximum voltage to prevent clipping
		if (signal > maxVoltage) {
			return maxVoltage;
		} else if (signal < -maxVoltage) {
			return -maxVoltage;
		}
		return signal;
	}

};

struct MoffenmixWidget : ModuleWidget {
	MoffenmixWidget(Moffenmix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Moffenmix.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(13.6, 16.838)), module, Moffenmix::GAIN_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(13.6, 34.796)), module, Moffenmix::GAIN_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(13.6, 52.67)), module, Moffenmix::GAIN_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(13.6, 70.682)), module, Moffenmix::GAIN_4_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(47.3, 16.838)), module, Moffenmix::VOLUME_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(47.3, 34.796)), module, Moffenmix::VOLUME_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(47.3, 52.67)), module, Moffenmix::VOLUME_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(47.3, 70.682)), module, Moffenmix::VOLUME_4_PARAM));

		// Added 3-position switches for range parameters
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(30.386, 15.898)), module, Moffenmix::RANGE_1_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(30.386, 33.828)), module, Moffenmix::RANGE_2_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(30.386, 51.655)), module, Moffenmix::RANGE_3_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(30.386, 69.527)), module, Moffenmix::RANGE_4_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(12.941, 88.384)), module, Moffenmix::MUTE_1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(24.787, 88.384)), module, Moffenmix::MUTE_2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(36.731, 88.384)), module, Moffenmix::MUTE_3_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(48.007, 88.384)), module, Moffenmix::MUTE_4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.91, 101.869)), module, Moffenmix::CH_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.479, 101.869)), module, Moffenmix::CH_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(12.864, 112.987)), module, Moffenmix::CH_3_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.432, 112.987)), module, Moffenmix::CH_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(48, 101.869)), module, Moffenmix::LINK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48, 112.987)), module, Moffenmix::MIX_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.488, 23.247)), module, Moffenmix::LED_1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.488, 41.122)), module, Moffenmix::LED_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.488, 59.057)), module, Moffenmix::LED_3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.488, 77.22)), module, Moffenmix::LED_4_LIGHT));
	}
};

Model* modelMoffenmix = createModel<Moffenmix, MoffenmixWidget>("Moffenmix");