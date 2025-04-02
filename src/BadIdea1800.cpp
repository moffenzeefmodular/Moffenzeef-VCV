#include "plugin.hpp"
#include <cmath> // for sine wave

#define TWO_PI (2.0 * M_PI)

struct BadIdea1800 : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		NUM_1_INPUT,
		NUM_2_INPUT,
		NUM_3_INPUT,
		NUM_4_INPUT,
		NUM_5_INPUT,
		NUM_6_INPUT,
		NUM_7_INPUT,
		NUM_8_INPUT,
		NUM_9_INPUT,
		ZERO_INPUT,
		ASTERISK_INPUT,
		POUND_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		BADIDEA_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LED_1_LIGHT,
		LED_2_LIGHT,
		LED_3_LIGHT,
		LED_4_LIGHT,
		LED_5_LIGHT,
		LED_6_LIGHT,
		LED_7_LIGHT,
		LED_8_LIGHT,
		LED_9_LIGHT,
		LED_10_LIGHT,
		LED_ASTERISK_LIGHT,
		LED_ZERO_LIGHT,
		LED_POUND_LIGHT,
		LIGHTS_LEN
	};

	BadIdea1800() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(NUM_1_INPUT, "1 Gate");
		configInput(NUM_2_INPUT, "2 Gate");
		configInput(NUM_3_INPUT, "3 Gate");
		configInput(NUM_4_INPUT, "4 Gate");
		configInput(NUM_5_INPUT, "5 Gate");
		configInput(NUM_6_INPUT, "6 Gate");
		configInput(NUM_7_INPUT, "7 Gate");
		configInput(NUM_8_INPUT, "8 Gate");
		configInput(NUM_9_INPUT, "9 Gate");
		configInput(ASTERISK_INPUT, "* Gate");
		configInput(ZERO_INPUT, "0 Gate");
		configInput(POUND_INPUT, "# Gate");
		configOutput(BADIDEA_OUTPUT, "Bad Idea 1800-Call-Yer-Mum Audio");
	}

	// Sample rate variable (to handle time-based calculations)
	float sampleRate;

	// DTMF Frequency pairs for each key
	const float frequencies[12][2] = {
		{697.0f, 1209.0f},  // 1
		{697.0f, 1336.0f},  // 2
		{697.0f, 1477.0f},  // 3
		{770.0f, 1209.0f},  // 4
		{770.0f, 1336.0f},  // 5
		{770.0f, 1477.0f},  // 6
		{852.0f, 1209.0f},  // 7
		{852.0f, 1336.0f},  // 8
		{852.0f, 1477.0f},  // 9
		{941.0f, 1336.0f},  // 0
		{941.0f, 1209.0f},  // *
		{941.0f, 1477.0f},  // #
	};

	// Time tracking variables for sine wave calculation
	float phase1[12] = {0.0f}; // Phase for 12 first oscillators
	float phase2[12] = {0.0f}; // Phase for 12 second oscillators

