#include "plugin.hpp"

struct DialUp : Module {
    // Parameters
    enum ParamId {
        TAIL_PARAM,       // Tail knob for decay time control
        RANGE_PARAM,      // Range switch for decay time
        MODEM_PARAM,
        PARAMS_LEN
    };

    // Inputs
    enum InputId {
        BANG_INPUT,
        MODEM_CV_INPUT,
        INPUTS_LEN
    };

    // Outputs
    enum OutputId {
        DIAL_UP_OUTPUT,
        OUTPUTS_LEN
    };

    // Lights
    enum LightId {
        LED_LIGHT,
        LIGHTS_LEN
    };

    // Internal variables for bytebeat
    float t = 0.0f; // Time variable for bytebeat (accumulator)
    int note = 1; // Default note value (range 1-32)

    // Jitter control variables
    float jitterValue = 0.0f;  // Current jitter value
    int jitterCounter = 0;     // Counter to control jitter update speed
    const int jitterSpeed = 500; // Fixed jitter speed, number of samples before updating jitter (higher = slower)
    float noiseLevel = 0.001f; // Level of noise added to the control value
    
    // Low-pass/high-pass filter variables
    float previousLowPassOutput = 0.0f; 
    float previousHighPassOutput = 0.0f;
    float previousInput = 0.0f; 

    // Envelope control variables for decay
    float envelopeValue = 1.0f;  // Envelope output value (for volume control)
    float decayTime = 5.0f;      // Decay time in ms (5ms as requested)
    float decayAlpha = 0.0f;     // Exponential decay coefficient
    bool pulseTriggered = false; // Flag to check if pulse was triggered
    float pulseTime = 0.0f;      // Time tracking for pulse duration
    bool lastBangState = false;  // Previous state of the Bang input
    float sampleRate = 44100.0f; // Default sample rate (you can modify this if needed)

