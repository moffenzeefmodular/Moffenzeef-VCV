#include "plugin.hpp"

struct Chirp {
    float stepSamples = 0.0f;
    int currentStep = 0;
    int seqLength = 16;
    int gates[64];
    int lastFreeze = 0;
    int freeze = 0;
    float timeRatio = 1.0f;

    Chirp() {
        for (int i = 0; i < 64; i++)
            gates[i] = rand() % 2;
    }

    void reset() {
        stepSamples = 0.0f;
        currentStep = 0;
    }

    int update(float basePeriodMs, float width, float sampleRate) {
        float msPerSample = 1.0f / (sampleRate * 0.001f);
        float stepDurationSamples = (basePeriodMs * timeRatio) / msPerSample;

        if (freeze > lastFreeze)
            seqLength = (rand() % 64) + 1;

        lastFreeze = freeze;

        stepSamples += 1.0f;

        if (stepSamples >= stepDurationSamples) {
            stepSamples -= stepDurationSamples;
            currentStep = (currentStep + 1) % seqLength;

            if (freeze == 0) {
                const int density = 20; // % chance
                gates[rand() % seqLength] = (rand() % 100 <= density);
            }
        }

        return (stepSamples < stepDurationSamples * width && gates[currentStep]) ? 1 : 0;
    }
};

struct Swarm : Module {
    enum ParamIds {
        TIMECOARSE_PARAM,
        RANGESWITCH_PARAM,
        TIMEFINE_PARAM,
        WIDTH_PARAM,
        LOOPONE_PARAM,
        LOOPTWO_PARAM,
        LOOPTHREE_PARAM,
        LOOPFOUR_PARAM,
        PARAMS_LEN
    };
    enum InputIds {
        TIMECVIN_INPUT,
        WIDTHCVIN_INPUT,
        INPUTS_LEN
    };
    enum OutputIds {
        ONEOUT_OUTPUT,
        TWOOUT_OUTPUT,
        THREEOUT_OUTPUT,
        FOUROUT_OUTPUT,
        CLOCKOUT_OUTPUT,
        VCOOUT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightIds {
        ONELED_LIGHT,
        TWOLED_LIGHT,
        THREELED_LIGHT,
        FOURLED_LIGHT,
        CLOCKLED_LIGHT,
        VCOLED_LIGHT,
        LIGHTS_LEN
    };

    Chirp chirps[4];
    bool chirpsInitialized = false;
    float clockTimer = 0.f;
    float pulseWidth = 0.5f;
    bool mainOutputHigh = false;
    float vcoPhase = 0.f;
    float vcoFreq = 100.f;

    Swarm() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(TIMECOARSE_PARAM, 0.f, 1.f, 0.5f, "Coarse Speed", "%", 0.f, 100.f);
        configSwitch(RANGESWITCH_PARAM, 0.f, 1.f, 0.f, "Range", {"Slow", "Fast"});
        configParam(TIMEFINE_PARAM, -1.f, 1.f, 0.f, "Fine Speed Adjust", "%", 0.f, 100.f);
        configParam(WIDTH_PARAM, 0.f, 1.f, 0.5f, "Width", "%", 10.f, 5.f);
        configSwitch(LOOPONE_PARAM, 0.f, 1.f, 0.f, "Loop Ch. 1", {"Off", "On"});
        configSwitch(LOOPTWO_PARAM, 0.f, 1.f, 0.f, "Loop Ch. 2", {"Off", "On"});
        configSwitch(LOOPTHREE_PARAM, 0.f, 1.f, 0.f, "Loop Ch. 3", {"Off", "On"});
        configSwitch(LOOPFOUR_PARAM, 0.f, 1.f, 0.f, "Loop Ch. 4", {"Off", "On"});
        configInput(TIMECVIN_INPUT, "Time CV");
        configInput(WIDTHCVIN_INPUT, "Width CV");
        configOutput(VCOOUT_OUTPUT, "VCO");
        configOutput(ONEOUT_OUTPUT, "Ch. 1");
        configOutput(TWOOUT_OUTPUT, "Ch. 2");
        configOutput(CLOCKOUT_OUTPUT, "Clock");
        configOutput(THREEOUT_OUTPUT, "Ch. 3");
        configOutput(FOUROUT_OUTPUT, "Ch. 4");
    }

