#include "plugin.hpp"


struct TwoHPBlank : Module {
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

	TwoHPBlank() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct TwoHPBlankWidget : ModuleWidget {
	TwoHPBlankWidget(TwoHPBlank* module) {
		setModule(module);
	setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/TwoHPBlank.svg")));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};

Model* modelTwoHPBlank = createModel<TwoHPBlank, TwoHPBlankWidget>("TwoHPBlank");