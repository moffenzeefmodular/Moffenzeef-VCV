#include "plugin.hpp"

struct TheRunner : Module {
	enum ParamId {
		GAIN_PARAM,
		VOLUME_PARAM,
		ANIMATE_PARAM,
		RANGE_PARAM,
		CHORUS_PARAM,
		HARMONICS_PARAM,
		CUTOFF_PARAM,
		RESONANCE_PARAM,
		PITCH_PARAM,
		NOTESHZ_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		GAINCVIN_INPUT,
		VOLUMECVIN_INPUT,
		ANIMATECVIN_INPUT,
		CHORUSCVIN_INPUT,
		HARMONICSCVIN_INPUT,
		CUTOFFCVIN_INPUT,
		RESONANCECVIN_INPUT,
		PITCHCVIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIOOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ANIMATELED_LIGHT,
		LIGHTS_LEN
	};

	TheRunner() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "Gain", "x", 20.f, 1.f);
		configParam(VOLUME_PARAM, 0.f, 1.f, 1.f, "Volume", "%", 0.f, 100.f);
		configParam(ANIMATE_PARAM, 0.f, 1.f, 0.f, "Animate", "%", 0.f, 100.f);
		configSwitch(RANGE_PARAM, 0.f, 1.f, 0.f, "Range", {"Low", "High"});
		configSwitch(CHORUS_PARAM, 0.f, 1.f, 0.f, "Chorus", {"Off", "On"});
		configParam(HARMONICS_PARAM, 0.f, 1.f, 0.f, "Harmonics", "%", 0.f, 100.f);
		configParam(CUTOFF_PARAM, 0.f, 1.f, 1.f, "Cutoff", "Hz", 250.f, 20.f);
		configParam(RESONANCE_PARAM, 0.f, 1.f, 0.f, "Resonance", "%", 0.f, 100.f);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.25f, "Pitch", "Hz", 32.f, 13.75f);
		configSwitch(NOTESHZ_PARAM, 0.f, 1.f, 0.f, "Quantize", {"Off", "On"});

		configInput(GAINCVIN_INPUT, "Gain CV");
		configInput(VOLUMECVIN_INPUT, "Volume CV");
		configInput(ANIMATECVIN_INPUT, "Animate CV");
		configInput(CHORUSCVIN_INPUT, "Chorus CV");
		configInput(HARMONICSCVIN_INPUT, "Harmonics CV");
		configInput(CUTOFFCVIN_INPUT, "Cutoff CV");
		configInput(RESONANCECVIN_INPUT, "Resonance CV");
		configInput(PITCHCVIN_INPUT, "1v/Oct");

		configOutput(AUDIOOUT_OUTPUT, "Audio");
	}

	float phases[5] = {};
	float lp = 0.f, bp = 0.f;
	enum { maxDelaySamplesHardLimit = 96000 };
	float delayBuffer[maxDelaySamplesHardLimit] = {};
	int delayWriteIndex = 0;
	float chorusPhase = 0.f;
	float hpfOut = 0.f;
	float hpfInPrev = 0.f;
	float drunkWalkPos = 0.5f;
	float drunkWalkSmoothed = 0.5f;

	void process(const ProcessArgs& args) override {
		const float minFreq = 13.75f;
		const float maxFreq = 440.f;

		float pitchKnob = params[PITCH_PARAM].getValue();
		float basePitchLog2 = rescale(pitchKnob, 0.f, 1.f, log2f(minFreq), log2f(maxFreq));
		float pitchCV = inputs[PITCHCVIN_INPUT].isConnected() ? clamp(inputs[PITCHCVIN_INPUT].getVoltage(), 0.f, 8.f) : 0.f;
		float pitchLog2 = clamp(basePitchLog2 + pitchCV, log2f(minFreq), log2f(maxFreq));
		if (params[NOTESHZ_PARAM].getValue() > 0.5f)
			pitchLog2 = std::round(pitchLog2 * 12.f) / 12.f;

		float freqs[5] = {
			powf(2.f, pitchLog2),
			powf(2.f, pitchLog2 - 1.f),
			powf(2.f, pitchLog2 + 1.f),
			powf(2.f, pitchLog2 + 19.f / 12.f),
			powf(2.f, pitchLog2 + 2.f)
		};

		float dt = args.sampleTime;

		float animate = clamp(params[ANIMATE_PARAM].getValue() + (inputs[ANIMATECVIN_INPUT].isConnected() ? clamp(inputs[ANIMATECVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 1.f);

		float stepSize = rescale(animate, 0.f, 1.f, 0.00001f, 0.00025f) * powf(10.f, params[RANGE_PARAM].getValue() * 1.2f);
		float depth = animate * 0.8f;

		drunkWalkPos = clamp(drunkWalkPos + (random::uniform() - 0.5f) * stepSize * 2.f, 0.1f, 0.9f);
		drunkWalkSmoothed = drunkWalkSmoothed * 0.97f + drunkWalkPos * 0.03f;

		float lfoOutput = (drunkWalkSmoothed - 0.5f) * 5.f * depth;
		lights[ANIMATELED_LIGHT].setBrightnessSmooth(clamp(std::fabs(lfoOutput), 0.f, 1.f), dt);

		float basePWM = clamp(0.5f + (drunkWalkSmoothed - 0.5f) * depth, 0.1f, 0.9f);

		float voices[5];
		for (int i = 0; i < 5; ++i) {
			phases[i] += freqs[i] * dt;
			if (phases[i] >= 1.f)
				phases[i] -= 1.f;
			float pwm = clamp(basePWM + (i - 2) * 0.05f, 0.1f, 0.9f);
			voices[i] = (phases[i] < pwm) ? 1.f : -1.f;
		}

		float harm = clamp(params[HARMONICS_PARAM].getValue() + (inputs[HARMONICSCVIN_INPUT].isConnected() ? clamp(inputs[HARMONICSCVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 1.f);

		float mix = 0.f;
		float gains[5];
		float harmAmt = clamp(harm, 0.f, 1.f);
		float sum = 0.f;
		for (int i = 0; i < 5; ++i) {
			gains[i] = (i == 0) ? 1.f : clamp((harmAmt - 0.25f * i) / 0.25f, 0.f, 1.f);
			sum += gains[i];
		}
		for (int i = 0; i < 5; ++i) {
			mix += voices[i] * gains[i];
		}
		if (sum > 0.f)
			mix = mix / sum * 4.f;

		float cutoffNorm = clamp(params[CUTOFF_PARAM].getValue() + (inputs[CUTOFFCVIN_INPUT].isConnected() ? clamp(inputs[CUTOFFCVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 1.f);
		float animateLFOForCutoff = (drunkWalkSmoothed - 0.5f) * 2.f;
		float modulatedCutoffNorm = clamp(cutoffNorm + animateLFOForCutoff * 0.3f, 0.f, 1.f);
		float cutoffFreq = rescale(modulatedCutoffNorm, 0.f, 1.f, 20.f, 5000.f);

		float resonance = clamp(params[RESONANCE_PARAM].getValue() + (inputs[RESONANCECVIN_INPUT].isConnected() ? clamp(inputs[RESONANCECVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 0.9f);

		float f = 2.f * sinf(float(M_PI) * cutoffFreq * dt);
		float q = 1.f - resonance;

		float hp = mix - lp - q * bp;
		bp += f * hp;
		lp += f * bp;

		float filtered = clamp(lp, -5.f, 5.f);

		bool chorusEnabled = params[CHORUS_PARAM].getValue() > 0.5f;
		if (inputs[CHORUSCVIN_INPUT].isConnected())
			chorusEnabled = inputs[CHORUSCVIN_INPUT].getVoltage() > 2.5f;

		float sampleRate = args.sampleRate;
		float delayBaseSamples = sampleRate * 0.0075f;
		const float chorusFrequency = 0.4f;
		const float chorusDepth = 0.6f;
		const float chorusDryWet = 0.5f;

		float lfo = sinf(2.f * M_PI * chorusPhase);
		chorusPhase += chorusFrequency / sampleRate;
		if (chorusPhase >= 1.f)
			chorusPhase -= 1.f;

		float delayModSamples = chorusDepth * sampleRate * 0.002f;
		float modulatedDelay = delayBaseSamples + lfo * delayModSamples;

		delayBuffer[delayWriteIndex] = filtered;
		float readIndex = delayWriteIndex - modulatedDelay;
		if (readIndex < 0)
			readIndex += maxDelaySamplesHardLimit;

		int readIndex1 = (int)readIndex;
		int readIndex2 = (readIndex1 + 1) % maxDelaySamplesHardLimit;
		float frac = readIndex - readIndex1;

		float delayedSample = delayBuffer[readIndex1] * (1.f - frac) + delayBuffer[readIndex2] * frac;
		delayWriteIndex = (delayWriteIndex + 1) % maxDelaySamplesHardLimit;

		float postChorus = chorusEnabled ? filtered * (1.f - chorusDryWet) + delayedSample * chorusDryWet : filtered;

		float gain = rescale(clamp(params[GAIN_PARAM].getValue() + (inputs[GAINCVIN_INPUT].isConnected() ? clamp(inputs[GAINCVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 1.f), 0.f, 1.f, 1.f, 20.f);

		float signal = postChorus * gain;

		signal = clamp(signal, -5.f, 5.f);

		float cutoffHz = 20.f;
		float RC = 1.f / (2.f * M_PI * cutoffHz);
		float alpha = RC / (RC + dt);
		hpfOut = alpha * (hpfOut + signal - hpfInPrev);
		hpfInPrev = signal;
		signal = hpfOut;

		float volumeNorm = clamp(params[VOLUME_PARAM].getValue() + (inputs[VOLUMECVIN_INPUT].isConnected() ? clamp(inputs[VOLUMECVIN_INPUT].getVoltage() / 10.f, -1.f, 1.f) : 0.f), 0.f, 1.f);

		signal *= volumeNorm;
		signal = clamp(signal, -5.f, 5.f);

		outputs[AUDIOOUT_OUTPUT].setVoltage(signal);

	}
};

struct TheRunnerWidget : ModuleWidget {
	TheRunnerWidget(TheRunner* module) {
		setModule(module);
	setPanel(createPanel(
		asset::plugin(pluginInstance, "res/TheRunner.svg"),
		asset::plugin(pluginInstance, "res/TheRunner-dark.svg")
		));
        
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Davies1900hLargeBlackKnob>(mm2px(Vec(28.141, 108.921)), module, TheRunner::PITCH_PARAM));

		addParam(createParamCentered<_2Pos>(mm2px(Vec(40.936, 49.41)), module, TheRunner::RANGE_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(66.526, 49.41)), module, TheRunner::CHORUS_PARAM));
		addParam(createParamCentered<_2Pos>(mm2px(Vec(53.754, 108.921)), module, TheRunner::NOTESHZ_PARAM));

		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(15.173, 19.704)), module, TheRunner::GAIN_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(40.79, 19.704)), module, TheRunner::VOLUME_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(15.346, 49.41)), module, TheRunner::ANIMATE_PARAM));
	
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(15.346, 79.116)), module, TheRunner::HARMONICS_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(40.936, 79.116)), module, TheRunner::CUTOFF_PARAM));
		addParam(createParamCentered<Davies1900hBlackKnob>(mm2px(Vec(66.526, 79.116)), module, TheRunner::RESONANCE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 23.525)), module, TheRunner::VOLUMECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 36.908)), module, TheRunner::GAINCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 50.298)), module, TheRunner::ANIMATECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 63.691)), module, TheRunner::CHORUSCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 77.098)), module, TheRunner::HARMONICSCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 90.488)), module, TheRunner::CUTOFFCVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.068, 103.882)), module, TheRunner::RESONANCECVIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.134, 117.265)), module, TheRunner::PITCHCVIN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(66.45, 19.704)), module, TheRunner::AUDIOOUT_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(28.085, 33.711)), module, TheRunner::ANIMATELED_LIGHT));
	}
};


Model* modelTheRunner = createModel<TheRunner, TheRunnerWidget>("TheRunner");