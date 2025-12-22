#include "plugin.hpp"

struct QuadDeviant : Module {
    enum ParamId {
        // Bus switches
        BUS1_PARAM,
        BUS2_PARAM,
        BUS3_PARAM,
        BUS4_PARAM,
        // Top knobs
        TOP1_PARAM,
        TOP2_PARAM,
        TOP3_PARAM,
        TOP4_PARAM,
        // Bottom knobs
        BOTTOM1_PARAM,
        BOTTOM2_PARAM,
        BOTTOM3_PARAM,
        BOTTOM4_PARAM,
        // Range switches
        RANGE1_PARAM,
        RANGE2_PARAM,
        RANGE3_PARAM,
        RANGE4_PARAM,
        PARAMS_LEN
    };

    enum InputId {
        // Clock inputs
        CLK1_INPUT,
        CLK2_INPUT,
        CLK3_INPUT,
        CLK4_INPUT,
        // Top CV inputs
        TOPCV1_INPUT,
        TOPCV2_INPUT,
        TOPCV3_INPUT,
        TOPCV4_INPUT,
        // Bottom CV inputs
        BOTTOMCV1_INPUT,
        BOTTOMCV2_INPUT,
        BOTTOMCV3_INPUT,
        BOTTOMCV4_INPUT,
        INPUTS_LEN
    };

    enum OutputId {
        // Math outputs
        SUMOUT_OUTPUT,
        INVSUMOUT_OUTPUT,
        MAXOUT_OUTPUT,
        MINOUT_OUTPUT,
        POSOUT_OUTPUT,
        NEGOUT_OUTPUT,
        FULLOUT_OUTPUT,
        INVFULLOUT_OUTPUT,
        // Channel outputs
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        OUT3_OUTPUT,
        OUT4_OUTPUT,
        // Additional outputs
        AVGOUT_OUTPUT,
        SLEWOUT_OUTPUT,
        OUTPUTS_LEN
    };

    enum LightId {
        // Channel LEDs (red/green pairs)
        LED1RED_LIGHT,
        LED1GREEN_LIGHT,
        LED2RED_LIGHT,
        LED2GREEN_LIGHT,
        LED3RED_LIGHT,
        LED3GREEN_LIGHT,
        LED4RED_LIGHT,
        LED4GREEN_LIGHT,
        // Math LEDs (red/green pairs)
        SUMLEDRED_LIGHT,
        SUMLEDGREEN_LIGHT,
        INVLEDRED_LIGHT,
        INVLEDGREEN_LIGHT,
        MAXLEDRED_LIGHT,
        MAXLEDGREEN_LIGHT,
        MINLEDRED_LIGHT,
        MINLEDGREEN_LIGHT,
        POSLEDRED_LIGHT,
        POSLEDGREEN_LIGHT,
        NEGLEDRED_LIGHT,
        NEGLEDGREEN_LIGHT,
        FULLLEDRED_LIGHT,
        FULLLEDGREEN_LIGHT,
        INVFULLLEDRED_LIGHT,
        INVFULLLEDGREEN_LIGHT,
        AVGLEDRED_LIGHT,
        AVGLEDGREEN_LIGHT,
        SLEWLEDRED_LIGHT,
        SLEWLEDGREEN_LIGHT,
        // Bus LEDs
        BUS1LED_LIGHT,
        BUS2LED_LIGHT,
        BUS3LED_LIGHT,
        BUS4LED_LIGHT,
        LIGHTS_LEN
    };
QuadDeviant() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    // --- Bus switches ---
    configSwitch(BUS1_PARAM, 0.f, 1.f, 0.f, "Bus 1");
    configSwitch(BUS2_PARAM, 0.f, 1.f, 0.f, "Bus 2");
    configSwitch(BUS3_PARAM, 0.f, 1.f, 0.f, "Bus 3");
    configSwitch(BUS4_PARAM, 0.f, 1.f, 0.f, "Bus 4");

