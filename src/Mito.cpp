#include "plugin.hpp"


struct Mito : Module {
	enum ParamId {
		KNOB1_PARAM,
		KNOB2_PARAM,
		KNOB3_PARAM,
		SWING_PARAM,
		MUTE1_PARAM,
		MUTE2_PARAM,
		MUTE3_PARAM,
		KNOB5_PARAM,
		KNOB6_PARAM,
		WIDTH_PARAM,
		MUTE4_PARAM,
		MUTE5_PARAM,
		MUTE6_PARAM,
		KNOB4_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SWING_CVINPUT,
		WIDTH_CVINPUT,
		BANG_INPUT,
		CH1_CVINPUT,
		CH2_CVINPUT,
		CH3_CVINPUT,
		RESET_INPUT,
		CH4_CVINPUT,
		CH5_CVINPUT,
		CH6_CVINPUT,
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
		configParam(KNOB1_PARAM, 0.f, 1.f, 1.f, "Division 1");
		configParam(KNOB2_PARAM, 0.f, 1.f, 1.f, "Division 2");
		configParam(KNOB3_PARAM, 0.f, 1.f, 1.f, "Division 3");
		configParam(KNOB5_PARAM, 0.f, 1.f, 1.f, "Division 4");
		configParam(KNOB6_PARAM, 0.f, 1.f, 1.f, "Division 5");
		configParam(KNOB4_PARAM, 0.f, 1.f, 1.f, "Division 6");

		configParam(MUTE1_PARAM, 0.f, 1.f, 1.f, "Mute 1");
		configParam(MUTE2_PARAM, 0.f, 1.f, 1.f, "Mute 2");
		configParam(MUTE3_PARAM, 0.f, 1.f, 1.f, "Mute 3");
		configParam(MUTE4_PARAM, 0.f, 1.f, 1.f, "Mute 4");
		configParam(MUTE5_PARAM, 0.f, 1.f, 1.f, "Mute 5");
		configParam(MUTE6_PARAM, 0.f, 1.f, 1.f, "Mute 6");

		configParam(SWING_PARAM, 0.f, 1.f, 0., "Swing amount");
		configParam(WIDTH_PARAM, 0.f, 1.f, 0.5f, "Width");

		configInput(SWING_CVINPUT, "Swing CV");
		configInput(WIDTH_CVINPUT, "Width CV");

		configInput(BANG_INPUT, "Bang!");
		configInput(RESET_INPUT, "Reset");

		configInput(CH1_CVINPUT, "Division 1 CV");
		configInput(CH2_CVINPUT, "Division 2 CV");
		configInput(CH3_CVINPUT, "Division 3 CV");
		configInput(CH4_CVINPUT, "Division 4 CV");
		configInput(CH5_CVINPUT, "Division 5 CV");
		configInput(CH6_CVINPUT, "Division 6 CV");

		configOutput(CH1_OUTPUT, "1");
		configOutput(CH2_OUTPUT, "2");
		configOutput(CH3_OUTPUT, "3");
		configOutput(CH4_OUTPUT, "4");
		configOutput(CH5_OUTPUT, "5");
		configOutput(CH6_OUTPUT, "6");
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

	float pulseWidthParam = 0.0f;
	float widthCvInput = 0.0f;
	float normalizedWidthCV = 0.0f;
	float widthParam = 0.0f;
	float widthValue = 0.0f;

	float swingCvInput = 0.0f;
	float normalizedSwingCV = 0.0f;
	float swingValue = 0.0f;
	float swingParamValue = 0.0f;
	float swingParam = 0.0f;

	float cvInput = 0.0f;
    float normalizedCV = 0.0f;
    float knob1Param = 0.0f;
    float knob1Value = 0.0f;

	float cvInput2 = 0.0f;
	float normalizedCV2 = 0.0f;
	float knob2Param = 0.0f;
	float knob2Value = 0.0f;

	float cvInput3 = 0.0f;
	float normalizedCV3 = 0.0f;
	float knob3Param = 0.0f;
	float knob3Value = 0.0f;

	float cvInput4 = 0.0f;
	float normalizedCV4 = 0.0f;
	float knob4Param = 0.0f;
	float knob4Value = 0.0f;

