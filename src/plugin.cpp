#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelBadIdea9);
	p->addModel(modelBadIdea1800);
	p->addModel(modelBobcat);
	p->addModel(modelBusMult);
	p->addModel(modelCount);
	p->addModel(modelDeviant);
	p->addModel(modelDialUp);
	p->addModel(modelGMO);
	p->addModel(modelKriket);
	p->addModel(modelMito);
	p->addModel(modelMoffenmix);
	p->addModel(modelMongrel);
	p->addModel(modelMuskrat);
	p->addModel(modelSimplify);
	p->addModel(modelTwoHPBlank);
	p->addModel(modelFourteenHPBlank);
	p->addModel(modelSwarm);
	p->addModel(modelINTENSIFIES);
	p->addModel(modelTheRunner);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
