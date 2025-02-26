#include "plugin.hpp"


struct Mito : Module {
	enum ParamId {
		KNOB_1_PARAM,
		KNOB_2_PARAM,
		KNOB_3_PARAM,
		SWING_PARAM,
		MUTE_1_PARAM,
		MUTE_2_PARAM,
		MUTE_3_PARAM,
		KNOB_5_PARAM,
		KNOB_6_PARAM,
		WIDTH_PARAM,
		MUTE_4_PARAM,
		MUTE_5_PARAM,
		MUTE_6_PARAM,
		KNOB_4_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SWING_CV_INPUT,
		WIDTH_CV_INPUT,
		BANG_INPUT,
		CH_1_CV_INPUT,
		CH_2_CV_INPUT,
		CH_3_CV_INPUT,
		RESET_INPUT,
		CH_4_CV_INPUT,
		CH_5_CV_INPUT,
		CH_6_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CH_1_OUTPUT,
		CH_2_OUTPUT,
		CH_3_OUTPUT,
		CH_4_OUTPUT,
		CH_5_OUTPUT,
		CH_6_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_1_LIGHT,
		LED_2_LIGHT,
		LED_3_LIGHT,
		LED_4_LIGHT,
		LED_5_LIGHT,
		LED_6_LIGHT,
		LIGHTS_LEN
	};

	Mito() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(KNOB_1_PARAM, 0.f, 1.f, 1.f, "Division 1");
		configParam(KNOB_2_PARAM, 0.f, 1.f, 1.f, "Division 2");
		configParam(KNOB_3_PARAM, 0.f, 1.f, 1.f, "Division 3");
		configParam(KNOB_5_PARAM, 0.f, 1.f, 1.f, "Division 4");
		configParam(KNOB_6_PARAM, 0.f, 1.f, 1.f, "Division 5");
		configParam(KNOB_4_PARAM, 0.f, 1.f, 1.f, "Division 6");

		configParam(MUTE_1_PARAM, 0.f, 1.f, 1.f, "Mute 1");
		configParam(MUTE_2_PARAM, 0.f, 1.f, 1.f, "Mute 2");
		configParam(MUTE_3_PARAM, 0.f, 1.f, 1.f, "Mute 3");
		configParam(MUTE_4_PARAM, 0.f, 1.f, 1.f, "Mute 4");
		configParam(MUTE_5_PARAM, 0.f, 1.f, 1.f, "Mute 5");
		configParam(MUTE_6_PARAM, 0.f, 1.f, 1.f, "Mute 6");

		configParam(SWING_PARAM, 0.f, 1.f, 0., "Swing amount");
		configParam(WIDTH_PARAM, 0.f, 1.f, 0.5f, "Width");

		configInput(SWING_CV_INPUT, "Swing CV");
		configInput(WIDTH_CV_INPUT, "Width CV");

		configInput(BANG_INPUT, "Bang!");
		configInput(RESET_INPUT, "Reset");

		configInput(CH_1_CV_INPUT, "Division 1 CV");
		configInput(CH_2_CV_INPUT, "Division 2 CV");
		configInput(CH_3_CV_INPUT, "Division 3 CV");
		configInput(CH_4_CV_INPUT, "Division 4 CV");
		configInput(CH_5_CV_INPUT, "Division 5 CV");
		configInput(CH_6_CV_INPUT, "Division 6 CV");