    // --- Top knobs ---
    configParam(TOP1_PARAM, 0.f, 1.f, 1.f, "Top 1", "v");
    configParam(TOP2_PARAM, 0.f, 1.f, 1.f, "Top 2", "v");
    configParam(TOP3_PARAM, 0.f, 1.f, 1.f, "Top 3", "v");
    configParam(TOP4_PARAM, 0.f, 1.f, 1.f, "Top 4", "v");

    // --- Bottom knobs ---
    configParam(BOTTOM1_PARAM, 0.f, 1.f, 0.f, "Bottom 1", "v");
    configParam(BOTTOM2_PARAM, 0.f, 1.f, 0.f, "Bottom 2", "v");
    configParam(BOTTOM3_PARAM, 0.f, 1.f, 0.f, "Bottom 3", "v");
    configParam(BOTTOM4_PARAM, 0.f, 1.f, 0.f, "Bottom 4", "v");

    // --- Range switches ---
    configSwitch(RANGE1_PARAM, 0.f, 2.f, 0.f, "Range 1", {"5vpp", "10vpp", "20vpp"});
    configSwitch(RANGE2_PARAM, 0.f, 2.f, 0.f, "Range 2", {"5vpp", "10vpp", "20vpp"});
    configSwitch(RANGE3_PARAM, 0.f, 2.f, 0.f, "Range 3", {"5vpp", "10vpp", "20vpp"});
    configSwitch(RANGE4_PARAM, 0.f, 2.f, 0.f, "Range 4", {"5vpp", "10vpp", "20vpp"});

    // --- Clock inputs ---
    configInput(CLK1_INPUT, "Clock 1");
    configInput(CLK2_INPUT, "Clock 2");
    configInput(CLK3_INPUT, "Clock 3");
    configInput(CLK4_INPUT, "Clock 4");

    // --- Top CV inputs ---
    configInput(TOPCV1_INPUT, "Top CV 1");
    configInput(TOPCV2_INPUT, "Top CV 2");
    configInput(TOPCV3_INPUT, "Top CV 3");
    configInput(TOPCV4_INPUT, "Top CV 4");

    // --- Bottom CV inputs ---
    configInput(BOTTOMCV1_INPUT, "Bottom CV 1");
    configInput(BOTTOMCV2_INPUT, "Bottom CV 2");
    configInput(BOTTOMCV3_INPUT, "Bottom CV 3");
    configInput(BOTTOMCV4_INPUT, "Bottom CV 4");

    // --- CV Bus outputs ---
    configOutput(SUMOUT_OUTPUT, "Sum");
    configOutput(INVSUMOUT_OUTPUT, "Inverse Sum");
    configOutput(MAXOUT_OUTPUT, "Max");
    configOutput(MINOUT_OUTPUT, "Min");
    configOutput(POSOUT_OUTPUT, "Positive");
    configOutput(NEGOUT_OUTPUT, "Negative");
    configOutput(FULLOUT_OUTPUT, "Full Wave Rectified");
    configOutput(INVFULLOUT_OUTPUT, "Inverse Full Wave Rectified");
    configOutput(AVGOUT_OUTPUT, "Average");
    configOutput(SLEWOUT_OUTPUT, "Slew");

    // --- Channel outputs ---
    configOutput(OUT1_OUTPUT, "Ch. 1");
    configOutput(OUT2_OUTPUT, "Ch. 2");
    configOutput(OUT3_OUTPUT, "Ch. 3");
    configOutput(OUT4_OUTPUT, "Ch. 4");
}



dsp::SchmittTrigger busTrigger[4];
bool busState[4] = {true, true, true, true};
float lastBang[4] = {0.f, 0.f, 0.f, 0.f};
float randomVoltage[4] = {0.01f, 0.01f, 0.01f, 0.01f};

float slewVoltage = 0.f;           // Last slew output
const float slewRate = 0.0001f;       // Max volts per sample (adjust for speed)


