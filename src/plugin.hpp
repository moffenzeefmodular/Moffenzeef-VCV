#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelBadIdea9;
extern Model* modelBadIdea1800;
extern Model* modelBobcat;
extern Model* modelBusMult;
extern Model* modelCount;
extern Model* modelDeviant;
extern Model* modelDialUp;
extern Model* modelGMO;
extern Model* modelKriket;
extern Model* modelMito;
extern Model* modelMoffenmix;
extern Model* modelMongrel;
extern Model* modelMuskrat;
extern Model* modelSimplify;
extern Model* model_2hpBlank;
extern Model* model_14hpBlank;
extern Model* modelSwarm;
extern Model* modelINTENSIFIES;
extern Model* modelTheRunner;

struct CKSSHorizontal : app::SvgSwitch {
	CKSSHorizontal() {
		shadow->opacity = 0.0;
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThreeHorizontal_0.svg")));
		addFrame(Svg::load(asset::system("res/ComponentLibrary/CKSSThreeHorizontal_2.svg")));
	}
};

struct _2Pos : SvgSwitch {
    _2Pos() {
        momentary = false; // true for momentary behavior
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggle_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggle_2.svg")));
    }
};

struct _2PosHorizontal : SvgSwitch {
    _2PosHorizontal() {
        momentary = false; // true for momentary behavior
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggleH_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggleH_2.svg")));
    }
};

struct _3Pos : SvgSwitch {
    _3Pos() {
        momentary = false; // true for momentary behavior
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggle_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggle_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggle_2.svg")));
    }
};

struct _3PosHorizontal : SvgSwitch {
    _3PosHorizontal() {
        momentary = false; // true for momentary behavior
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggleH_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggleH_1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/SubMiniToggleH_2.svg")));
    }
};
