#include "plugin.hpp"

struct BusMult : Module {
    enum ParamId {
        PARAMS_LEN
    };
    enum InputId {
        IN1_INPUT,
        IN2_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        OUT5_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LIGHTS_LEN
    };

    BusMult() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configInput(IN1_INPUT, "1");
        configInput(IN2_INPUT, "2");
        configOutput(OUT1_OUTPUT, "1");
        configOutput(OUT2_OUTPUT, "2");
        configOutput(OUT3_OUTPUT, "3");
        configOutput(OUT4_OUTPUT, "4");
        configOutput(OUT5_OUTPUT, "5");
    }

    void process(const ProcessArgs& args) override {
        // Get the signal from Input 1
        float input1 = inputs[IN1_INPUT].getVoltage();

        // Check if Input 2 is connected, if not normalize it to Input 1
        float input2 = inputs[IN2_INPUT].isConnected() ? inputs[IN2_INPUT].getVoltage() : input1;

        // Set the outputs based on the inputs
        outputs[OUT1_OUTPUT].setVoltage(input1);  // OUT1 = Input 1
        outputs[OUT2_OUTPUT].setVoltage(input1);  // OUT2 = Input 1
        outputs[OUT3_OUTPUT].setVoltage(input1);  // OUT3 = Input 1
        outputs[OUT4_OUTPUT].setVoltage(input2);  // OUT4 = Input 2 (normalized to Input 1 if not connected)
        outputs[OUT5_OUTPUT].setVoltage(input2);  // OUT5 = Input 2 (normalized to Input 1 if not connected)
    }
};

struct BusMultWidget : ModuleWidget {
    BusMultWidget(BusMult* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/BusMult.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.141, 17.9)), module, BusMult::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.125, 78.549)), module, BusMult::IN2_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.106, 33.007)), module, BusMult::OUT1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.157, 48.234)), module, BusMult::OUT2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.152, 63.351)), module, BusMult::OUT3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.132, 93.763)), module, BusMult::OUT4_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.134, 108.915)), module, BusMult::OUT5_OUTPUT));
    }
};

Model* modelBusMult = createModel<BusMult, BusMultWidget>("BusMult");
