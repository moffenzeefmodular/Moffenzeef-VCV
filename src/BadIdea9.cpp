#include "plugin.hpp"
#include <algorithm> 

struct BadIdea9 : Module {
	enum ParamId {
		RUH_PARAM,
		ROH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PWR_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIO_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_LIGHT,
		LIGHTS_LEN
	};
	    // Frequencies for both oscillators
		float ruhFreq = 440.0f;  // Default frequency for main oscillator
		float rohFreq = 440.0f;  // Default frequency for sync oscillator
	    float pitchCV = 0.0f; 

		// Phase tracking for both oscillators
		float mainOscPhase = 0.0f;
		float syncOscPhase = 0.0f;
	
		float lastCvInput = 0.0f;
		float prevOutput = 0.0f;
		
		// Constructor
	BadIdea9() {
			config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
			configParam(RUH_PARAM, 0.0f, 1.0f, 0.5f, "Main Oscillator Frequency", " Hz", 11.781f, 160.f); 
			configParam(ROH_PARAM, 0.0f, 1.0f, 0.5f, "Sync Oscillator Frequency", " Hz", 300.f, 10.f); 
			configInput(PWR_INPUT, "Voltage Starve CV");
			configOutput(AUDIO_OUTPUT, "Bad Idea #9");
		}

		void process(const ProcessArgs& args) override {
			
			float rate = 50.0f;
			
			float cvInput = inputs[PWR_INPUT].getVoltage();
			cvInput = std::clamp(cvInput, 0.0f, 5.0f);
			
			float delta = cvInput - lastCvInput;
			
			if (delta > rate * args.sampleTime) {
				delta = rate * args.sampleTime;
			} else if (delta < -rate * args.sampleTime) {
				delta = -rate * args.sampleTime;
			}
			
			lastCvInput += delta;
			
			float finalCvInput = lastCvInput;
			
			float ruhParam = (5.0f - params[RUH_PARAM].getValue() * 10.0f);
			float rohParam = (5.0f - params[ROH_PARAM].getValue() * 10.0f);
			
			float summedValue1 = ruhParam + finalCvInput;
			float summedValue2 = rohParam + finalCvInput;
			
			float controlValue1 = std::clamp(summedValue1, 0.0f, 5.0f);
			float controlValue2 = std::clamp(summedValue2, 0.0f, 5.0f);
			
			float minFreq1 = 120.0f;
			float maxFreq1 = 1885.0f;
			float minFreq2 = 10.0f;
			float maxFreq2 = 3000.0f;
			
			float freq1 = maxFreq1 - (controlValue1 / 5.0f) * (maxFreq1 - minFreq1);
			float freq2 = maxFreq2 - (controlValue2 / 5.0f) * (maxFreq2 - minFreq2);
			
			freq1 = std::clamp(freq1, minFreq1, maxFreq1);
			freq2 = std::clamp(freq2, minFreq2, maxFreq2);
			
			if (freq1 < freq2) {
				freq1 = freq2;  // Ensure main oscillator frequency is never lower than modulator frequency
			}
			
			float mainOscInc = freq1 / args.sampleRate;
			float syncOscInc = freq2 / args.sampleRate;
			
			mainOscPhase += mainOscInc;
			syncOscPhase += syncOscInc;
			
			if (syncOscPhase >= 1.0f) {
				syncOscPhase -= 1.0f;
				mainOscPhase = 0.0f;
			}
			
			float mainOscOutput = (mainOscPhase < 0.5f) ? 5.0f : -5.0f;
			mainOscOutput = std::clamp(mainOscOutput, -5.0f, 5.0f);
			
			float f_cutoff = 1000.0f;
			float fs = args.sampleRate;
			float tau = 1.0f / (2 * M_PI * f_cutoff);
			float alpha = tau / (tau + 1.0f / fs);
			
			float filteredOutput = mainOscOutput - prevOutput + alpha * prevOutput;
			
			prevOutput = filteredOutput;
			
			float finalOutput = filteredOutput * cvInput * 0.2f;
			finalOutput = std::clamp(finalOutput, -3.5f, 3.5f);
			float LED = cvInput * 0.2f;
			
			outputs[AUDIO_OUTPUT].setVoltage(finalOutput);
			lights[LED_LIGHT].setBrightnessSmooth(LED, args.sampleTime);
		}					
	};
		
struct BadIdea9Widget : ModuleWidget {
	BadIdea9Widget(BadIdea9* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/BadIdea9.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.29, 17.153)), module, BadIdea9::RUH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.224, 46.111)), module, BadIdea9::ROH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.231, 87.118)), module, BadIdea9::PWR_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.217, 110.056)), module, BadIdea9::AUDIO_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.175, 76.098)), module, BadIdea9::LED_LIGHT));
	}
};


Model* modelBadIdea9 = createModel<BadIdea9, BadIdea9Widget>("BadIdea9");