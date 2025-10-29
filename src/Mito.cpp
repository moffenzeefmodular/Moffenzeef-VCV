#include "plugin.hpp"


struct Mito : Module {
	enum ParamId {
		KNOB1_PARAM,
		KNOB2_PARAM,
		KNOB3_PARAM,
		KNOB4_PARAM,
		KNOB5_PARAM,
		KNOB6_PARAM,
		MUTE1_PARAM,
		MUTE2_PARAM,
		MUTE3_PARAM,
		MUTE4_PARAM,
		MUTE5_PARAM,
		MUTE6_PARAM,
		SWING_PARAM,
		WIDTH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BANG_INPUT,
		RESET_INPUT,
		CH1_CVINPUT,
		CH2_CVINPUT,
		CH3_CVINPUT,
		CH4_CVINPUT,
		CH5_CVINPUT,
		CH6_CVINPUT,
		SWING_CVINPUT,
		WIDTH_CVINPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CH1_OUTPUT,
		CH2_OUTPUT,
		CH3_OUTPUT,
		CH4_OUTPUT,
		CH5_OUTPUT,
		CH6_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED1_LIGHT,
		LED2_LIGHT,
		LED3_LIGHT,
		LED4_LIGHT,
		LED5_LIGHT,
		LED6_LIGHT,
		LIGHTS_LEN
	};

	Mito() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(KNOB1_PARAM, 16.f, 1.f, 1.f, "Division 1");
		configParam(KNOB2_PARAM, 16.f, 1.f, 1.f, "Division 2");
		configParam(KNOB3_PARAM, 16.f, 1.f, 1.f, "Division 3");
		configParam(KNOB5_PARAM, 16.f, 1.f, 1.f, "Division 5");
		configParam(KNOB4_PARAM, 16.f, 1.f, 1.f, "Division 4");
		configParam(KNOB6_PARAM, 16.f, 1.f, 1.f, "Division 6");

		paramQuantities[KNOB1_PARAM]->snapEnabled = true; 
		paramQuantities[KNOB2_PARAM]->snapEnabled = true; 
		paramQuantities[KNOB3_PARAM]->snapEnabled = true; 
		paramQuantities[KNOB4_PARAM]->snapEnabled = true; 
		paramQuantities[KNOB5_PARAM]->snapEnabled = true; 
		paramQuantities[KNOB6_PARAM]->snapEnabled = true; 

		configSwitch(MUTE1_PARAM, 0.f, 1.f, 1.f, "Mute 1", {"Mute", "Unmute"});
		configSwitch(MUTE2_PARAM, 0.f, 1.f, 1.f, "Mute 2", {"Mute", "Unmute"});
		configSwitch(MUTE3_PARAM, 0.f, 1.f, 1.f, "Mute 3", {"Mute", "Unmute"});
		configSwitch(MUTE4_PARAM, 0.f, 1.f, 1.f, "Mute 4", {"Mute", "Unmute"});
		configSwitch(MUTE5_PARAM, 0.f, 1.f, 1.f, "Mute 5", {"Mute", "Unmute"});
		configSwitch(MUTE6_PARAM, 0.f, 1.f, 1.f, "Mute 6", {"Mute", "Unmute"});

		configParam(SWING_PARAM, 0.f, 1.f, 0., "Swing amount", " %", 0.f, 100.f);
		configParam(WIDTH_PARAM, 0.f, 1.f, 0.5f, "Width", " %", 5.f, 5.f);

		configInput(BANG_INPUT, "Bang! Gate");
		configInput(RESET_INPUT, "Reset Gate");

		configInput(CH1_CVINPUT, "Division 1 CV");
		configInput(CH2_CVINPUT, "Division 2 CV");
		configInput(CH3_CVINPUT, "Division 3 CV");
		configInput(CH4_CVINPUT, "Division 4 CV");
		configInput(CH5_CVINPUT, "Division 5 CV");
		configInput(CH6_CVINPUT, "Division 6 CV");

