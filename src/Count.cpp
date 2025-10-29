#include "plugin.hpp"

struct ClockModule : Module {
    // Parameters from the provided Count module
    enum ParamIds {
        TOP_BOTTOM_PARAM,
        COARSE_PARAM,
        RANGE_PARAM,
        FINE_PARAM,
        PULSEWIDTH_PARAM,
        START_STOP_PARAM,
        UPBEAT_DOWNBEAT_PARAM,
        LEFT_RIGHT_PARAM,
        NUM_PARAMS
    };

    // Inputs and outputs
    enum InputIds {
        TIME_INPUT,
        WIDTH_INPUT,
        NUM_INPUTS
    };

    enum OutputIds {
        DIVIDED_1_OUTPUT,
        DIVIDED_2_OUTPUT,
        DIVIDED_3_OUTPUT,
        DIVIDED_4_OUTPUT,
        START_OUTPUT,
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightId {
        LED_1_LIGHT,
        LED_2_LIGHT,
        LED_3_LIGHT,
        LED_4_LIGHT,
        LED_5_LIGHT,
        LED_6_LIGHT,
        NUM_LIGHTS
    };

    ClockModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(COARSE_PARAM, 0.f, 1.f, 0.5f, "Coarse Speed", " %", 0.f, 100.f); 
        configParam(FINE_PARAM, -1.f, 1.f, 0.f, "Fine Speed Adjust", " ms", 0.f, 5.f); // 
        configParam(PULSEWIDTH_PARAM, 0.f, 1.f, 0.5f, "Pulse Width", " %", 10.f, 5.f); // Get values 
        configSwitch(START_STOP_PARAM, 0.f, 1.f, 1.f, "Start/Stop", {"Stop", "Start"});
        configSwitch(UPBEAT_DOWNBEAT_PARAM, 0.f, 1.f, 0.f, "Upbeat/Downbeat", {"Downbeat", "Upbeat"});
        configSwitch(LEFT_RIGHT_PARAM, 0.f, 1.f, 0.f, "Left/Right", {"Left", "Right"});
        configSwitch(TOP_BOTTOM_PARAM, 0.f, 1.f, 0.f, "Top/Bottom", {"Bottom", "Top"});
        configSwitch(RANGE_PARAM, 0.f, 1.f, 1.f, "Range", {"Slow", "Fast"});

        configInput(TIME_INPUT, "Time CV");
        configInput(WIDTH_INPUT, "Width CV");
        configOutput(DIVIDED_1_OUTPUT, "Divided 1 Gate");
        configOutput(DIVIDED_2_OUTPUT, "Divided 2 Gate");
        configOutput(DIVIDED_3_OUTPUT, "Divided 3 Gate");
        configOutput(DIVIDED_4_OUTPUT, "Divided 4 Gate");
        configOutput(START_OUTPUT, "Start Pulse Gate");
        configOutput(MAIN_OUTPUT, "Main Gate");
    }