		configOutput(CH_1_OUTPUT, "1");
		configOutput(CH_2_OUTPUT, "2");
		configOutput(CH_3_OUTPUT, "3");
		configOutput(CH_4_OUTPUT, "4");
		configOutput(CH_5_OUTPUT, "5");
		configOutput(CH_6_OUTPUT, "6");
	}

	bool prevBangState = 0; 
	bool prevResetState = 0; 
	int masterCount = 0;
	float clockTimer = 0.0f;
	bool isOutputOn = false;
	float onTime = 0.f; 
	float offTime = 0.f;
	float onTimeSamples = 0.f;
	float offTimeSamples = 0.0f;
	float lastBangTime = 0.0f;  // To track the last bang time
	float cycleTime = 0.0f;     // Duration between two BANG pulses

	float sinceClock = 0.0f;
	float sinceClock2 = 0.0f;
	float sinceClock3 = 0.0f;
	float sinceClock4 = 0.0f;
	float sinceClock5 = 0.0f;
	float sinceClock6 = 0.0f;

	float sinceOut = 0.0f;
	float sinceOut2 = 0.0f;
	float sinceOut3 = 0.0f;
	float sinceOut4 = 0.0f;
	float sinceOut5 = 0.0f;
	float sinceOut6 = 0.0f;

	float pw = 0.0f; 
	int divisionAmount = 0; 
	int divisionAmount2 = 0; 
	int divisionAmount3 = 0; 
	int divisionAmount4 = 0; 
	int divisionAmount5 = 0; 
	int divisionAmount6 = 0; 
	float clockDuration = 1000.f;
	float pulseWidth = 0.5f; 
	float swing = 0.0f;
	
	void process(const ProcessArgs& args) override {

		// Pulsewidth CV and knob scale
        float pulseWidthParam = params[WIDTH_PARAM].getValue() * 0.2f + 0.05f;
		float widthCvInput = inputs[WIDTH_CV_INPUT].getVoltage();  // Read CV input for WIDTH
		float normalizedWidthCV = (widthCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float widthParam = params[WIDTH_PARAM].getValue();  // Original WIDTH_PARAM value
		float widthValue = widthParam + (normalizedWidthCV - 0.5f);  // Apply the CV input as an offset
		widthValue = clamp(widthValue, 0.0f, 1.0f);
		pulseWidthParam = widthValue * 0.2f + 0.05f;  // Scale pulse width
		pw = (clockDuration * pulseWidthParam);
        
		// Swing CV and knob scale 
        float swingCvInput = inputs[SWING_CV_INPUT].getVoltage();  // Read CV input for SWING
        float normalizedSwingCV = (swingCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        float swingValue = params[SWING_PARAM].getValue();  // Original SWING_PARAM value
        float swingParamValue = swingValue + (normalizedSwingCV - 0.5f);  // Apply the CV input as an offset
        swingParamValue = clamp(swingParamValue, 0.0f, 1.0f);
        float swingParam = swingParamValue * 100.0f; // The max value of the swing parameter is scaled to 100ms

        // CH 1 CV
        float cvInput = inputs[CH_1_CV_INPUT].getVoltage();  // Read CV input
        float normalizedCV = (cvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        float knob1Param = params[KNOB_1_PARAM].getValue();  // Original knob value
        float knob1Value = knob1Param + (normalizedCV - 0.5f);  // Apply the CV input as an offset
        knob1Value = clamp(knob1Value, 0.0f, 1.0f);
        divisionAmount = 1 + (1.0 - knob1Value) * 15;  // Update division based on knob and CV input

		// CH 2 CV
		float cvInput2 = inputs[CH_2_CV_INPUT].getVoltage();  // Read CV input
		float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob2Param = params[KNOB_2_PARAM].getValue();  // Original knob value
		float knob2Value = knob2Param + (normalizedCV2 - 0.5f);  // Apply the CV input as an offset
		knob2Value = clamp(knob2Value, 0.0f, 1.0f);
		divisionAmount2 = 1 + (1.0 - knob2Value) * 15;  // Update division based on knob and CV input

		// CH 3 CV
		float cvInput3 = inputs[CH_3_CV_INPUT].getVoltage();  // Read CV input
		float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob3Param = params[KNOB_3_PARAM].getValue();  // Original knob value
		float knob3Value = knob3Param + (normalizedCV3 - 0.5f);  // Apply the CV input as an offset
		knob3Value = clamp(knob3Value, 0.0f, 1.0f);
		divisionAmount3 = 1 + (1.0 - knob3Value) * 15;  // Update division based on knob and CV input
	   
       // CH 4 CV
	   float cvInput4 = inputs[CH_4_CV_INPUT].getVoltage();  // Read CV input
	   float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
	   float knob4Param = params[KNOB_4_PARAM].getValue();  // Original knob value
	   float knob4Value = knob4Param + (normalizedCV4 - 0.5f);  // Apply the CV input as an offset
	   knob4Value = clamp(knob4Value, 0.0f, 1.0f);
	   divisionAmount4 = 1 + (1.0 - knob4Value) * 15;  // Update division based on knob and CV input

	    // CH 5 CV
		float cvInput5 = inputs[CH_5_CV_INPUT].getVoltage();  // Read CV input
		float normalizedCV5 = (cvInput5 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob5Param = params[KNOB_5_PARAM].getValue();  // Original knob value
		float knob5Value = knob5Param + (normalizedCV5 - 0.5f);  // Apply the CV input as an offset
		knob5Value = clamp(knob5Value, 0.0f, 1.0f);
		divisionAmount5 = 1 + (1.0 - knob5Value) * 15;  // Update division based on knob and CV input
	   
		// CH 6 CV
		float cvInput6 = inputs[CH_6_CV_INPUT].getVoltage();  // Read CV input
		float normalizedCV6 = (cvInput6 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob6Param = params[KNOB_6_PARAM].getValue();  // Original knob value
		float knob6Value = knob6Param + (normalizedCV6 - 0.5f);  // Apply the CV input as an offset
		knob6Value = clamp(knob6Value, 0.0f, 1.0f);
		divisionAmount6 = 1 + (1.0 - knob6Value) * 15;  // Update division based on knob and CV input
		   
		// Update clock timer (accumulate time in samples)
		sinceClock += args.sampleTime * 1000.0f;
		sinceClock2 += args.sampleTime * 1000.0f;
		sinceClock3 += args.sampleTime * 1000.0f;
		sinceClock4 += args.sampleTime * 1000.0f;
		sinceClock5 += args.sampleTime * 1000.0f;
		sinceClock6 += args.sampleTime * 1000.0f;

		sinceOut += args.sampleTime * 1000.0f;
		sinceOut2 += args.sampleTime * 1000.0f;
		sinceOut3 += args.sampleTime * 1000.0f;
		sinceOut4 += args.sampleTime * 1000.0f;
		sinceOut5 += args.sampleTime * 1000.0f;
		sinceOut6 += args.sampleTime * 1000.0f;


		// Read the current state of the BANG_INPUT
		bool bangState = inputs[BANG_INPUT].getVoltage() > 0.5f;
		bool resetState = inputs[RESET_INPUT].getVoltage() > 0.5f;

		if(resetState && !prevResetState){
	    masterCount = 1; 
		}

		prevResetState = resetState; 
        
		// Detect a rising edge (transition from low to high)
		if (bangState && !prevBangState) {
			masterCount++;  // 
			if (masterCount % divisionAmount == 0) {
			sinceOut = 0;     
			sinceOut2 = 0;     
			sinceOut3 = 0;     
			sinceOut4 = 0;     
			sinceOut5 = 0;     
			sinceOut6 = 0;     

			}
			clockDuration = sinceClock; 
			sinceClock = 0;
			sinceClock2 = 0;
			sinceClock3 = 0;
			sinceClock4 = 0;
			sinceClock5 = 0;
			sinceClock6 = 0;
			}
	
			// Update previous state of BANG_INPUT
		    prevBangState = bangState;

			if (masterCount % 4 == 0){
				swing = 0; 
				}
				else{
				swing = swingParam; 
				}
       

	    // Out channel 1 
		bool mute1 = params[MUTE_1_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
		if (!mute1) {
			outputs[CH_1_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
			lights[LED_1_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
		} else {
			if (sinceClock < (pw + swing) && sinceClock > swing) {
				if (masterCount % divisionAmount == 0) {
					outputs[CH_1_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
					lights[LED_1_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
				}
			} else {
				if (sinceOut > ((pw * divisionAmount) + swing)) {
					outputs[CH_1_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
					lights[LED_1_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
				}
			}
		}

			    // Out channel 2
				bool mute2 = params[MUTE_2_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute2) {
					outputs[CH_2_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
					lights[LED_2_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
				} else {
					if (sinceClock2 < (pw + swing) && sinceClock2 > swing) {
						if (masterCount % divisionAmount2 == 0) {
							outputs[CH_2_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
							lights[LED_2_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
						}
					} else {
						if (sinceOut2 > ((pw * divisionAmount2) + swing)) {
							outputs[CH_2_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
							lights[LED_2_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
						}
					}
				}

			    // Out channel 3
				bool mute3 = params[MUTE_3_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute3) {
					outputs[CH_3_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
					lights[LED_3_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
				} else {
					if (sinceClock3 < (pw + swing) && sinceClock3 > swing) {
						if (masterCount % divisionAmount3 == 0) {
							outputs[CH_3_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
							lights[LED_3_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
						}
					} else {
						if (sinceOut3 > ((pw * divisionAmount3) + swing)) {
							outputs[CH_3_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
							lights[LED_3_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
						}
					}
				}
					    // Out channel 4
						bool mute4 = params[MUTE_4_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						if (!mute4) {
							outputs[CH_4_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
							lights[LED_4_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
						} else {
							if (sinceClock4 < (pw + swing) && sinceClock4 > swing) {
								if (masterCount % divisionAmount4 == 0) {
									outputs[CH_4_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
									lights[LED_4_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
								}
							} else {
								if (sinceOut4 > ((pw * divisionAmount4) + swing)) {
									outputs[CH_4_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
									lights[LED_4_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
								}
							}
						}

						   // Out channel 5
						   bool mute5 = params[MUTE_5_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						   if (!mute5) {
							   outputs[CH_5_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
							   lights[LED_5_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
						   } else {
							   if (sinceClock5 < (pw + swing) && sinceClock5 > swing) {
								   if (masterCount % divisionAmount5 == 0) {
									   outputs[CH_5_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
									   lights[LED_5_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
								   }
							   } else {
								   if (sinceOut5 > ((pw * divisionAmount5) + swing)) {
									   outputs[CH_5_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
									   lights[LED_5_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
								   }
							   }
						   }

						     // Out channel 6
							 bool mute6 = params[MUTE_6_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
							 if (!mute6) {
								 outputs[CH_6_OUTPUT].setVoltage(0.0f);  // Mute CH_1 (output off)
								 lights[LED_6_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
							 } else {
								 if (sinceClock6 < (pw + swing) && sinceClock6 > swing) {
									 if (masterCount % divisionAmount6 == 0) {
										 outputs[CH_6_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
										 lights[LED_6_LIGHT].setBrightness(5.0f); // Turn on the LED for CH_1
									 }
								 } else {
									 if (sinceOut6 > ((pw * divisionAmount6) + swing)) {
										 outputs[CH_6_OUTPUT].setVoltage(0.0f);  // Turn CH_1 output off
										 lights[LED_6_LIGHT].setBrightness(0.0f); // Turn off the LED for CH_1
									 }
								 }
							 }
	}
};
	
	
struct MitoWidget : ModuleWidget {
	MitoWidget(Mito* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Mito.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.796, 16.559)), module, Mito::KNOB_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(35.224, 16.574)), module, Mito::KNOB_2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.449, 16.472)), module, Mito::KNOB_3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.711, 56.698)), module, Mito::KNOB_4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(35.141, 56.698)), module, Mito::KNOB_5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.459, 56.814)), module, Mito::KNOB_6_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(12.827, 39.616)), module, Mito::MUTE_1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(35.131, 39.662)), module, Mito::MUTE_2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.264, 39.626)), module, Mito::MUTE_3_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(12.711, 79.681)), module, Mito::MUTE_4_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(35.134, 79.81)), module, Mito::MUTE_5_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.254, 79.856)), module, Mito::MUTE_6_PARAM));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(80.008, 16.459)), module, Mito::SWING_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(80.031, 56.722)), module, Mito::WIDTH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.445, 109.559)), module, Mito::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.409, 95.354)), module, Mito::BANG_INPUT));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(79.851, 37.084)), module, Mito::SWING_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(79.861, 77.246)), module, Mito::WIDTH_CV_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.511, 95.33)), module, Mito::CH_1_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.579, 95.307)), module, Mito::CH_2_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.552, 95.248)), module, Mito::CH_3_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.524, 109.592)), module, Mito::CH_4_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.523, 109.492)), module, Mito::CH_5_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.516, 109.492)), module, Mito::CH_6_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.531, 95.207)), module, Mito::CH_1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.506, 95.225)), module, Mito::CH_2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.597, 95.238)), module, Mito::CH_3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.553, 109.546)), module, Mito::CH_4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.572, 109.502)), module, Mito::CH_5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.584, 109.502)), module, Mito::CH_6_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.754, 27.761)), module, Mito::LED_1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.154, 27.7)), module, Mito::LED_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.366, 27.779)), module, Mito::LED_3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.791, 68.019)), module, Mito::LED_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.108, 68.032)), module, Mito::LED_5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.379, 68.055)), module, Mito::LED_6_LIGHT));
	}
};


Model* modelMito = createModel<Mito, MitoWidget>("Mito");