    void process(const ProcessArgs& args) override {
        if (!chirpsInitialized) {
            chirps[0].timeRatio = 1.3f;
            chirps[1].timeRatio = 3.7f;
            chirps[2].timeRatio = 5.5f;
            chirps[3].timeRatio = 7.9f;
            chirpsInitialized = true;
        }

        float pulseWidthParam = params[WIDTH_PARAM].getValue();
        float coarse = params[TIMECOARSE_PARAM].getValue();
        float fine = params[TIMEFINE_PARAM].getValue();
        float range = params[RANGESWITCH_PARAM].getValue();

        float timeCV = clamp(inputs[TIMECVIN_INPUT].getVoltage(), -5.f, 5.f);
        float widthCV = clamp(inputs[WIDTHCVIN_INPUT].getVoltage(), -5.f, 5.f);

        float summedTime = clamp((coarse * 10.f - 5.f) + (fine * 5.f) + timeCV, -5.f, 5.f);

        float periodMs = (range < 0.5f)
            ? clamp(rescale(summedTime, -5.f, 5.f, 500.f, 150.f), 150.f, 500.f)
            : clamp(rescale(summedTime, -5.f, 5.f, 150.f, 15.f), 15.f, 150.f);

        float samplesPerMs = args.sampleRate * 0.001f;
        float periodSamples = periodMs * samplesPerMs;

        pulseWidth = clamp(pulseWidthParam + widthCV * 0.1f, 0.05f, 0.5f);
        float pulseSamples = periodSamples * pulseWidth;

        clockTimer += args.sampleTime * args.sampleRate;
        if (clockTimer >= periodSamples)
            clockTimer -= periodSamples;

        mainOutputHigh = (clockTimer < pulseSamples);
        outputs[CLOCKOUT_OUTPUT].setVoltage(mainOutputHigh ? 5.f : 0.f);
        lights[CLOCKLED_LIGHT].setBrightnessSmooth(mainOutputHigh, args.sampleTime);

        for (int i = 0; i < 4; i++) {
            chirps[i].freeze = (params[LOOPONE_PARAM + i].getValue() > 0.5f) ? 1 : 0;
            int gate = chirps[i].update(periodMs, pulseWidth, args.sampleRate);
            outputs[ONEOUT_OUTPUT + i].setVoltage(gate ? 5.f : 0.f);
            lights[ONELED_LIGHT + i].setBrightnessSmooth(gate, args.sampleTime);
        }

        vcoFreq = (range < 0.5f)
            ? clamp(rescale(summedTime, -5.f, 5.f, 40.f, 200.f), 40.f, 200.f)
            : clamp(rescale(summedTime, -5.f, 5.f, 1000.f, 15000.f), 1000.f, 15000.f);

        vcoPhase += vcoFreq * args.sampleTime;
        if (vcoPhase >= 1.f)
            vcoPhase -= 1.f;

        outputs[VCOOUT_OUTPUT].setVoltage((vcoPhase < 0.5f) ? 5.f : -5.f);
        lights[VCOLED_LIGHT].setBrightness(5.f);
    }
};

struct SwarmWidget : ModuleWidget {
    SwarmWidget(Swarm* module) {
        setModule(module);
        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/panels/Swarm.svg")));
        
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(20.616, 16.629)), module, Swarm::TIMECOARSE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(35.099, 27.847)), module, Swarm::RANGESWITCH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.616, 35.046)), module, Swarm::TIMEFINE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.616, 53.459)), module, Swarm::WIDTH_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(5.702, 72.999)), module, Swarm::LOOPONE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(15.327, 72.999)), module, Swarm::LOOPTWO_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(25.351, 72.999)), module, Swarm::LOOPTHREE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(35.099, 72.999)), module, Swarm::LOOPFOUR_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.702, 27.246)), module, Swarm::TIMECVIN_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.702, 53.463)), module, Swarm::WIDTHCVIN_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.702, 92.263)), module, Swarm::VCOOUT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.616, 92.263)), module, Swarm::ONEOUT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.378, 92.263)), module, Swarm::TWOOUT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.702, 109.747)), module, Swarm::CLOCKOUT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.616, 109.747)), module, Swarm::THREEOUT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.378, 109.747)), module, Swarm::FOUROUT_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.702, 84.018)), module, Swarm::VCOLED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.602, 84.018)), module, Swarm::ONELED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.378, 84.018)), module, Swarm::TWOLED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.702, 102.163)), module, Swarm::CLOCKLED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.602, 102.163)), module, Swarm::THREELED_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.378, 102.163)), module, Swarm::FOURLED_LIGHT));
    }
};

Model* modelSwarm = createModel<Swarm, SwarmWidget>("Swarm");
