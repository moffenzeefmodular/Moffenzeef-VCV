#include "plugin.hpp"
#include <cstdlib>
#include <cmath>

// Biquad filter parameters (fixed resonance)
#define RESONANCE 0.4f

struct Bobcat : Module {
	enum ParamId {
		TAIL_1__PARAM,
		HISS_1__PARAM,
		SNARL_PARAM,
		TAIL_2_PARAM,
		HISS_2__PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BANG_1_INPUT,
		TAIL_1_CV_INPUT,
		HISS_1_CV_INPUT,
		BANG_2_INPUT,
		TAIL_2_CV_INPUT,
		HISS_2_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		BOBCAT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_LIGHT,
		LIGHTS_LEN
	};

	Bobcat() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TAIL_1__PARAM, 0.f, 1.f, 0.f, "Decay Time 1", " %", 0.f, 100.f);
		configParam(HISS_1__PARAM, 0.f, 1.f, 0.f, "Pitch 1", " %", 0.f, 100.f);
		configSwitch(SNARL_PARAM, 0.f, 1.f, 0.f, "Decay Time Range", {"Short", "Long"});
		configParam(TAIL_2_PARAM, 0.f, 1.f, 0.f, "Decay Time 2", " %", 0.f, 100.f);
		configParam(HISS_2__PARAM, 0.f, 1.f, 0.f, "Pitch 2", " %", 0.f, 100.f);
		configInput(BANG_1_INPUT, "Bang! 1 Gate");
		configInput(TAIL_1_CV_INPUT, "Tail 1 CV");
		configInput(HISS_1_CV_INPUT, "Hiss 1 CV");
		configInput(BANG_2_INPUT, "Bang! 2 Gate");
		configInput(TAIL_2_CV_INPUT, "Tail 2 CV");
		configInput(HISS_2_CV_INPUT, "Hiss 2 CV");
		configOutput(BOBCAT_OUTPUT, "Bobcat Audio");
	}
    

	//Ch. 1
    float hiss1Value = 0.0f;          // HISS_1 parameter value
    float clockSpeed1 = 0.0f;          // Clock speed controlled by HISS_1
    float clockTime1 = 0.0f;           // Time accumulator for clock
    float sampleAndHoldValue1 = 0.0f;  // Holds the sampled value from the white noise
	float decayAlpha1 = 0.0f;
	float envelopeValue1 = 0.0f;
	bool pulseTriggered1 = false;
	float decayTime1 = 0.0f;  // Time in milliseconds for decay
	float pulseTime1 = 0.0f;  // Pulse duration (in samples)
	static bool lastBangState1;
	float filterB01 = 0.0f, filterB11 = 0.0f, filterB21 = 0.0f, filterA01 = 0.0f, filterA11 = 0.0f, filterA21 = 0.0f;
    float filterX11 = 0.0f, filterX21 = 0.0f, filterY11 = 0.0f, filterY21 = 0.0f;
	bool triggerStates1[2] = {false, false};  // triggerStates[0] = last state, triggerStates[1] = current state

	//Ch. 2
    float hiss2Value = 0.0f;          // HISS_1 parameter value
    float clockSpeed2 = 0.0f;          // Clock speed controlled by HISS_1
    float clockTime2 = 0.0f;           // Time accumulator for clock
    float sampleAndHoldValue2 = 0.0f;  // Holds the sampled value from the white noise
	float decayAlpha2 = 0.0f;
	float envelopeValue2 = 0.0f;
	bool pulseTriggered2 = false;
	float decayTime2 = 0.0f;  // Time in milliseconds for decay
	float pulseTime2 = 0.0f;  // Pulse duration (in samples)
	static bool lastBangState2;
	float filterB02 = 0.0f, filterB12 = 0.0f, filterB22 = 0.0f, filterA02 = 0.0f, filterA12 = 0.0f, filterA22 = 0.0f;
    float filterX12 = 0.0f, filterX22 = 0.0f, filterY12 = 0.0f, filterY22 = 0.0f;
	bool triggerStates2[2] = {false, false};  // triggerStates[0] = last state, triggerStates[1] = current state

    // Function to generate white noise
    float whiteNoise1() {
        return (rand() / (float)RAND_MAX) * 10.0f - 5.0f;  // Random between -5V and +5V
    }
	   // Function to generate white noise
	   float whiteNoise2() {
        return (rand() / (float)RAND_MAX) * 10.0f - 5.0f;  // Random between -5V and +5V
    }


    // Function to apply a biquad bandpass filter
    float biquadBandpassFilter1(float input1, float cutoff1, float sampleRate1) {
        // Calculate the filter coefficients for the bandpass filter (using cutoff and resonance)
        float omega1 = 2.0f * M_PI * cutoff1 / sampleRate1;
        float sn1 = sinf(omega1);
        float cs1 = cosf(omega1);
        float alpha1 = sn1 / (2.0f * RESONANCE);

        // Compute the bandpass filter coefficients (Biquad)
        filterB01 = alpha1;
        filterB11 = 0.0f;
        filterB21 = -alpha1;
        filterA01 = 1.0f + alpha1;
        filterA11 = -2.0f * cs1;
        filterA21 = 1.0f - alpha1;

        // Normalize coefficients
        filterB01 /= filterA01;
        filterB11 /= filterA01;
        filterB21 /= filterA01;
        filterA11 /= filterA01;
        filterA21 /= filterA01;

        // Apply the filter to the input signal
        float output1 = filterB01 * input1 + filterB11 * filterX11 + filterB21 * filterX21
                       - filterA11 * filterY11 - filterA21 * filterY21;

        // Update the filter states
        filterX21 = filterX11;
        filterX11 = input1;
        filterY21 = filterY11;
        filterY11 = output1;

        return output1;
    }

	 // Function to apply a biquad bandpass filter
	 float biquadBandpassFilter2(float input2, float cutoff2, float sampleRate2) {
        // Calculate the filter coefficients for the bandpass filter (using cutoff and resonance)
        float omega2 = 2.0f * M_PI * cutoff2 / sampleRate2;
        float sn2 = sinf(omega2);
        float cs2 = cosf(omega2);
        float alpha2 = sn2 / (2.0f * RESONANCE);

        // Compute the bandpass filter coefficients (Biquad)
        filterB02 = alpha2;
        filterB12 = 0.0f;
        filterB22 = -alpha2;
        filterA02 = 1.0f + alpha2;
        filterA12 = -2.0f * cs2;
        filterA22 = 1.0f - alpha2;

        // Normalize coefficients
        filterB02 /= filterA02;
        filterB12 /= filterA02;
        filterB22 /= filterA02;
        filterA12 /= filterA02;
        filterA22 /= filterA02;

        // Apply the filter to the input signal
        float output2 = filterB02 * input2 + filterB12 * filterX12 + filterB22 * filterX22
                       - filterA12 * filterY12 - filterA22 * filterY22;

        // Update the filter states
        filterX22 = filterX12;
        filterX12 = input2;
        filterY22 = filterY12;
        filterY12 = output2;

        return output2;
    }
	
	float mapToRange(float value, float oldMin, float oldMax, float newMin, float newMax) {
		return newMin + (newMax - newMin) * ((value - oldMin) / (oldMax - oldMin));
	}

	void process(const ProcessArgs& args) override {
		// Hiss 1
		float cvInput = inputs[HISS_1_CV_INPUT].getVoltage();
		float normalizedCV = (cvInput + 5.0f) / 10.0f;
		float knob1Param = params[HISS_1__PARAM].getValue() + 0.05;
		float knob1Value = knob1Param + (normalizedCV - 0.5f);
		float controlValue = clamp(knob1Value, 0.0f, 1.0f);
		float clockSpeed1 = mapToRange(controlValue, 0.0f, 1.0f, 80.0f, 8000.0f);
		float cutoffFrequency1 = mapToRange(controlValue, 0.0f, 1.0f, 500.0f, 10000.0f);
        clockTime1 += args.sampleTime * clockSpeed1;

		// Tail 1
		float cvInput2 = inputs[TAIL_1_CV_INPUT].getVoltage();
		float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f;
		float knob2Param = params[TAIL_1__PARAM].getValue() + 0.05;
		float knob2Value = knob2Param + (normalizedCV2 - 0.5f);
		float controlValue2 = clamp(knob2Value, 0.0f, 1.0f);
        float maxTail = 100.0f; 

				// Hiss 2
				float cvInput3 = inputs[HISS_2_CV_INPUT].getVoltage();
				float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f;
				float knob3Param = params[HISS_2__PARAM].getValue() + 0.05;
				float knob3Value = knob3Param + (normalizedCV3 - 0.5f);
				float controlValue3 = clamp(knob3Value, 0.0f, 1.0f);
				float clockSpeed2 = mapToRange(controlValue3, 0.0f, 1.0f, 80.0f, 8000.0f);
				float cutoffFrequency2 = mapToRange(controlValue3, 0.0f, 1.0f, 500.0f, 10000.0f);
				clockTime2 += args.sampleTime * clockSpeed2;
		
				// Tail 2
				float cvInput4 = inputs[TAIL_2_CV_INPUT].getVoltage();
				float normalizedCV4 = (cvInput4 + 5.0f) / 10.0f;
				float knob4Param = params[TAIL_2_PARAM].getValue() + 0.05;
				float knob4Value = knob4Param + (normalizedCV4 - 0.5f);
				float controlValue4 = clamp(knob4Value, 0.0f, 1.0f);

		bool snarlSwitch = params[SNARL_PARAM].getValue() > 0.5f;
		if(!snarlSwitch){
		maxTail = 100.0f;
		}
		else{
		maxTail = 2000.0f; 
		}

		float decayTime1 = 5 + mapToRange(controlValue2, 0.0f, 1.0f, 5.0f, maxTail);
        decayAlpha1 = exp(-1.0f / (args.sampleRate * (decayTime1 / 1000.0f)));

		float decayTime2 = 5 + mapToRange(controlValue4, 0.0f, 1.0f, 5.0f, maxTail);
        decayAlpha2 = exp(-1.0f / (args.sampleRate * (decayTime2 / 1000.0f)));

    		// Check if the trigger input is high
		bool currentTriggerState1 = inputs[BANG_1_INPUT].getVoltage() > 0.5f;
		bool bangRisingEdge1 = !triggerStates1[0] && currentTriggerState1;
		triggerStates1[0] = triggerStates1[1];
		triggerStates1[1] = currentTriggerState1;

 		// Check if the trigger input is high
		 bool currentTriggerState2 = inputs[BANG_2_INPUT].getVoltage() > 0.5f;
		 bool bangRisingEdge2 = !triggerStates2[0] && currentTriggerState2;
		 triggerStates2[0] = triggerStates2[1];
		 triggerStates2[1] = currentTriggerState2;
 

        // Envelope logic
        if (bangRisingEdge1) {
            pulseTriggered1 = true;
            envelopeValue1 = 1.0f;
            pulseTime1 = decayTime1 * (args.sampleRate / 1000.0f);
        }

        // Envelope logic
        if (bangRisingEdge2) {
            pulseTriggered2 = true;
            envelopeValue2 = 1.0f;
            pulseTime2 = decayTime2 * (args.sampleRate / 1000.0f);
        }

        // Apply the decay over time
        if (pulseTriggered1) {
            envelopeValue1 *= decayAlpha1;

            if (envelopeValue1 < 0.01f) {
                envelopeValue1 = 0.0f;
                pulseTriggered1 = false;
            }
        } else {
            envelopeValue1 = 0.0f;
        }

		 if (pulseTriggered2) {
            envelopeValue2 *= decayAlpha2;

            if (envelopeValue2 < 0.01f) {
                envelopeValue2 = 0.0f;
                pulseTriggered2 = false;
            }
        } else {
            envelopeValue2 = 0.0f;
        }

        // Sample and hold the white noise value (clocked by HISS_1)
        if (clockTime1 >= 1.0f) {
            sampleAndHoldValue1 = whiteNoise1();
            clockTime1 -= 1.0f;
        }
			 if (clockTime2 >= 1.0f) {
				sampleAndHoldValue2 = whiteNoise2();
				clockTime2 -= 1.0f;
			}

        // Apply resonant bandpass filter to the white noise value
        float filteredNoise1 = biquadBandpassFilter1(sampleAndHoldValue1, cutoffFrequency1, args.sampleRate);
		float filteredNoise2 = biquadBandpassFilter2(sampleAndHoldValue2, cutoffFrequency2, args.sampleRate);

        // Modulate the output with the decay envelope value
        float outputSignal1 = filteredNoise1 * envelopeValue1;
		float outputSignal2 = filteredNoise2 * envelopeValue2;

		float finalOutput = (outputSignal1 + outputSignal2) * 0.5f;

		finalOutput = clamp(finalOutput, -5.0f, 5.0f);

        // Output the processed and decayed signal
        outputs[BOBCAT_OUTPUT].setVoltage(finalOutput);

		float ledBrightness = (envelopeValue1 + envelopeValue2);  // Map -5V to +5V range to 0-1
		lights[LED_LIGHT].setBrightnessSmooth(ledBrightness, args.sampleTime);  // Set LED brightness	
    }
};

struct BobcatWidget : ModuleWidget {
	BobcatWidget(Bobcat* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Bobcat.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKSSHorizontal>(mm2px(Vec(30.981, 46.558)), module, Bobcat::SNARL_PARAM));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(13.147, 26.401)), module, Bobcat::TAIL_1__PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(49.071, 26.35)), module, Bobcat::HISS_1__PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(13.055, 61.672)), module, Bobcat::TAIL_2_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(49.088, 61.755)), module, Bobcat::HISS_2__PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.194, 95.853)), module, Bobcat::BANG_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.061, 95.853)), module, Bobcat::TAIL_1_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.761, 95.87)), module, Bobcat::HISS_1_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.194, 109.016)), module, Bobcat::BANG_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.077, 109.066)), module, Bobcat::TAIL_2_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.695, 108.967)), module, Bobcat::HISS_2_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(51.916, 103.013)), module, Bobcat::BOBCAT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.733, 22.497)), module, Bobcat::LED_LIGHT));
	}
};


Model* modelBobcat = createModel<Bobcat, BobcatWidget>("Bobcat");