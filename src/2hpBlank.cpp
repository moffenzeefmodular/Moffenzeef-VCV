#include "plugin.hpp"


struct _2hpBlank : Module {
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

	_2hpBlank() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct _2hpBlankWidget : ModuleWidget {
	_2hpBlankWidget(_2hpBlank* module) {
		setModule(module);
	setPanel(createPanel(
		asset::plugin(pluginInstance, "res/2hpBlank.svg"),
		asset::plugin(pluginInstance, "res/2hpBlank-dark.svg")
		));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* model_2hpBlank = createModel<_2hpBlank, _2hpBlankWidget>("2hpBlank");