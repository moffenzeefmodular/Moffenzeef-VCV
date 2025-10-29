#include "plugin.hpp"
#include "../res/sequences/SimplifySequences.hpp"

struct Simplify : Module {
	enum ParamId {
		SELECT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BANG_INPUT,
		RESET_INPUT,
		SELECT_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SIMPLIFY_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_LIGHT,
		LIGHTS_LEN
	};

	Simplify() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SELECT_PARAM, 1.f, 137.f, 0.f, "Pattern"); // Figure out how to truncate decimals from display 
		paramQuantities[SELECT_PARAM]->snapEnabled = true; 
		configInput(BANG_INPUT, "Bang! Gate");
		configInput(RESET_INPUT, "Reset Gate");
		configInput(SELECT_CV_INPUT, "Select CV");
		configOutput(SIMPLIFY_OUTPUT, "Simplify Gate");
	}
  
bool prevBangState = 0; 
bool prevResetState = 0; 

int numSeq = 137;
int numSteps = 16;

int masterCount = 0;

float sinceClock = 0.0f;
int pulseWidth = 10; // 5ms pulse width

int seqSelect = 0;
int seqOut = 0;

bool trig1 = false; 

	void process(const ProcessArgs& args) override {
        // CH 1 CV
        float cvInput = inputs[SELECT_CV_INPUT].getVoltage();  // Read CV input
        float normalizedCV = (cvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        float knob1Param = params[SELECT_PARAM].getValue() / 137.f;  // Original knob value
        float knob1Value = knob1Param + (normalizedCV - 0.5f);  // Apply the CV input as an offset
        knob1Value = clamp(knob1Value, 0.0f, 1.0f);

        seqSelect = knob1Value * 136;  // Update division based on knob and CV input

		sinceClock += args.sampleTime * 1000.0f;

				// Read the current state of the BANG_INPUT
				bool bangState = inputs[BANG_INPUT].getVoltage() > 0.5f;
				bool resetState = inputs[RESET_INPUT].getVoltage() > 0.5f;
				
				// Detect a rising edge (transition from low to high)
				if (bangState && !prevBangState) {
					sinceClock = 0;
					masterCount++;  // 
					}
			
					// Update previous state of BANG_INPUT
					prevBangState = bangState;

					if(masterCount == (numSteps +1)){
				    masterCount = 1;
					}
					clamp(masterCount, 1, numSteps);
						
				if(resetState && !prevResetState){
					masterCount = 1; 
					}
			
					prevResetState = resetState; 

					seqOut = *(seq[seqSelect] + masterCount);

					// Lookup + AND + PWM + PRINT OUTPUTS
					if (seqOut == 0 && bangState == 1 && sinceClock < pulseWidth) {
						outputs[SIMPLIFY_OUTPUT].setVoltage(5.0f);  // Turn on CH_1 output voltage
						trig1 = true;
						}
					else {
						outputs[SIMPLIFY_OUTPUT].setVoltage(0.0f);  // Turn on CH_1 output voltage
						trig1 = false;
						}

						lights[LED_LIGHT].setBrightnessSmooth(trig1, args.sampleTime); // Turn on the LED for CH_1

	}
};


struct SimplifyWidget : ModuleWidget {
	SimplifyWidget(Simplify* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Simplify.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.162, 32.667)), module, Simplify::SELECT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.162, 76.56)), module, Simplify::BANG_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.122, 88.66)), module, Simplify::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.149, 100.567)), module, Simplify::SELECT_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.149, 112.654)), module, Simplify::SIMPLIFY_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.133, 19.544)), module, Simplify::LED_LIGHT));
	}
};


Model* modelSimplify = createModel<Simplify, SimplifyWidget>("Simplify");