// Rescale helper: 0-1 param -> -5V to 5V
float paramToVoltage(float p) {
    return -5.f + p * 10.f;
}

void onReset() override {
    // --- Turn off all LEDs except bus LEDs ---
    for (int i = 0; i < LIGHTS_LEN; i++) {
        // Bus LEDs are at the end of the enum
        if (i < BUS1LED_LIGHT || i > BUS4LED_LIGHT) {
            lights[i].setBrightness(0.f);
        }
    }

    // --- Initialize bus states ---
    for (int i = 0; i < 4; i++)
        busState[i] = true;

    // --- Reset lastBang trackers ---
    for (int i = 0; i < 4; i++)
        lastBang[i] = 0.f;

    // --- Reset random voltages ---
    for (int i = 0; i < 4; i++)
        randomVoltage[i] = 0.0f;

    // --- Set all CV outputs to 0 ---
    for (int i = 0; i < OUTPUTS_LEN; i++)
        outputs[i].setVoltage(0.f);
}



void onRandomize() override {
    for (int i = 0; i < 4; i++) busState[i] = (rand() % 2 == 0);
}

// Range scaling helper
float rangeScale(float voltage, float rangeParam) {
    float ranges[3] = {2.5f, 5.f, 10.f};  // ±2.5V, ±5V, ±10V
    return clamp(voltage * (ranges[(int)rangeParam] / 5.f), -ranges[(int)rangeParam], ranges[(int)rangeParam]);
}