    // Variables for controlling the LED blink and output
float clockTimer = 0.0f;  // Timer to control the blink
float clockDelay = 1000.0f;  // Delay between blinks (in milliseconds)
float pulseWidth = 0.5f;  // Duration of the on/off time (0 to 1)
     
bool gateHigh = false;    // Gate state (high or low)
bool gate2High = false;    // Gate state (high or low)
bool gate3High = false;    // Gate state (high or low)
bool gate4High = false;    // Gate state (high or low)
bool gate5High = false;    // Gate state (high or low)

bool mainOutputHigh = false; 
bool divided1High = false;
bool divided2High = false;
bool divided3High = false;
bool divided4High = false;

bool pulseActive = false; // To control one-shot pulse activation
float pulseTimer = 0.0f;  // Timer for the one-shot pulse duration
int masterCount = 1; 
int ch1 = 4; 
int ch2 = 8;
int ch3 = 12;
int ch4 = 16;
int masterDivide = 1; 

float oneShotTimer = 0.0f;
bool prevStartStop = false;
bool pulseSent = false;
bool resetFlag = false;

void process(const ProcessArgs& args) override {
    // Get values from the knobs and controls
    float fineTuneParam = params[FINE_PARAM].getValue();
    float pulseWidthParam = params[PULSEWIDTH_PARAM].getValue();
    bool rangeSwitch = params[RANGE_PARAM].getValue() > 0.5f;
    bool startStop = params[START_STOP_PARAM].getValue() > 0.5f;
    bool topBottom = params[TOP_BOTTOM_PARAM].getValue() > 0.5f;
    bool leftRight = params[LEFT_RIGHT_PARAM].getValue() > 0.5f;
    bool upbeatDownbeat = params[UPBEAT_DOWNBEAT_PARAM].getValue() > 0.5f;

    // Get the coarse frequency knob value
    float coarseFrequencyKnob = params[COARSE_PARAM].getValue();
    float scaledKnob = 1.0f / (1.0f + expf(-10.0f * (coarseFrequencyKnob - 0.25f)));
    scaledKnob = 1.0f - scaledKnob;
    scaledKnob = clamp(scaledKnob, 0.0f, 1.0f);
    scaledKnob = 1.0f - scaledKnob;
    
    float timeInputVoltage = inputs[TIME_INPUT].getVoltage();
    float cvCoarseFrequency = (timeInputVoltage + 5.0f) / 10.0f;

    float coarseFrequency = scaledKnob + cvCoarseFrequency - 0.5f;
    coarseFrequency = clamp(coarseFrequency, 0.0f, 1.0f);

    // Handle start/stop logic with reset logic on "start/stop" being off
    if (!startStop && prevStartStop) {
        if (!resetFlag) {
            masterCount = 1;
            clockTimer = 0.0f;
            ch1 = 4;
            ch2 = 8;
            ch3 = 12;
            ch4 = 16;
            pulseSent = false;

            // Reset gates and LEDs
            gateHigh = false;
            gate2High = false;
            gate3High = false;
            gate4High = false;
            gate5High = false;

            lights[LED_1_LIGHT].setBrightness(0.0f);
            lights[LED_2_LIGHT].setBrightness(0.0f);
            lights[LED_3_LIGHT].setBrightness(0.0f);
            lights[LED_4_LIGHT].setBrightness(0.0f);
            lights[LED_5_LIGHT].setBrightness(0.0f);
            lights[LED_6_LIGHT].setBrightness(0.0f);

            resetFlag = true;  // Mark that reset was done
        }
        outputs[MAIN_OUTPUT].setVoltage(0.0f);
    } else if (startStop) {
        resetFlag = false;
        if (startStop && !prevStartStop && !pulseSent) {
            outputs[START_OUTPUT].setVoltage(5.0f);
            lights[LED_1_LIGHT].setBrightnessSmooth(5.0f, args.sampleTime);
            oneShotTimer = 100.0f;
            pulseSent = true;
        }

        if (oneShotTimer > 0.0f) {
            oneShotTimer -= args.sampleTime * 1000.0f;
            if (oneShotTimer <= 0.0f) {
                outputs[START_OUTPUT].setVoltage(0.0f);
                lights[LED_1_LIGHT].setBrightness(0.0f);
            }
        }

        // Adjust clockDelay based on coarse frequency and fine-tuning
        if (!rangeSwitch) {
            clockDelay = (1.0f - coarseFrequency) * 2500.0f + 500.0f;
        } else {
            clockDelay = (1.0f - coarseFrequency) * 475.0f + 25.0f;
        }

        float fineTuneAdjustment = -fineTuneParam * 5.0f;
        clockDelay += fineTuneAdjustment;

        // Adjust pulse width
        float widthInputVoltage = inputs[WIDTH_INPUT].getVoltage();
        float cvPulseWidth = (widthInputVoltage + 5.0f) / 10.0f;
        pulseWidth = pulseWidthParam * 0.5f + cvPulseWidth * 0.5f;
        pulseWidth = clamp(pulseWidth, 0.0f, 1.0f);

        // Increment clock timer
        clockTimer += args.sampleTime * 1000.0f;
        float period = clockDelay;
        float onTime = period * pulseWidth;

        if (clockTimer >= period) {
            masterCount++;  // Increment master count once per cycle
            clockTimer -= period;  // Reset clockTimer to sync with next cycle
            gateHigh = !gateHigh;  // Toggle clock gate
        }

        if (gateHigh) {
            if (clockTimer < onTime) {
                mainOutputHigh = true;
            } else {
                mainOutputHigh = false;
            }
        }

        // Handling division and beat division for upbeat/downbeat
        int adjustedMasterCount = masterCount;
        if (upbeatDownbeat) {
            adjustedMasterCount--;  // Delay divided outputs by 1 pulse
        }

        // Adjust division based on left/right and top/bottom settings
        if (leftRight == false) {
            masterDivide = 1;
        } else {
            masterDivide = 4;
        }

        if (topBottom == false) {
            ch1 = 4 * masterDivide;
            ch2 = 8 * masterDivide;
            ch3 = 12 * masterDivide;
            ch4 = 16 * masterDivide;
        }

        if (topBottom == true) {
            ch1 = 6 * masterDivide;
            ch2 = 10 * masterDivide;
            ch3 = 14 * masterDivide;
            ch4 = 18 * masterDivide;
        }

        // Divided Output 1 (Divide by 2)
        if (adjustedMasterCount % ch1 == 0) {
            if (clockTimer < onTime) {
                divided1High = true;  // Gate high
            } else {
                divided1High = false;  // Gate low
            }
        } else {
            divided1High = false;  // Gate low
        }

        // Divided Output 2 (Divide by 4)
        if (adjustedMasterCount % ch2 == 0) {
            if (clockTimer < onTime) {
                divided2High = true;  // Gate high
            } else {
                divided2High = false;  // Gate low
            }
        } else {
            divided2High = false;  // Gate low
        }

        // Divided Output 3 (Divide by 8)
        if (adjustedMasterCount % ch3 == 0) {
            if (clockTimer < onTime) {
                divided3High = true;  // Gate high
            } else {
                divided3High = false;  // Gate low
            }
        } else {
            divided3High = false;  // Gate low
        }

        // Divided Output 4 (Divide by 16)
        if (adjustedMasterCount % ch4 == 0) {
            if (clockTimer < onTime) {
                divided4High = true;  // Gate high
            } else {
                divided4High = false;  // Gate low
            }
        } else {
            divided4High = false;  // Gate low
        }

        // Set voltages for all outputs based on boolean values
        outputs[DIVIDED_1_OUTPUT].setVoltage(divided1High ? 5.0f : 0.0f);
        outputs[DIVIDED_2_OUTPUT].setVoltage(divided2High ? 5.0f : 0.0f);
        outputs[DIVIDED_3_OUTPUT].setVoltage(divided3High ? 5.0f : 0.0f);
        outputs[DIVIDED_4_OUTPUT].setVoltage(divided4High ? 5.0f : 0.0f);
        outputs[MAIN_OUTPUT].setVoltage(mainOutputHigh ? 5.0f : 0.0f);

        // Directly set the LED brightness to the same voltage as the outputs
        lights[LED_2_LIGHT].setBrightnessSmooth(divided1High, args.sampleTime);
        lights[LED_3_LIGHT].setBrightnessSmooth(divided2High, args.sampleTime);
        lights[LED_5_LIGHT].setBrightnessSmooth(divided3High, args.sampleTime); 
        lights[LED_6_LIGHT].setBrightnessSmooth(divided4High, args.sampleTime);
        lights[LED_4_LIGHT].setBrightnessSmooth(mainOutputHigh, args.sampleTime);
    }

    // Store the previous start/stop state for the next iteration
    prevStartStop = startStop;
}
};

