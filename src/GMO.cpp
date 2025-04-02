#include "plugin.hpp"
#include "GMO_tables.hh"

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


GMO() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SPECIMEN_PARAM, 0.f, 1.f, 0.f, "Specimen");
		configParam(SPEED_PARAM, 0.f, 1.f, 0.5f, "Speed");
		configParam(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop");
		configParam(HEAD_PARAM, 0.f, 1.f, 0.f, "Head");
		configParam(TAIL_PARAM, 0.f, 1.f, 1.f, "Tail");
		configInput(BANG_INPUT, "Bang!");
		configInput(SPECIMEN_CV_INPUT, "Specimen CV");
		configInput(SPEED_CV_INPUT, "Speed CV");
		configInput(LOOP_CV_INPUT, "Loop CV");
		configInput(HEAD_CV_INPUT, "Head CV");
		configInput(TAIL_CV_INPUT, "Tail CV");
		configOutput(GMO_OUTPUT, "GMO");
    }

    float speed = 1.0f;  // Default speed value
    float wavetableIndex = 0.0f;
    float outputSignal = 0.0f;
	int8_t sample;  

	bool previousBangState = false; 

	void process(const ProcessArgs &args) override {
        using namespace GMOTables;
		const int GAP = 200; // Define the gap between HEAD and TAIL (can be adjusted)
	    
		//Speed
		float cvInput = inputs[SPEED_CV_INPUT].getVoltage();
		float normalizedCV = (cvInput + 5.0f) / 10.0f;
		float knob1Param = params[SPEED_PARAM].getValue() + 0.05;
		float knob1Value = knob1Param + (normalizedCV - 0.5f);
		float controlValue = clamp(knob1Value, 0.0f, 1.0f);
		speed = (controlValue * 3.f) + 0.05f;
	    
		//Specimen
		float cvInput2 = inputs[SPECIMEN_CV_INPUT].getVoltage();
		float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f;
		float knob2Param = params[SPECIMEN_PARAM].getValue();
		float knob2Value = knob2Param + (normalizedCV2 - 0.5f);
		float controlValue2 = clamp(knob2Value, 0.0f, 1.0f);
		int sampleSelect = (controlValue2 * 7) + 1;
        
		//Head
		float cvInput3 = inputs[HEAD_CV_INPUT].getVoltage();
		float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f;
		float knob3Param = params[HEAD_PARAM].getValue();
		float knob3Value = knob3Param + (normalizedCV3 - 0.5f);
		float controlValue3 = clamp(knob3Value, 0.0f, 1.0f);
		int headIndex = (int)(controlValue3 * WAVETABLE_SIZE);

		//Tail
		float cvInput4 = inputs[TAIL_CV_INPUT].getVoltage();
		float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f;
		float knob4Param = params[TAIL_PARAM].getValue();
		float knob4Value = knob4Param + (normalizedCV4 - 0.5f);
		float controlValue4 = clamp(knob4Value, 0.0f, 1.0f);
		int tailIndex = (int)(controlValue4 * WAVETABLE_SIZE);

		// Don't let knobs cross 
		if (tailIndex <= headIndex + GAP) {
			tailIndex = headIndex + GAP;
		}
		if (headIndex >= tailIndex) {
			headIndex = tailIndex - GAP;
		}
	    
		// LOOP SWITCH
		bool switchState = params[LOOP_PARAM].getValue() > 0.5f;
		bool gateState = inputs[LOOP_CV_INPUT].getVoltage() > 0.5f;
        
		// One shot states
		bool bangState = inputs[BANG_INPUT].getVoltage() < 0.5f;  

		if(switchState || gateState){ // LOOPING MODE
		if (wavetableIndex >= WAVETABLE_SIZE) {
				wavetableIndex -= WAVETABLE_SIZE;
			}
			wavetableIndex = clamp(wavetableIndex, (float)headIndex, (float)tailIndex);
			if (wavetableIndex >= tailIndex) {
				wavetableIndex = (float)headIndex;  // Reset to HEAD index when TAIL is reached
				}		
			}
		else{	
		if (bangState < previousBangState) { 
		wavetableIndex = headIndex; 
		}
		previousBangState = bangState; 
	    } 

		wavetableIndex = clamp(wavetableIndex, (float)headIndex, (float)tailIndex);
		wavetableIndex += speed * args.sampleTime * WAVETABLE_SIZE;
		
		
switch(sampleSelect) {
			case 1: sample = wavetable[(int)wavetableIndex]; break;
			case 2: sample = wavetable2[(int)wavetableIndex]; break;
			case 3: sample = wavetable3[(int)wavetableIndex]; break;
			case 4: sample = wavetable4[(int)wavetableIndex]; break;
			case 5: sample = wavetable5[(int)wavetableIndex]; break;
			case 6: sample = wavetable6[(int)wavetableIndex]; break;
			case 7: sample = wavetable7[(int)wavetableIndex]; break;
			case 8: sample = wavetable8[(int)wavetableIndex]; break;
		}
	
		outputSignal = (float)sample / 128.0f;
		float finalOutput = outputSignal * 20.0f;
		finalOutput = clamp(finalOutput, -5.0f, 5.0f);
	
		outputs[GMO_OUTPUT].setVoltage(finalOutput);
		lights[LED_LIGHT].setBrightness(finalOutput * 0.2);
	}
};	
struct GMOWidget : ModuleWidget {
	GMOWidget(GMO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/GMO.svg")));

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
};


Model* modelGMO = createModel<GMO, GMOWidget>("GMO");
