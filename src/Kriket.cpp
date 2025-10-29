#include "plugin.hpp"

struct VoltageControlledOscillator {
    bool analog = false;
    float phase = 0.f; 
    float freq = 0.f;
    float pulseWidth = 0.5f;

    float sqrValue = 0.f;

    void setPulseWidth(float pulseWidth) {
        const float pwMin = 0.01f;
        this->pulseWidth = simd::clamp(pulseWidth, pwMin, 1.f - pwMin);
    }

    void process(float deltaTime) {
        // Advance phase with oversampling for smoother transitions
        float deltaPhase = simd::clamp(freq * deltaTime, 0.f, 0.35f);
        phase += deltaPhase;
        phase -= simd::floor(phase); // Wrap phase

        // Direct square wave calculation based on the pulse width
        sqrValue = sqr(phase);
    }

    float sqr(float phase) {
        return simd::ifelse(phase < pulseWidth, 1.f, -1.f);
    }

    float sqr() {
        return sqrValue;
    }
};

struct Kriket : Module {
    enum ParamId {
        PITCH1_PARAM,
        PITCH2_PARAM,
        PITCH3_PARAM,
        PITCH4_PARAM,
        CUTOFF_PARAM, // Optional parameter for variable cutoff frequency
        PARAMS_LEN
    };
    enum InputId {
        BANG1_INPUT,
        BANG2_INPUT,
        BANG3_INPUT,
        BANG4_INPUT,
        CVIN_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LED_LIGHT,
        LIGHTS_LEN
    };

    VoltageControlledOscillator vco1, vco2, vco3, vco4;

    // High-pass filter variables
    float hpFilterPrevInput1 = 0.0f;
    float hpFilterPrevOutput1 = 0.0f;
    float hpFilterPrevInput2 = 0.0f;
    float hpFilterPrevOutput2 = 0.0f;
    float hpFilterPrevInput3 = 0.0f;
    float hpFilterPrevOutput3 = 0.0f;
    float hpFilterPrevInput4 = 0.0f;
    float hpFilterPrevOutput4 = 0.0f;

    // Low-pass filter variables (for each oscillator)
    float lpFilterPrevInput1 = 0.0f;
    float lpFilterPrevOutput1 = 0.0f;
    float lpFilterPrevInput2 = 0.0f;
    float lpFilterPrevOutput2 = 0.0f;
    float lpFilterPrevInput3 = 0.0f;
    float lpFilterPrevOutput3 = 0.0f;
    float lpFilterPrevInput4 = 0.0f;
    float lpFilterPrevOutput4 = 0.0f;

    const float hpCutoffFreq = 100.0f;  // 100 Hz cutoff for high-pass filter
    const float sampleRate = 44100.0f;  // Assume 44.1 kHz sample rate