// The UI widget (module panel) code, integrated with the interface controls
struct ClockModuleWidget : ModuleWidget {
    ClockModuleWidget(ClockModule* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Count.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Knobs for controls
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(20.702, 16.846)), module, ClockModule::COARSE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.643, 35.352)), module, ClockModule::FINE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.718, 53.746)), module, ClockModule::PULSEWIDTH_PARAM));

        // Switches for toggling features
        addParam(createParamCentered<CKSS>(mm2px(Vec(35.126, 28.063)), module, ClockModule::RANGE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(5.619, 73.39)), module, ClockModule::START_STOP_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(15.351, 73.36)), module, ClockModule::UPBEAT_DOWNBEAT_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(25.395, 73.468)), module, ClockModule::TOP_BOTTOM_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(35.094, 73.63)), module, ClockModule::LEFT_RIGHT_PARAM));

        // Input ports for external control
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.698, 27.387)), module, ClockModule::TIME_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.755, 53.671)), module, ClockModule::WIDTH_INPUT));

        // Output ports for divided clock signals and main output
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.845, 92.355)), module, ClockModule::START_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.783, 109.865)), module, ClockModule::MAIN_OUTPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.689, 92.499)), module, ClockModule::DIVIDED_1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.299, 92.459)), module, ClockModule::DIVIDED_2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.653, 109.86)), module, ClockModule::DIVIDED_3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.335, 109.842)), module, ClockModule::DIVIDED_4_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.755, 84)), module, ClockModule::LED_1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.689, 84)), module, ClockModule::LED_2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.299, 84)), module, ClockModule::LED_3_LIGHT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(5.755, 102)), module, ClockModule::LED_4_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.689, 102)), module, ClockModule::LED_5_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(35.299, 102)), module, ClockModule::LED_6_LIGHT));
    }
};

// Define the module instance
Model* modelCount = createModel<ClockModule, ClockModuleWidget>("Count");
