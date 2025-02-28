#include "plugin.hpp"

// Variables for controlling the LED blink and output
float clockTimer = 0.0f;  // Timer to control the blink
float clockDelay = 1000.0f;  // Delay between blinks (in milliseconds)
float pulseWidth = 0.5f;  // Duration of the on/off time (0 to 1)
     
bool gateHigh = false;    // Gate state (high or low)
bool gate2High = false;    // Gate state (high or low)
bool gate3High = false;    // Gate state (high or low)
bool gate4High = false;    // Gate state (high or low)
bool gate5High = false;    // Gate state (high or low)

bool pulseActive = false; // To control one-shot pulse activation
float pulseTimer = 0.0f;  // Timer for the one-shot pulse duration
int masterCount = 1; 
int ch1 = 4; 
int ch2 = 8;
int ch3 = 12;
int ch4 = 16;
int masterDivide = 1; 

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
        configParam(COARSE_PARAM, 0.f, 1.f, 0.5f, "Coarse Speed");
        configParam(FINE_PARAM, -1.f, 1.f, 0.f, "Fine Speed");
        configParam(PULSEWIDTH_PARAM, 0.f, 1.f, 0.5f, "Pulse Width");
        configParam(START_STOP_PARAM, 0.f, 1.f, 1.f, "Start/Stop");
        configParam(UPBEAT_DOWNBEAT_PARAM, 0.f, 1.f, 0.f, "Upbeat/Downbeat");
        configParam(LEFT_RIGHT_PARAM, 0.f, 1.f, 0.f, "Left/Right");
        configParam(TOP_BOTTOM_PARAM, 0.f, 1.f, 0.f, "Top/Bottom");
        configParam(RANGE_PARAM, 0.f, 1.f, 1.f, "Range Switch");
        configInput(TIME_INPUT, "Time");
        configInput(WIDTH_INPUT, "Width");
        configOutput(DIVIDED_1_OUTPUT, "Divided 1");
        configOutput(DIVIDED_2_OUTPUT, "Divided 2");
        configOutput(DIVIDED_3_OUTPUT, "Divided 3");
        configOutput(DIVIDED_4_OUTPUT, "Divided 4");
        configOutput(START_OUTPUT, "Start Pulse");
        configOutput(MAIN_OUTPUT, "Main Output");
    }
    void process(const ProcessArgs& args) override {
        // Get values from the knobs and controls (no changes here)
        float fineTuneParam = params[FINE_PARAM].getValue();
        float pulseWidthParam = params[PULSEWIDTH_PARAM].getValue();
        bool rangeSwitch = params[RANGE_PARAM].getValue() > 0.5f;
        bool startStop = params[START_STOP_PARAM].getValue() > 0.5f;
        bool topBottom = params[TOP_BOTTOM_PARAM].getValue() > 0.5f;
        bool leftRight = params[LEFT_RIGHT_PARAM].getValue() > 0.5f;
        bool upbeatDownbeat = params[UPBEAT_DOWNBEAT_PARAM].getValue() > 0.5f;  // Track the upbeat/downbeat state
    
        // Previous state of start/stop (no changes here)
        static bool prevStartStop = false;
        static float oneShotTimer = 0.0f;
        static bool pulseSent = false;
    
        // Flag to track if masterCount should be reset
        static bool resetFlag = false;
    
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
    
                // Reset lights (LEDs)
                for (int i = 0; i < NUM_LIGHTS; ++i) {
                    lights[i].setBrightness(0.0f);
                }
    
                resetFlag = true;  // Mark that reset was done
            }
            outputs[MAIN_OUTPUT].setVoltage(0.0f);
        } else if (startStop) {
            resetFlag = false;
            if (startStop && !prevStartStop && !pulseSent) {
                outputs[START_OUTPUT].setVoltage(5.0f);
                lights[LED_1_LIGHT].setBrightness(1.0f);
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
                    outputs[MAIN_OUTPUT].setVoltage(5.0f);
                } else {
                    outputs[MAIN_OUTPUT].setVoltage(0.0f);
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
    
            // Divided Output 1 (Divide by 2) -> LED_2
            if (adjustedMasterCount % ch1 == 0) {
                if (clockTimer < onTime) {
                    outputs[DIVIDED_1_OUTPUT].setVoltage(5.0f);  // Gate high
                } else {
                    outputs[DIVIDED_1_OUTPUT].setVoltage(0.0f);  // Gate low
                }
            } else {
                outputs[DIVIDED_1_OUTPUT].setVoltage(0.0f);  // Gate low
            }
    
            // Divided Output 2 (Divide by 4) -> LED_3
            if (adjustedMasterCount % ch2 == 0) {
                if (clockTimer < onTime) {
                    outputs[DIVIDED_2_OUTPUT].setVoltage(5.0f);  // Gate high
                } else {
                    outputs[DIVIDED_2_OUTPUT].setVoltage(0.0f);  // Gate low
                }
            } else {
                outputs[DIVIDED_2_OUTPUT].setVoltage(0.0f);  // Gate low
            }
    
            // Divided Output 3 (Divide by 6) -> LED_5
            if (adjustedMasterCount % ch3 == 0) {
                if (clockTimer < onTime) {
                    outputs[DIVIDED_3_OUTPUT].setVoltage(5.0f);  // Gate high
                } else {
                    outputs[DIVIDED_3_OUTPUT].setVoltage(0.0f);  // Gate low
                }
            } else {
                outputs[DIVIDED_3_OUTPUT].setVoltage(0.0f);  // Gate low
            }
    
            // Divided Output 4 (Divide by 8) -> LED_6
            if (adjustedMasterCount % ch4 == 0) {
                if (clockTimer < onTime) {
                    outputs[DIVIDED_4_OUTPUT].setVoltage(5.0f);  // Gate high
                } else {
                    outputs[DIVIDED_4_OUTPUT].setVoltage(0.0f);  // Gate low
                }
            } else {
                outputs[DIVIDED_4_OUTPUT].setVoltage(0.0f);  // Gate low
            }
        }
    
        // Set light states based on outputs
        if (outputs[DIVIDED_1_OUTPUT].getVoltage() > 0.0f) {
            lights[LED_2_LIGHT].setBrightness(1.0f);
        } else {
            lights[LED_2_LIGHT].setBrightness(0.0f);
        }
    
        if (outputs[DIVIDED_2_OUTPUT].getVoltage() > 0.0f) {
            lights[LED_3_LIGHT].setBrightness(1.0f);
        } else {
            lights[LED_3_LIGHT].setBrightness(0.0f);
        }
    
        if (outputs[DIVIDED_3_OUTPUT].getVoltage() > 0.0f) {
            lights[LED_5_LIGHT].setBrightness(1.0f);
        } else {
            lights[LED_5_LIGHT].setBrightness(0.0f);
        }
    
        if (outputs[DIVIDED_4_OUTPUT].getVoltage() > 0.0f) {
            lights[LED_6_LIGHT].setBrightness(1.0f);
        } else {
            lights[LED_6_LIGHT].setBrightness(0.0f);
        }
    
        if (gateHigh) {
            lights[LED_4_LIGHT].setBrightness(outputs[MAIN_OUTPUT].getVoltage() / 5.0f);
        } else {
            lights[LED_4_LIGHT].setBrightness(0.0f);
        }
    
        // Save previous state of start/stop
        prevStartStop = startStop;
    }      
};

// The UI widget (module panel) code, integrated with the interface controls
struct ClockModuleWidget : ModuleWidget {
    ClockModuleWidget(ClockModule* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Count.svg")));

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