		configInput(SWING_CVINPUT, "Swing CV");
		configInput(WIDTH_CVINPUT, "Width CV");

		configOutput(CH1_OUTPUT, "Gate 1");
		configOutput(CH2_OUTPUT, "Gate 2");
		configOutput(CH3_OUTPUT, "Gate 3");
		configOutput(CH4_OUTPUT, "Gate 4");
		configOutput(CH5_OUTPUT, "Gate 5");
		configOutput(CH6_OUTPUT, "Gate 6");
	}

	bool prevBangState = false; 
	bool prevResetState = false; 

	int masterCount = 0;

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

	float clockDuration = 1000.f;

	bool trig1 = false;
	bool trig2 = false;
	bool trig3 = false;
	bool trig4 = false;
	bool trig5 = false;
	bool trig6 = false;

	float swing = 0; 


	void process(const ProcessArgs& args) override {

        // CH 1 CV
        float cvInput = inputs[CH1_CVINPUT].getVoltage();  // Read CV input
        float normalizedCV = (cvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        float knob1Param = 1.f - (params[KNOB1_PARAM].getValue() / 16.f);  // Original knob value
        float knob1Value = knob1Param + (normalizedCV - 0.5f);  // Apply the CV input as an offset
        knob1Value = std::clamp(knob1Value, 0.0f, 1.0f);
        int divisionAmount = 1 + (1.0 - knob1Value) * 15;  // Update division based on knob and CV input

		// CH 2 CV
		float cvInput2 = inputs[CH2_CVINPUT].getVoltage();  // Read CV input
		float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob2Param = 1.f - (params[KNOB2_PARAM].getValue() / 16.f);  // Original knob value
		float knob2Value = knob2Param + (normalizedCV2 - 0.5f);  // Apply the CV input as an offset
		knob2Value = std::clamp(knob2Value, 0.0f, 1.0f);
		int divisionAmount2 = 1 + (1.0 - knob2Value) * 15;  // Update division based on knob and CV input

		// CH 3 CV
		float cvInput3 = inputs[CH3_CVINPUT].getVoltage();  // Read CV input
		float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob3Param = 1.f - (params[KNOB3_PARAM].getValue() / 16.f);  // Original knob value
		float knob3Value = knob3Param + (normalizedCV3 - 0.5f);  // Apply the CV input as an offset
		knob3Value = std::clamp(knob3Value, 0.0f, 1.0f);
		int divisionAmount3 = 1 + (1.0 - knob3Value) * 15;  // Update division based on knob and CV input
	   
       // CH 4 CV
	   float cvInput4 = inputs[CH4_CVINPUT].getVoltage();  // Read CV input
	   float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
	   float knob4Param = 1.f - (params[KNOB4_PARAM].getValue() / 16.f);  // Original knob value
	   float knob4Value = knob4Param + (normalizedCV4 - 0.5f);  // Apply the CV input as an offset
	   knob4Value = std::clamp(knob4Value, 0.0f, 1.0f);
	   int divisionAmount4 = 1 + (1.0 - knob4Value) * 15;  // Update division based on knob and CV input

	    // CH 5 CV
		float cvInput5 = inputs[CH5_CVINPUT].getVoltage();  // Read CV input
		float normalizedCV5 = (cvInput5 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob5Param = 1.f - (params[KNOB5_PARAM].getValue() / 16.f);  // Original knob value
		float knob5Value = knob5Param + (normalizedCV5 - 0.5f);  // Apply the CV input as an offset
		knob5Value = std::clamp(knob5Value, 0.0f, 1.0f);
		int divisionAmount5 = 1 + (1.0 - knob5Value) * 15;  // Update division based on knob and CV input
	   
		// CH 6 CV
		float cvInput6 = inputs[CH6_CVINPUT].getVoltage();  // Read CV input
		float normalizedCV6 = (cvInput6 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float knob6Param = 1.f - (params[KNOB6_PARAM].getValue() / 16.f);  // Original knob value
		float knob6Value = knob6Param + (normalizedCV6 - 0.5f);  // Apply the CV input as an offset
		knob6Value = std::clamp(knob6Value, 0.0f, 1.0f);
		int divisionAmount6 = 1 + (1.0 - knob6Value) * 15;  // Update division based on knob and CV input

		// Pulsewidth CV and knob scale
		float widthCvInput = inputs[WIDTH_CVINPUT].getVoltage();  // Read CV input for WIDTH
		float normalizedWidthCV = (widthCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float widthParam = params[WIDTH_PARAM].getValue();  // Original WIDTH_PARAM value
		float widthValue = widthParam + (normalizedWidthCV - 0.5f);  // Apply the CV input as an offset
		widthValue = std::clamp(widthValue, 0.0f, 1.0f);
		float pulseWidthParam = widthValue * 0.2f + 0.05f;  // Scale pulse width
		float pw = (clockDuration * pulseWidthParam);
				
		// Swing CV and knob scale 
	 	float swingCvInput = inputs[SWING_CVINPUT].getVoltage();  // Read CV input for SWING
		float normalizedSwingCV = (swingCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		float swingValue = params[SWING_PARAM].getValue();  // Original SWING_PARAM value
		float swingParamValue = swingValue + (normalizedSwingCV - 0.5f);  // Apply the CV input as an offset
		swingParamValue = std::clamp(swingParamValue, 0.0f, 1.0f);
		float swingParam = swingParamValue * 100.0f; // The max value of the swing parameter is scaled to 100ms
		   
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
		bool mute1 = params[MUTE1_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
		if (!mute1) {
		trig1 = false;
		outputs[CH1_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
		} else {
			if (sinceClock < (pw + swing) && sinceClock > swing) {
				if (masterCount % divisionAmount == 0) {
					trig1 = true;
					outputs[CH1_OUTPUT].setVoltage(5.0f);  // Mute CH1 (output off)
				}
			} else {
				if (sinceOut > ((pw * divisionAmount) + swing)) {
					trig1 = false;
					outputs[CH1_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
				}
			}
		}

			    // Out channel 2
				bool mute2 = params[MUTE2_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute2) {
				trig2 = false;
				outputs[CH2_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
				} else {
					if (sinceClock2 < (pw + swing) && sinceClock2 > swing) {
						if (masterCount % divisionAmount2 == 0) {
							trig2 = true;
							outputs[CH2_OUTPUT].setVoltage(5.0f);  // Mute CH1 (output off)
						}
					} else {
						if (sinceOut2 > ((pw * divisionAmount2) + swing)) {
							trig2 = false; 
							outputs[CH2_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
						}
					}
				}

			    // Out channel 3
				bool mute3 = params[MUTE3_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute3) {
					trig3 = false;
					outputs[CH3_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
				} else {
					if (sinceClock3 < (pw + swing) && sinceClock3 > swing) {
						if (masterCount % divisionAmount3 == 0) {
							trig3 = true;
							outputs[CH3_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
						}
					} else {
						if (sinceOut3 > ((pw * divisionAmount3) + swing)) {
							trig3 = false;
							outputs[CH3_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
						}
					}
				}
					    // Out channel 4
						bool mute4 = params[MUTE4_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						if (!mute4) {
							trig4 = false;
							outputs[CH4_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
						} else {
							if (sinceClock4 < (pw + swing) && sinceClock4 > swing) {
								if (masterCount % divisionAmount4 == 0) {
									trig4 = true;
									outputs[CH4_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
								}
							} else {
								if (sinceOut4 > ((pw * divisionAmount4) + swing)) {
									trig4 = false; 
									outputs[CH4_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
								}
							}
						}

						   // Out channel 5
						   bool mute5 = params[MUTE5_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						   if (!mute5) {
							trig5 = false;
							   outputs[CH5_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
						   } else {
							   if (sinceClock5 < (pw + swing) && sinceClock5 > swing) {
								   if (masterCount % divisionAmount5 == 0) {
									trig5 = true;
									   outputs[CH5_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
								   }
							   } else {
								   if (sinceOut5 > ((pw * divisionAmount5) + swing)) {
									trig5 = false;
									   outputs[CH5_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
								   }
							   }
						   }

						     // Out channel 6
							bool mute6 = params[MUTE6_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
							 if (!mute6) {
								trig6 = false;
								 outputs[CH6_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
							 } else {
								 if (sinceClock6 < (pw + swing) && sinceClock6 > swing) {
									 if (masterCount % divisionAmount6 == 0) {
										trig6 = true;
										 outputs[CH6_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
									 }
								 } else {
									 if (sinceOut6 > ((pw * divisionAmount6) + swing)) {
										trig6 = false; 
										 outputs[CH6_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
									 }
								 }
							 }

							 lights[LED1_LIGHT].setBrightnessSmooth(trig1, args.sampleTime); // Turn on the LED for CH1
							 lights[LED2_LIGHT].setBrightnessSmooth(trig2, args.sampleTime); // Turn on the LED for CH1
							 lights[LED3_LIGHT].setBrightnessSmooth(trig3, args.sampleTime); // Turn on the LED for CH1
							 lights[LED4_LIGHT].setBrightnessSmooth(trig4, args.sampleTime); // Turn on the LED for CH1
							 lights[LED5_LIGHT].setBrightnessSmooth(trig5, args.sampleTime); // Turn on the LED for CH1
							 lights[LED6_LIGHT].setBrightnessSmooth(trig6, args.sampleTime); // Turn on the LED for CH1
	}
};
	
	
struct MitoWidget : ModuleWidget {
	MitoWidget(Mito* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Mito.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.796, 16.559)), module, Mito::KNOB1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(35.224, 16.574)), module, Mito::KNOB2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.449, 16.472)), module, Mito::KNOB3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(12.711, 56.698)), module, Mito::KNOB4_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(35.141, 56.698)), module, Mito::KNOB5_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.459, 56.814)), module, Mito::KNOB6_PARAM));

		addParam(createParamCentered<CKSS>(mm2px(Vec(12.827, 39.616)), module, Mito::MUTE1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(35.131, 39.662)), module, Mito::MUTE2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.264, 39.626)), module, Mito::MUTE3_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(12.711, 79.681)), module, Mito::MUTE4_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(35.134, 79.81)), module, Mito::MUTE5_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(57.254, 79.856)), module, Mito::MUTE6_PARAM));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(80.008, 16.459)), module, Mito::SWING_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(80.031, 56.722)), module, Mito::WIDTH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.445, 109.559)), module, Mito::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.409, 95.354)), module, Mito::BANG_INPUT));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(79.851, 37.084)), module, Mito::SWING_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(79.861, 77.246)), module, Mito::WIDTH_CVINPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.511, 95.33)), module, Mito::CH1_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.579, 95.307)), module, Mito::CH2_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.552, 95.248)), module, Mito::CH3_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.524, 109.592)), module, Mito::CH4_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(34.523, 109.492)), module, Mito::CH5_CVINPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.516, 109.492)), module, Mito::CH6_CVINPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.531, 95.207)), module, Mito::CH1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.506, 95.225)), module, Mito::CH2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.597, 95.238)), module, Mito::CH3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.553, 109.546)), module, Mito::CH4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.572, 109.502)), module, Mito::CH5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(82.584, 109.502)), module, Mito::CH6_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.754, 27.761)), module, Mito::LED1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.154, 27.7)), module, Mito::LED2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.366, 27.779)), module, Mito::LED3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(12.791, 68.019)), module, Mito::LED4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.108, 68.032)), module, Mito::LED5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(57.379, 68.055)), module, Mito::LED6_LIGHT));
	}
};


Model* modelMito = createModel<Mito, MitoWidget>("Mito");