void process(const ProcessArgs &args) override {
    // --- Bus buttons ---
    for (int i = 0; i < 4; i++) {
        if (busTrigger[i].process(params[BUS1_PARAM + i].getValue()))
            busState[i] = !busState[i];

        // Only button LEDs light up immediately
        lights[BUS1LED_LIGHT + i].setBrightness(busState[i] ? 1.f : 0.f);
    }

    // --- Clock normalization cascade ---
    float normalizedClocks[4];
    bool hasClock = false;
    for (int i = 0; i < 4; i++) {
        if (inputs[CLK1_INPUT + i].isConnected()) {
            normalizedClocks[i] = inputs[CLK1_INPUT + i].getVoltage();
            hasClock = true;
        } else if (hasClock) {
            normalizedClocks[i] = normalizedClocks[i - 1];
        } else {
            normalizedClocks[i] = 0.f;
        }
    }

    // --- Channel processing ---
    for (int i = 0; i < 4; i++) {
        bool bangTriggered = (normalizedClocks[i] > 0.5f && lastBang[i] <= 0.5f);
        lastBang[i] = normalizedClocks[i];

        if (bangTriggered) {
            // Compute channel voltage
            float topValue    = paramToVoltage(params[TOP1_PARAM + i].getValue()) + inputs[TOPCV1_INPUT + i].getVoltage();
            float bottomValue = paramToVoltage(params[BOTTOM1_PARAM + i].getValue()) + inputs[BOTTOMCV1_INPUT + i].getVoltage();
            if (bottomValue >= topValue) bottomValue = topValue;

            float rawVoltage = clamp(bottomValue + (topValue - bottomValue) * random::uniform(), -5.f, 5.f);
            randomVoltage[i] = rangeScale(rawVoltage, params[RANGE1_PARAM + i].getValue());

            // --- Channel output always active ---
            outputs[OUT1_OUTPUT + i].setVoltage(randomVoltage[i]);

            // --- Rectified channel LEDs ---
            int ledBase = LED1RED_LIGHT + i * 2;
            float maxRange = rangeScale(10.f, params[RANGE1_PARAM + i].getValue());
            lights[ledBase].setBrightness(std::max(-randomVoltage[i] / maxRange, 0.f));       // red = negative
            lights[ledBase + 1].setBrightness(std::max(randomVoltage[i] / maxRange, 0.f));    // green = positive
        }
    }

	/////// CV BUS /////////

float sumVoltage = 0.f;
for (int i = 0; i < 4; i++)
    sumVoltage += busState[i] ? randomVoltage[i] : 0.f;

float sumClamped = clamp(sumVoltage, -10.f, 10.f);
outputs[SUMOUT_OUTPUT].setVoltage(sumClamped);
lights[SUMLEDRED_LIGHT].setBrightness(std::max(-sumClamped / 10.f, 0.f));
lights[SUMLEDGREEN_LIGHT].setBrightness(std::max(sumClamped / 10.f, 0.f));

float invSum = clamp(-sumClamped, -10.f, 10.f);
outputs[INVSUMOUT_OUTPUT].setVoltage(invSum);
lights[INVLEDRED_LIGHT].setBrightness(std::max(-invSum / 10.f, 0.f));
lights[INVLEDGREEN_LIGHT].setBrightness(std::max(invSum / 10.f, 0.f));

bool hasActive = false;
float maxV = -10.f;
float minV = 10.f;

for (int i = 0; i < 4; i++) {
    if (!busState[i]) continue;
    hasActive = true;
    maxV = std::max(maxV, randomVoltage[i]);
    minV = std::min(minV, randomVoltage[i]);
}

if (!hasActive) maxV = minV = 0.f;

maxV = clamp(maxV, -10.f, 10.f);
minV = clamp(minV, -10.f, 10.f);

outputs[MAXOUT_OUTPUT].setVoltage(maxV);
outputs[MINOUT_OUTPUT].setVoltage(minV);

lights[MAXLEDRED_LIGHT].setBrightness(std::max(-maxV / 10.f, 0.f));
lights[MAXLEDGREEN_LIGHT].setBrightness(std::max(maxV / 10.f, 0.f));
lights[MINLEDRED_LIGHT].setBrightness(std::max(-minV / 10.f, 0.f));
lights[MINLEDGREEN_LIGHT].setBrightness(std::max(minV / 10.f, 0.f));

float pos = clamp(std::max(sumClamped, 0.f), -10.f, 10.f);
float neg = clamp(std::min(sumClamped, 0.f), -10.f, 10.f);

outputs[POSOUT_OUTPUT].setVoltage(pos);
outputs[NEGOUT_OUTPUT].setVoltage(neg);

lights[POSLEDGREEN_LIGHT].setBrightness(pos / 10.f);
lights[NEGLEDRED_LIGHT].setBrightness(-neg / 10.f);

float full = clamp(std::abs(sumClamped), -10.f, 10.f);
outputs[FULLOUT_OUTPUT].setVoltage(full);
outputs[INVFULLOUT_OUTPUT].setVoltage(clamp(-full, -10.f, 10.f));

lights[FULLLEDGREEN_LIGHT].setBrightness(full / 10.f);
lights[INVFULLLEDRED_LIGHT].setBrightness(full / 10.f);

float avg = hasActive ? (sumClamped / 4.f) : 0.f;
avg = clamp(avg, -10.f, 10.f);
outputs[AVGOUT_OUTPUT].setVoltage(avg);

lights[AVGLEDRED_LIGHT].setBrightness(std::max(-avg / 10.f, 0.f));
lights[AVGLEDGREEN_LIGHT].setBrightness(std::max(avg / 10.f, 0.f));

float delta = sumClamped - slewVoltage;
delta = clamp(delta, -slewRate, slewRate);
slewVoltage += delta;

slewVoltage = clamp(slewVoltage, -10.f, 10.f);
outputs[SLEWOUT_OUTPUT].setVoltage(slewVoltage);

lights[SLEWLEDRED_LIGHT].setBrightnessSmooth(std::max(-slewVoltage / 10.f, 0.f), args.sampleTime);
lights[SLEWLEDGREEN_LIGHT].setBrightnessSmooth(std::max(slewVoltage / 10.f, 0.f), args.sampleTime);
}
};


struct QuadDeviantWidget : ModuleWidget {

    // Store pointers to ParamQuantities for dynamic labels
    ParamQuantity* bus1Q = nullptr;
    ParamQuantity* bus2Q = nullptr;
    ParamQuantity* bus3Q = nullptr;
    ParamQuantity* bus4Q = nullptr;