    // Constructor for DialUp module
    DialUp() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(TAIL_PARAM, 0.f, 1.f, 0.f, "Decay Time", " %", 0.f, 100.f);
        configSwitch(RANGE_PARAM, 0.f, 2.f, 0.f, "Decay Range", {"Fast", "Medium", "Slow"});
        configParam(MODEM_PARAM, 0.f, 1.f, 0.5f, "Modem", " %", 0.f, 100.f);
        configInput(BANG_INPUT, "Bang Gate");
        configInput(MODEM_CV_INPUT, "Modem CV");
        configOutput(DIAL_UP_OUTPUT, "Dial-Up Audio");
    }

    // Process function (audio generation logic)
    void process(const ProcessArgs &args) override {
        float cvInput = inputs[MODEM_CV_INPUT].getVoltage();  // Read CV input
        float normalizedCV = (cvInput + 5.0f) / 10.0f; // Map -5V -> 0.0 and 5V -> 1.0
        float knob1Param = params[MODEM_PARAM].getValue() + 0.05;  // Original knob value
        float knob1Value = knob1Param + (normalizedCV - 0.5f);  // Apply the CV input as an offset
        float controlValue = std::clamp(knob1Value, 0.0f, 1.0f);
        
        jitterCounter++;
        if (jitterCounter >= jitterSpeed) {
            jitterCounter = 0; // Reset counter
            jitterValue += (random::uniform() * 2.0f - 1.0f) * noiseLevel; // Add random jitter
            jitterValue = std::clamp(jitterValue, -1.0f, 1.0f); // Clamp jitter to stay within range
        }

        // Apply the jittered control value to the original parameter value
        controlValue += jitterValue;

        // Clamp the noisy control value to stay between 0 and 1
        controlValue = std::clamp(controlValue, 0.0f, 1.0f);

        float scaleFactor = args.sampleRate / 8000.0f; // Fix the output to 8kHz

        t += controlValue * scaleFactor;  // Increment time for the next sample

        // Map the control value from the modem knob to the note range (1-32)
        note = (int)((1.0f - controlValue) * 255.0f + 1.0f); // Reverse the range and map it to 1-32

        // Bytebeat algorithm
        int result = (int(t)*(note)) * (int(t)*(note)) ^ (int(t)*(note)) >> 8;

        // Map the result to a range of -5V to +5V
        float mappedResult = ((result & 1) * 10.0f) - 5.0f;
        
        // Apply low-pass filter for anti-aliasing at 10 kHz
        float lowPassCutoffFreq = 10000.0f;  // 10 kHz cutoff
        float lowPassAlpha = args.sampleRate / (args.sampleRate + 2.0f * M_PI * lowPassCutoffFreq);  // Low-pass filter coefficient

        float lowPassFilteredOutput = lowPassAlpha * mappedResult + (1.0f - lowPassAlpha) * previousLowPassOutput; // Low-pass filter

        // Apply high-pass filter for 100 Hz cutoff
        float highPassCutoffFreq = 100.0f;  // 100 Hz cutoff
        float highPassAlpha = args.sampleRate / (args.sampleRate + 2.0f * M_PI * highPassCutoffFreq);  // High-pass filter coefficient

        // High-pass filter equation
        float highPassFilteredOutput = highPassAlpha * (previousHighPassOutput + lowPassFilteredOutput - previousInput);

        // Store the filtered output and input for the next sample
        previousInput = lowPassFilteredOutput;
        previousHighPassOutput = highPassFilteredOutput;
        previousLowPassOutput = lowPassFilteredOutput;

        // Adjust decay time based on RANGE_PARAM
        int range = (int)params[RANGE_PARAM].getValue(); // Read the RANGE_PARAM value
		switch (range) {
			case 0:
				decayTime = 5.0f + (params[TAIL_PARAM].getValue() * 25.0f); // 5ms to 30ms range
				break;
			case 1:
				decayTime = 30.0f + (params[TAIL_PARAM].getValue() * 170.0f); // 30ms to 200ms range
				break;
			case 2:
				decayTime = 200.0f + (params[TAIL_PARAM].getValue() * 3800.0f); // 200ms to 4000ms range
				break;
		}

        // Calculate decayAlpha based on the decayTime (in ms)
        decayAlpha = exp(-1.0f / (args.sampleRate * (decayTime / 1000.0f))); // Calculate decay coefficient based on decay time

        // Rising edge detection for the BANG_INPUT
        bool bangState = inputs[BANG_INPUT].getVoltage() > 0.5f;
        if (bangState && !lastBangState) {
            // Trigger the envelope when Bang goes from low to high
            pulseTriggered = true;
            envelopeValue = 1.0f;  // Start the envelope with full amplitude
            pulseTime = decayTime * (args.sampleRate / 1000.0f);
        }

        // Update the lastBangState for the next cycle
        lastBangState = bangState;

        // If pulse is active, begin decay envelope
        if (pulseTriggered) {
            envelopeValue *= decayAlpha;  // Apply exponential decay

            if (envelopeValue < 0.01f) {  // End decay when value is small enough
                envelopeValue = 0.0f;
                pulseTriggered = false;
            }
        } else {
            envelopeValue = 0.0f; // Reset envelope when no pulse is triggered
        }

        // Final output is the filtered signal multiplied by the envelope value
        float finalOutput = highPassFilteredOutput * envelopeValue;

        finalOutput = std::clamp(finalOutput, -3.0f, 3.0f);

        // Output the result to the output port
        outputs[DIAL_UP_OUTPUT].setVoltage(finalOutput);  // Output audio as a control voltage

        // Map final output to LED light (0.0 to 1.0 range for LED)
        float ledBrightness = envelopeValue;  // Map -5V to +5V range to 0-1
        lights[LED_LIGHT].setBrightnessSmooth(ledBrightness, args.sampleTime);  // Set LED brightness
    }
};

struct DialUpWidget : ModuleWidget {
    DialUpWidget(DialUp* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/DialUp.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.15, 17.569)), module, DialUp::TAIL_PARAM));
        addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(10.15, 35.230)), module, DialUp::RANGE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.15, 52.693)), module, DialUp::MODEM_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.15, 85.071)), module, DialUp::BANG_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.15, 99.656)), module, DialUp::MODEM_CV_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.15, 113.171)), module, DialUp::DIAL_UP_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(10.15, 75.116)), module, DialUp::LED_LIGHT));
    }
};

Model* modelDialUp = createModel<DialUp, DialUpWidget>("DialUp");