    Kriket() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(PITCH1_PARAM, 0.f, 1.f, 0.f, "Pitch 1");
        configParam(PITCH2_PARAM, 0.f, 1.f, 0.f, "Pitch 2");
        configParam(PITCH3_PARAM, 0.f, 1.f, 0.f, "Pitch 3");
        configParam(PITCH4_PARAM, 0.f, 1.f, 0.f, "Pitch 4");
        configInput(BANG1_INPUT, "Bang! 1");
        configInput(BANG2_INPUT, "Bang! 2");
        configInput(BANG3_INPUT, "Bang! 3");
        configInput(BANG4_INPUT, "Bang! 4");
        configInput(CVIN_INPUT, "CV");
        configOutput(OUT_OUTPUT, "Kriket");
    }

    // High-pass filter processing for individual oscillator outputs
    float highPassFilter(float input, float& prevInput, float& prevOutput) {
        float RC = 1.0f / (hpCutoffFreq * 2.0f * M_PI);
        float dt = 1.0f / sampleRate;
        float alpha = RC / (RC + dt);

        float output = alpha * (input - prevInput) + (1 - alpha) * prevOutput;

        prevInput = input;
        prevOutput = output;

        return output;
    }

    // Low-pass filter processing for individual oscillator outputs
    float lowPassFilter(float input, float& prevInput, float& prevOutput, float cutoffFreq) {
        // Calculate the dynamic low-pass filter alpha based on the cutoff frequency
        float RC = 1.0f / (cutoffFreq * 2.0f * M_PI);
        float dt = 1.0f / sampleRate;
        float alpha = dt / (RC + dt);

        // Apply low-pass filter equation
        float output = alpha * input + (1 - alpha) * prevOutput;

        prevInput = input;
        prevOutput = output;

        return output;
    }

    void process(const ProcessArgs& args) override {
        float cvInput = inputs[CVIN_INPUT].getVoltage();
        cvInput = std::clamp(cvInput, 0.0f, 5.0f);

        // Map parameters to frequency range of 1kHz to 5kHz
        float freq1 = 1000.0f + (4000.0f) * params[PITCH1_PARAM].getValue();
        float freq2 = 1000.0f + (4000.0f) * params[PITCH2_PARAM].getValue();
        float freq3 = 1000.0f + (4000.0f) * params[PITCH3_PARAM].getValue();
        float freq4 = 1000.0f + (4000.0f) * params[PITCH4_PARAM].getValue();

        // Apply CV input as a 1V/octave offset
        freq1 = std::clamp(freq1 * std::pow(2.0f, cvInput), 1000.0f, 5000.0f);
        freq2 = std::clamp(freq2 * std::pow(2.0f, cvInput), 1000.0f, 5000.0f);
        freq3 = std::clamp(freq3 * std::pow(2.0f, cvInput), 1000.0f, 5000.0f);
        freq4 = std::clamp(freq4 * std::pow(2.0f, cvInput), 1000.0f, 5000.0f);

        bool bang1 = inputs[BANG1_INPUT].isConnected() && inputs[BANG1_INPUT].getVoltage() > 0.5f;
        bool bang2 = inputs[BANG2_INPUT].isConnected() && inputs[BANG2_INPUT].getVoltage() > 0.5f;
        bool bang3 = inputs[BANG3_INPUT].isConnected() && inputs[BANG3_INPUT].getVoltage() > 0.5f;
        bool bang4 = inputs[BANG4_INPUT].isConnected() && inputs[BANG4_INPUT].getVoltage() > 0.5f;

        float timeStep = args.sampleTime;

        // Set frequency for each oscillator
        vco1.freq = freq1;
        vco2.freq = freq2;
        vco3.freq = freq3;
        vco4.freq = freq4;

        vco1.setPulseWidth(0.5f);
        vco2.setPulseWidth(0.5f);
        vco3.setPulseWidth(0.5f);
        vco4.setPulseWidth(0.5f);

        vco1.process(timeStep);
        vco2.process(timeStep);
        vco3.process(timeStep);
        vco4.process(timeStep);

        // Apply high-pass filter
        float output1 = highPassFilter(vco1.sqr(), hpFilterPrevInput1, hpFilterPrevOutput1);
        float output2 = highPassFilter(vco2.sqr(), hpFilterPrevInput2, hpFilterPrevOutput2);
        float output3 = highPassFilter(vco3.sqr(), hpFilterPrevInput3, hpFilterPrevOutput3);
        float output4 = highPassFilter(vco4.sqr(), hpFilterPrevInput4, hpFilterPrevOutput4);

        // Determine the highest oscillator frequency to set low-pass filter cutoff
        float maxFreq = std::max({freq1, freq2, freq3, freq4});

        // Apply low-pass filter to each oscillator output
        output1 = lowPassFilter(output1, lpFilterPrevInput1, lpFilterPrevOutput1, maxFreq);
        output2 = lowPassFilter(output2, lpFilterPrevInput2, lpFilterPrevOutput2, maxFreq);
        output3 = lowPassFilter(output3, lpFilterPrevInput3, lpFilterPrevOutput3, maxFreq);
        output4 = lowPassFilter(output4, lpFilterPrevInput4, lpFilterPrevOutput4, maxFreq);

        // Sum outputs based on bang states
        float output = 0.0f;
        output += bang1 ? output1 : 0.0f;
        output += bang2 ? output2 : 0.0f;
        output += bang3 ? output3 : 0.0f;
        output += bang4 ? output4 : 0.0f;

        float finalOutput = output * 4.0f; 
        finalOutput = std::clamp(finalOutput, -3.0f, 3.0f);

        // Send the output signal
        outputs[OUT_OUTPUT].setVoltage(finalOutput); 

        // LED feedback on Bang states
        lights[LED_LIGHT].setBrightnessSmooth((bang1 || bang2 || bang3 || bang4 ? 1.0f : 0.0f), args.sampleTime);
    }
};

struct KriketWidget : ModuleWidget {
    KriketWidget(Kriket* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Kriket.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.34, 17.91)), module, Kriket::PITCH1_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.241, 40.866)), module, Kriket::PITCH2_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.246, 63.777)), module, Kriket::PITCH3_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(27.359, 86.644)), module, Kriket::PITCH4_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.704, 17.947)), module, Kriket::BANG1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.747, 41.098)), module, Kriket::BANG2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.596, 64.098)), module, Kriket::BANG3_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.369, 86.966)), module, Kriket::BANG4_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.596, 109.399)), module, Kriket::CVIN_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(29.698, 109.658)), module, Kriket::OUT_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.887, 99.326)), module, Kriket::LED_LIGHT));
    }
};

Model* modelKriket = createModel<Kriket, KriketWidget>("Kriket");