    // 🔴 ADDED: knob + range ParamQuantities
    ParamQuantity* topQ[4]    = {};
    ParamQuantity* bottomQ[4] = {};
    ParamQuantity* rangeQ[4]  = {};

	QuadDeviantWidget(QuadDeviant* module) {
		setModule(module);
	setPanel(createPanel(
		asset::plugin(pluginInstance, "res/panels/QuadDeviant.svg")
		));


		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(108.324, 18.277)), module, QuadDeviant::BUS1_PARAM, QuadDeviant::BUS1LED_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(121.633, 18.308)), module, QuadDeviant::BUS2_PARAM, QuadDeviant::BUS2LED_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(108.324, 30.54)), module, QuadDeviant::BUS3_PARAM, QuadDeviant::BUS3LED_LIGHT));
		addParam(createLightParamCentered<VCVLightBezel<WhiteLight>>(mm2px(Vec(121.633, 30.531)), module, QuadDeviant::BUS4_PARAM, QuadDeviant::BUS4LED_LIGHT));
    
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(14.664, 25.23)), module, QuadDeviant::TOP1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(39.664, 25.23)), module, QuadDeviant::TOP2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(64.664, 25.23)), module, QuadDeviant::TOP3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(89.664, 25.23)), module, QuadDeviant::TOP4_PARAM));
	
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(14.664, 45.608)), module, QuadDeviant::BOTTOM1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(39.664, 45.608)), module, QuadDeviant::BOTTOM2_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(64.664, 45.608)), module, QuadDeviant::BOTTOM3_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(89.664, 45.608)), module, QuadDeviant::BOTTOM4_PARAM));

		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(14.664, 61.826)), module, QuadDeviant::RANGE1_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(39.664, 61.826)), module, QuadDeviant::RANGE2_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(64.664, 61.826)), module, QuadDeviant::RANGE3_PARAM));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(89.664, 61.826)), module, QuadDeviant::RANGE4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.664, 76.014)), module, QuadDeviant::CLK1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.664, 75.929)), module, QuadDeviant::CLK2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(64.664, 75.929)), module, QuadDeviant::CLK3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(89.664, 75.929)), module, QuadDeviant::CLK4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.664, 88.876)), module, QuadDeviant::TOPCV1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.664, 88.876)), module, QuadDeviant::TOPCV2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(64.664, 88.876)), module, QuadDeviant::TOPCV3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(89.664, 88.876)), module, QuadDeviant::TOPCV4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.664, 101.449)), module, QuadDeviant::BOTTOMCV1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(39.664, 101.449)), module, QuadDeviant::BOTTOMCV2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(64.664, 101.449)), module, QuadDeviant::BOTTOMCV3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(89.664, 101.449)), module, QuadDeviant::BOTTOMCV4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(108.324, 44.897)), module, QuadDeviant::SUMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.633, 44.9)), module, QuadDeviant::INVSUMOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(108.324, 62.102)), module, QuadDeviant::MAXOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.633, 62.047)), module, QuadDeviant::MINOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(108.324, 79.23)), module, QuadDeviant::POSOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.633, 79.194)), module, QuadDeviant::NEGOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(108.324, 96.435)), module, QuadDeviant::FULLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.633, 96.417)), module, QuadDeviant::INVFULLOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.664, 113.602)), module, QuadDeviant::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.664, 113.602)), module, QuadDeviant::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(64.664, 113.602)), module, QuadDeviant::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(89.664, 113.602)), module, QuadDeviant::OUT4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(108.324, 113.602)), module, QuadDeviant::AVGOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.633, 113.602)), module, QuadDeviant::SLEWOUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(11.801, 13.745)), module, QuadDeviant::LED1RED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(17.189, 13.745)), module, QuadDeviant::LED1GREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(36.801, 13.745)), module, QuadDeviant::LED2RED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(42.189, 13.745)), module, QuadDeviant::LED2GREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(61.801, 13.745)), module, QuadDeviant::LED3RED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(67.190, 13.745)), module, QuadDeviant::LED3GREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(86.801, 13.745)), module, QuadDeviant::LED4RED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(92.189, 13.745)), module, QuadDeviant::LED4GREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(108.324, 38.165)), module, QuadDeviant::SUMLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.324, 38.165)), module, QuadDeviant::SUMLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.633, 38.162)), module, QuadDeviant::INVLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.633, 38.162)), module, QuadDeviant::INVLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(108.324, 54.841)), module, QuadDeviant::MAXLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.324, 54.841)), module, QuadDeviant::MAXLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.633, 54.78)), module, QuadDeviant::MINLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.633, 54.78)), module, QuadDeviant::MINLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(108.324, 71.969)), module, QuadDeviant::POSLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.324, 71.969)), module, QuadDeviant::POSLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.633, 71.927)), module, QuadDeviant::NEGLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.633, 71.927)), module, QuadDeviant::NEGLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(108.324, 89.174)), module, QuadDeviant::FULLLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.324, 89.174)), module, QuadDeviant::FULLLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.633, 89.15)), module, QuadDeviant::INVFULLLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.633, 89.15)), module, QuadDeviant::INVFULLLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(108.324, 106.341)), module, QuadDeviant::AVGLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(108.324, 106.341)), module, QuadDeviant::AVGLEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.633, 106.335)), module, QuadDeviant::SLEWLEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.633, 106.335)), module, QuadDeviant::SLEWLEDGREEN_LIGHT));

  if (module) {
            bus1Q = module->paramQuantities[QuadDeviant::BUS1_PARAM];
            bus2Q = module->paramQuantities[QuadDeviant::BUS2_PARAM];
            bus3Q = module->paramQuantities[QuadDeviant::BUS3_PARAM];
            bus4Q = module->paramQuantities[QuadDeviant::BUS4_PARAM];

               // 🔴 ADDED: cache knob + range quantities
            for (int i = 0; i < 4; i++) {
                topQ[i]    = module->paramQuantities[QuadDeviant::TOP1_PARAM + i];
                bottomQ[i] = module->paramQuantities[QuadDeviant::BOTTOM1_PARAM + i];
                rangeQ[i]  = module->paramQuantities[QuadDeviant::RANGE1_PARAM + i];
            }
        }
    }
