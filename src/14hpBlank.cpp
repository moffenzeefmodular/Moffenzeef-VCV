#include "plugin.hpp"


struct _14hpBlank : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	_14hpBlank() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct _14hpBlankWidget : ModuleWidget {
	_14hpBlankWidget(_14hpBlank* module) {
		setModule(module);
setPanel(createPanel(
		asset::plugin(pluginInstance, "res/14hpBlank.svg"),
		asset::plugin(pluginInstance, "res/14hpBlank-dark.svg")
		));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* model_14hpBlank = createModel<_14hpBlank, _14hpBlankWidget>("14hpBlank");