void process(const ProcessArgs& args) override {
    // Time step (delta time)
    float dt = 1.0f / args.sampleRate;
  
    // Sum of all the oscillator outputs
    float totalSum = 0.0f;
  
    // Loop through all 12 inputs (10 number keys, * and # keys)
    for (int i = 0; i < 12; ++i) { // 12 total inputs: 10 numbers, * and #
  
        // Determine the correct input index for each key:
        int inputIndex = -1;
        int ledIndex = -1;

        // Mapping the input index and corresponding LED index
        if (i < 10) {
            // Number keys (1-10)
            inputIndex = NUM_1_INPUT + i;
            ledIndex = LED_1_LIGHT + i; // Corresponding LED for number keys
        } else if (i == 10) {
            // Asterisk key
            inputIndex = ASTERISK_INPUT;
            ledIndex = LED_ASTERISK_LIGHT; // LED for asterisk key
        } else if (i == 11) {
            // Pound key
            inputIndex = POUND_INPUT;
            ledIndex = LED_POUND_LIGHT; // LED for pound key
        }
  
        // Get the corresponding frequencies for the current input
        float freq1 = frequencies[i][0];  // First frequency in the pair
        float freq2 = frequencies[i][1];  // Second frequency in the pair
  
        // Increment the phases of the oscillators
        phase1[i] += freq1 * TWO_PI * dt;
        phase2[i] += freq2 * TWO_PI * dt;
  
        // Wrap the phases to prevent overflow
        if (phase1[i] >= TWO_PI) phase1[i] -= TWO_PI;
        if (phase2[i] >= TWO_PI) phase2[i] -= TWO_PI;
  
        // Generate sine waves for the oscillators
        float sine1 = sinf(phase1[i]); // First oscillator (DTMF)
        float sine2 = sinf(phase2[i]); // Second oscillator (DTMF)
  
        // Sum the two sine waves together for this oscillator pair
        float oscillatorOutput = sine1 + sine2;
  
        // Read the input voltage for the current NUM channel
        float inputVoltage = inputs[inputIndex].getVoltage();
  
        // If the input is above 0.5V, scale the output accordingly
        if (inputVoltage > 0.5f) {
            oscillatorOutput *= inputVoltage * 2.5f;  // Scale by input voltage (max amplitude of 5V)
            lights[ledIndex].setBrightnessSmooth(1.0f, args.sampleTime);  // Turn on the corresponding LED
        } else {
            oscillatorOutput = 0.0f;  // Zero output when input is low
            lights[ledIndex].setBrightnessSmooth(0.0f);  // Turn off the corresponding LED
        }
  
        // Scale the oscillator output by 1/48 before adding it to the total sum
        oscillatorOutput *= (1.0f / 48.0f);
  
        // Add the scaled oscillator output to the total sum
        totalSum += oscillatorOutput;
    }
  
    // Clamp the total sum to a range of -5V to +5V
    float outputVoltage = clamp(totalSum, -5.0f, 5.0f);
  
    // Set the output voltage
    outputs[BADIDEA_OUTPUT].setVoltage(outputVoltage);
}

	};

struct BadIdea1800Widget : ModuleWidget {
	BadIdea1800Widget(BadIdea1800* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BadIdea1800.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.502, 22.904)), module, BadIdea1800::NUM_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.595, 22.904)), module, BadIdea1800::NUM_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.555, 22.94)), module, BadIdea1800::NUM_3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.541, 44.903)), module, BadIdea1800::NUM_4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.559, 44.845)), module, BadIdea1800::NUM_5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.576, 44.957)), module, BadIdea1800::NUM_6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.541, 66.899)), module, BadIdea1800::NUM_7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.575, 66.881)), module, BadIdea1800::NUM_8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.553, 66.92)), module, BadIdea1800::NUM_9_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.576, 88.861)), module, BadIdea1800::ZERO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.52, 88.824)), module, BadIdea1800::ASTERISK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(52.555, 88.86)), module, BadIdea1800::POUND_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(52.517, 110.859)), module, BadIdea1800::BADIDEA_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.569, 11.873)), module, BadIdea1800::LED_1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.52, 11.96)), module, BadIdea1800::LED_2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.553, 11.885)), module, BadIdea1800::LED_3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.54, 33.92)), module, BadIdea1800::LED_4_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.539, 33.884)), module, BadIdea1800::LED_5_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.536, 33.902)), module, BadIdea1800::LED_6_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.56, 55.921)), module, BadIdea1800::LED_7_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.558, 55.903)), module, BadIdea1800::LED_8_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.534, 55.936)), module, BadIdea1800::LED_9_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.541, 77.917)), module, BadIdea1800::LED_ASTERISK_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.575, 77.897)), module, BadIdea1800::LED_10_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.556, 77.898)), module, BadIdea1800::LED_POUND_LIGHT));
	}
};


Model* modelBadIdea1800 = createModel<BadIdea1800, BadIdea1800Widget>("BadIdea1800");