	float cvInput5 = 0.0f;
	float normalizedCV5 = 0.0f;
	float knob5Param = 0.0f;
	float knob5Value = 0.0f;

	float cvInput6 = 0.0f;
	float normalizedCV6 = 0.0f;
	float knob6Param = 0.0f;
	float knob6Value = 0.0f;

	bool mute1 = false; 
	bool mute2 = false; 
	bool mute3 = false; 
	bool mute4 = false; 
	bool mute5 = false; 
	bool mute6 = false; 


	void process(const ProcessArgs& args) override {

		// Pulsewidth CV and knob scale
        pulseWidthParam = params[WIDTH_PARAM].getValue() * 0.2f + 0.05f;
		widthCvInput = inputs[WIDTH_CVINPUT].getVoltage();  // Read CV input for WIDTH
		normalizedWidthCV = (widthCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		widthParam = params[WIDTH_PARAM].getValue();  // Original WIDTH_PARAM value
		widthValue = widthParam + (normalizedWidthCV - 0.5f);  // Apply the CV input as an offset
		widthValue = clamp(widthValue, 0.0f, 1.0f);
		pulseWidthParam = widthValue * 0.2f + 0.05f;  // Scale pulse width
		pw = (clockDuration * pulseWidthParam);
        
		// Swing CV and knob scale 
        swingCvInput = inputs[SWING_CVINPUT].getVoltage();  // Read CV input for SWING
        normalizedSwingCV = (swingCvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        swingValue = params[SWING_PARAM].getValue();  // Original SWING_PARAM value
        swingParamValue = swingValue + (normalizedSwingCV - 0.5f);  // Apply the CV input as an offset
        swingParamValue = clamp(swingParamValue, 0.0f, 1.0f);
        swingParam = swingParamValue * 100.0f; // The max value of the swing parameter is scaled to 100ms

        // CH 1 CV
        cvInput = inputs[CH1_CVINPUT].getVoltage();  // Read CV input
        normalizedCV = (cvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        knob1Param = params[KNOB1_PARAM].getValue();  // Original knob value
        knob1Value = knob1Param + (normalizedCV - 0.5f);  // Apply the CV input as an offset
        knob1Value = clamp(knob1Value, 0.0f, 1.0f);
        divisionAmount = 1 + (1.0 - knob1Value) * 15;  // Update division based on knob and CV input

		// CH 2 CV
		cvInput2 = inputs[CH2_CVINPUT].getVoltage();  // Read CV input
		normalizedCV2 = (cvInput2 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		knob2Param = params[KNOB2_PARAM].getValue();  // Original knob value
		knob2Value = knob2Param + (normalizedCV2 - 0.5f);  // Apply the CV input as an offset
		knob2Value = clamp(knob2Value, 0.0f, 1.0f);
		divisionAmount2 = 1 + (1.0 - knob2Value) * 15;  // Update division based on knob and CV input

		// CH 3 CV
		cvInput3 = inputs[CH3_CVINPUT].getVoltage();  // Read CV input
		normalizedCV3 = (cvInput3 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		knob3Param = params[KNOB3_PARAM].getValue();  // Original knob value
		knob3Value = knob3Param + (normalizedCV3 - 0.5f);  // Apply the CV input as an offset
		knob3Value = clamp(knob3Value, 0.0f, 1.0f);
		divisionAmount3 = 1 + (1.0 - knob3Value) * 15;  // Update division based on knob and CV input
	   
       // CH 4 CV
	   cvInput4 = inputs[CH4_CVINPUT].getVoltage();  // Read CV input
	   normalizedCV4 = (cvInput4 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
	   knob4Param = params[KNOB4_PARAM].getValue();  // Original knob value
	   knob4Value = knob4Param + (normalizedCV4 - 0.5f);  // Apply the CV input as an offset
	   knob4Value = clamp(knob4Value, 0.0f, 1.0f);
	   divisionAmount4 = 1 + (1.0 - knob4Value) * 15;  // Update division based on knob and CV input

	    // CH 5 CV
		cvInput5 = inputs[CH5_CVINPUT].getVoltage();  // Read CV input
		normalizedCV5 = (cvInput5 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		knob5Param = params[KNOB5_PARAM].getValue();  // Original knob value
		knob5Value = knob5Param + (normalizedCV5 - 0.5f);  // Apply the CV input as an offset
		knob5Value = clamp(knob5Value, 0.0f, 1.0f);
		divisionAmount5 = 1 + (1.0 - knob5Value) * 15;  // Update division based on knob and CV input
	   
		// CH 6 CV
		cvInput6 = inputs[CH6_CVINPUT].getVoltage();  // Read CV input
		normalizedCV6 = (cvInput6 + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
		knob6Param = params[KNOB6_PARAM].getValue();  // Original knob value
		knob6Value = knob6Param + (normalizedCV6 - 0.5f);  // Apply the CV input as an offset
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
		mute1 = params[MUTE1_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
		if (!mute1) {
			outputs[CH1_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
			lights[LED1_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
		} else {
			if (sinceClock < (pw + swing) && sinceClock > swing) {
				if (masterCount % divisionAmount == 0) {
					outputs[CH1_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
					lights[LED1_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
				}
			} else {
				if (sinceOut > ((pw * divisionAmount) + swing)) {
					outputs[CH1_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
					lights[LED1_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
				}
			}
		}

			    // Out channel 2
				mute2 = params[MUTE2_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute2) {
					outputs[CH2_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
					lights[LED2_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
				} else {
					if (sinceClock2 < (pw + swing) && sinceClock2 > swing) {
						if (masterCount % divisionAmount2 == 0) {
							outputs[CH2_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
							lights[LED2_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
						}
					} else {
						if (sinceOut2 > ((pw * divisionAmount2) + swing)) {
							outputs[CH2_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
							lights[LED2_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
						}
					}
				}

			    // Out channel 3
				mute3 = params[MUTE3_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
				if (!mute3) {
					outputs[CH3_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
					lights[LED3_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
				} else {
					if (sinceClock3 < (pw + swing) && sinceClock3 > swing) {
						if (masterCount % divisionAmount3 == 0) {
							outputs[CH3_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
							lights[LED3_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
						}
					} else {
						if (sinceOut3 > ((pw * divisionAmount3) + swing)) {
							outputs[CH3_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
							lights[LED3_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
						}
					}
				}
					    // Out channel 4
						mute4 = params[MUTE4_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						if (!mute4) {
							outputs[CH4_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
							lights[LED4_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
						} else {
							if (sinceClock4 < (pw + swing) && sinceClock4 > swing) {
								if (masterCount % divisionAmount4 == 0) {
									outputs[CH4_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
									lights[LED4_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
								}
							} else {
								if (sinceOut4 > ((pw * divisionAmount4) + swing)) {
									outputs[CH4_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
									lights[LED4_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
								}
							}
						}

						   // Out channel 5
						   mute5 = params[MUTE5_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
						   if (!mute5) {
							   outputs[CH5_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
							   lights[LED5_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
						   } else {
							   if (sinceClock5 < (pw + swing) && sinceClock5 > swing) {
								   if (masterCount % divisionAmount5 == 0) {
									   outputs[CH5_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
									   lights[LED5_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
								   }
							   } else {
								   if (sinceOut5 > ((pw * divisionAmount5) + swing)) {
									   outputs[CH5_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
									   lights[LED5_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
								   }
							   }
						   }

						     // Out channel 6
							 mute6 = params[MUTE6_PARAM].getValue() > 0.5f; // Mute is active if value is > 0.5
							 if (!mute6) {
								 outputs[CH6_OUTPUT].setVoltage(0.0f);  // Mute CH1 (output off)
								 lights[LED6_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
							 } else {
								 if (sinceClock6 < (pw + swing) && sinceClock6 > swing) {
									 if (masterCount % divisionAmount6 == 0) {
										 outputs[CH6_OUTPUT].setVoltage(5.0f);  // Turn on CH1 output voltage
										 lights[LED6_LIGHT].setBrightness(5.0f); // Turn on the LED for CH1
									 }
								 } else {
									 if (sinceOut6 > ((pw * divisionAmount6) + swing)) {
										 outputs[CH6_OUTPUT].setVoltage(0.0f);  // Turn CH1 output off
										 lights[LED6_LIGHT].setBrightness(0.0f); // Turn off the LED for CH1
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