// 🔴 ADDED: display-only scaling helper
    static void applyRangeLabel(ParamQuantity* pq, int range) {
        if (!pq) return;

        switch (range) {
            case 0: pq->displayMultiplier = 5.f;  pq->displayOffset = -2.5f; break;
            case 1: pq->displayMultiplier = 10.f; pq->displayOffset = -5.f;  break;
            case 2: pq->displayMultiplier = 20.f; pq->displayOffset = -10.f; break;
        }
        pq->unit = " V";
    }

   void step() override {
        ModuleWidget::step();

        if (!module) return;
        auto* m = dynamic_cast<QuadDeviant*>(module);

        ParamQuantity* qs[4] = {bus1Q, bus2Q, bus3Q, bus4Q};

        // 🔥 Loop through channels for dynamic labeling
        for (int i = 0; i < 4; i++) {
            if (qs[i]) {
                qs[i]->name = m->busState[i]
                    ? ("Ch. " + std::to_string(i + 1) + " (ON)")
                    : ("Ch. " + std::to_string(i + 1) + " (OFF)");
            }
        }

              // 🔴 ADDED: update knob numeric display from range switch
        for (int i = 0; i < 4; i++) {
            int range = (int)m->params[QuadDeviant::RANGE1_PARAM + i].getValue();
            applyRangeLabel(topQ[i], range);
            applyRangeLabel(bottomQ[i], range);
        }
    }
};


Model* modelQuadDeviant = createModel<QuadDeviant, QuadDeviantWidget>("QuadDeviant");