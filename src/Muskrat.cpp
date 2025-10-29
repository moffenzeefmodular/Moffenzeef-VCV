#include <vector>
#include <random> 
#include "rack.hpp"
#include "plugin.hpp"
#include "dsp/digital.hpp"  // For utilities like `mix` and other DSP-related functions
#include "../res/wavetables/MuskratWavetables.hpp"

using namespace rack;

struct Muskrat : Module {
    enum ParamId {
        TAIL_PARAM,
        RANGE_PARAM,
        SCRATCH_PARAM,
        DIG_PARAM,
        CHEW_PARAM,
        RATSWITCH_PARAM,
        SELECT_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        BANG_INPUT,
        SCRATCH_CV_INPUT,
        DIG_CV_INPUT,
        CHEW_CV_INPUT,
        MUSKRAT_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        AUDIO_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LED_LIGHT,
        LIGHTS_LEN
    };

 // Constructor
 Muskrat() {
  config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	configParam(TAIL_PARAM, 0.f, 1.f, 0.5f, "Decay Time", " %", 0.f, 100.f);
	configSwitch(RANGE_PARAM, 0.f, 2.f, 1.f, "Decay Time Range", {"Fast", "Medium", "Slow"});
	configParam(SCRATCH_PARAM, 0.f, 1.f, 0.5f, "Scratch", " %", 0.f, 100.f);
	configParam(DIG_PARAM, 0.f, 1.f, 0.5f, "Dig", " %", 0.f, 100.f);
	configParam(CHEW_PARAM, 0.f, 1.f, 0.5f, "Chew", " %", 0.f, 100.f);

	configInput(BANG_INPUT, "Bang! Gate");
	configInput(SCRATCH_CV_INPUT, "Scratch CV");
	configInput(DIG_CV_INPUT, "Dig CV");
	configInput(CHEW_CV_INPUT, "Chew CV");
	configSwitch(RATSWITCH_PARAM, 0.f, 1.f, 0.f, "Ratswitch", {"Off", "On"});
	configInput(MUSKRAT_INPUT, "Ratswitch Gate");
	configOutput(AUDIO_OUTPUT, "Muskrat Audio");

  configSwitch(SELECT_PARAM, 1.f, 4.f, 1.f, "Algorithm", {"Muskrat", "FM", "PD", "Granular"}); 

  initializeWavetables();
}
 
  //Granular 
  float wavetablesGranular[WAVETABLE_SIZE_GRANULAR];
  float phaseGranular = 0.0f; // phaseGranular accumulator for oscillator
  //OG
  float wavetables[WAVETABLE_SIZE];
  float phase = 0.0f; // Phase accumulator for oscillator
  
  // FM
  float phase1 = 0.0f;
  float phase2 = 0.0f;
  float freq2 = 0.0f;

  //PD
   float phase3 = 0.0f;
   float freq3 = 0.0f;

  // Global 
   float decayTime = 0.0f; 
   bool lastBangState = false; 
   bool pulseTriggered = false; 
   float envelopeValue = 0.0f; 
   float pulseTime = 0.0f; 

   float triangleWave(float phaseFM) {
    phaseFM = phaseFM - floor(phaseFM);
    return 2.0f * fabs(2.0f * phaseFM - 1.0f) - 1.0f;
    }
  
   float triangleWave2(float phaseFM) {
   phaseFM = phaseFM - floor(phaseFM);
   return 2.0f * fabs(2.0f * phaseFM - 1.0f) - 1.0f;
   }

   float sawtoothWave(float phasePD) {
   return 2.0f * (phasePD - floor(phasePD + 0.5f)); 
   }
   
   float amplitude = 40.0f; 
   std::random_device rd;
   std::mt19937 gen{rd()};
   std::uniform_real_distribution<float> dis{-amplitude, amplitude}; // Range -5V to +5V

// Add to the class member variables
private:
  // Normalized wavetables (pre-computed)
  float normalizedWavetables[24][WAVETABLE_SIZE];
  float normalizedWavetablesGranular[16][WAVETABLE_SIZE_GRANULAR];
  
  // State tracking for wavetable selection
  int lastWavetableIndex = -1;
  int lastGranularIndex = -1;
  int lastWavetableLength = -1;

public:
  // Method to initialize all wavetables at startup
  void initializeWavetables() {
    const unsigned char* wavetablesData[] = {
        one_wav, two_wav, three_wav, four_wav, five_wav,
        six_wav, seven_wav, eight_wav, nine_wav, ten_wav,
        eleven_wav, twelve_wav, thirteen_wav, fourteen_wav, fifteen_wav,
        sixteen_wav, seventeen_wav, eighteen_wav, nineteen_wav, twenty_wav,
        twentyOne_wav, twentyTwo_wav, twentyThree_wav, twentyFour_wav
    };
    
    const unsigned char* wavetablesDataGranular[] = {
        msgOne_wav, msgTwo_wav, msgThree_wav, msgFour_wav, msgFive_wav,
        msgSix_wav, msgSeven_wav, msgEight_wav, msgNine_wav, msgTen_wav,
        msgEleven_wav, msgTwelve_wav, msgThirteen_wav, msgFourteen_wav,
        msgFifteen_wav, msgSixteen_wav
    };
    
    // Precompute all normalized wavetables
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < WAVETABLE_SIZE; j++) {
            normalizedWavetables[i][j] = (float)(wavetablesData[i][j] - 128) / 128.0f;
        }
    }
    
    // Precompute all normalized granular wavetables
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < WAVETABLE_SIZE_GRANULAR; j++) {
            normalizedWavetablesGranular[i][j] = (float)(wavetablesDataGranular[i][j] - 128) / 128.0f;
        }
    }
  }
  
  void process(const ProcessArgs &args) override {
    // Parameters and CV processing - same as before
    float cvInput2 = inputs[SCRATCH_CV_INPUT].getVoltage();
    float normalizedCV2 = (cvInput2 + 5.0f) / 10.0f;
    float knob2Param = params[SCRATCH_PARAM].getValue() + 0.05;
    float knob2Value = knob2Param + (normalizedCV2 - 0.5f);
    float controlValue2 = std::clamp(knob2Value, 0.0f, 1.0f);

    float cvInput3 = inputs[DIG_CV_INPUT].getVoltage();
    float normalizedCV3 = (cvInput3 + 5.0f) / 10.0f;
    float knob3Param = params[DIG_PARAM].getValue() + 0.05;
    float knob3Value = knob3Param + (normalizedCV3 - 0.5f);
    float controlValue3 = std::clamp(knob3Value, 0.0f, 1.0f);

    int WAVETABLE_LENGTH = (controlValue3 * 308.0f) + 10.0f;
    // Only update granular tables if length has changed
    if (WAVETABLE_LENGTH != lastWavetableLength) {
        lastWavetableLength = WAVETABLE_LENGTH;
        lastGranularIndex = -1; // Force update of granular wavetable
    }
    
    float foldThreshold = 0.25f + (controlValue3 * 0.25);
    float distortion = foldThreshold * 500;
    
    float cvInput = inputs[CHEW_CV_INPUT].getVoltage();
    float normalizedCV = (cvInput + 5.0f) / 10.0f;
    float knob1Param = params[CHEW_PARAM].getValue() + 0.05;
    float knob1Value = knob1Param + (normalizedCV - 0.5f);
    float controlValue = std::clamp(knob1Value, 0.0f, 1.0f);
    float frequency = controlValue * args.sampleRate;
    
    // Wavetable selection with change detection
    int index = static_cast<int>(controlValue2 * 23);
    index = std::clamp(index, 0, 23); // Ensure index is in valid range
    
    // Only copy wavetable if the index has changed
    if (index != lastWavetableIndex) {
        lastWavetableIndex = index;
        // No need to copy/normalize - just point to the pre-normalized table
    }
    
    int indexGranular = static_cast<int>(controlValue2 * 15);
    indexGranular = std::clamp(indexGranular, 0, 15); // Ensure granular index is in valid range
    
    // Only update granular wavetable if needed
    if (indexGranular != lastGranularIndex) {
        lastGranularIndex = indexGranular;
        // No need to copy/normalize here either
    }
    
    // Rest of code uses direct access to normalized wavetables
    float phaseGranularIncrement = frequency / args.sampleRate;

    // OG
    float phaseIncrement = frequency / args.sampleRate;
    phase += phaseIncrement;
    phase = fmod(phase, WAVETABLE_SIZE);
    int phaseIndex = static_cast<int>(phase);
    float outputSample = normalizedWavetables[index][phaseIndex]; // Use pre-normalized table
    float foldedSample = outputSample;
    if (foldedSample > foldThreshold) {
        foldedSample = foldThreshold - (foldedSample - foldThreshold);
    } else if (foldedSample < -foldThreshold) {
        foldedSample = -foldThreshold - (foldedSample + foldThreshold);
    }

    // FM processing - unchanged
    float fmDepth = controlValue2;
    float pitch1 = controlValue;
    float pitch2 = controlValue3;
    phase2 += freq2 / args.sampleRate;
    if (phase2 >= 1.0f) phase2 -= 1.0f;
    float modulator = triangleWave(phase2) * 10.0f;
    float freq1 = pitch1 * (920.0f * pitch1) + modulator * fmDepth * 100.0f;
    phase1 += freq1 / args.sampleRate;
    if (phase1 >= 1.0f) phase1 -= 1.0f;
    float fmOutput = triangleWave(phase1) * 5.0f;

    // PD and switches - unchanged
    float pdDrive = controlValue2 * 32.f;
    float pitch3 = controlValue;
    int DIG = (int)(controlValue3 * 7.0f) + 1;
    float octaveMultiplier = (float)DIG;
    freq3 = pitch3 * octaveMultiplier * (275.0f * pitch3);
    phase3 += freq3 / args.sampleRate;
    if (phase3 >= 1.0f) phase3 -= 1.0f;
    float waveformOutput;
    
    // Switch handling
    bool switchState = params[RATSWITCH_PARAM].getValue() > 0.5f;
    bool gateState = inputs[MUSKRAT_INPUT].getVoltage() > 0.5f;
    float chaos = 0.0f;
    
    if(switchState || gateState) {
        waveformOutput = triangleWave2(phase3) * 5.0f;
        chaos = dis(gen);
        phaseGranular -= phaseGranularIncrement;
        if (phaseGranular < 0) {
            phaseGranular += WAVETABLE_LENGTH;
        }
        freq2 = pitch2 * (1.0f + (20.0f - 1.0f) * pitch1);
    } else {
        chaos = 1;
        phaseGranular += phaseGranularIncrement;
        phaseGranular = fmod(phaseGranular, WAVETABLE_LENGTH);
        freq2 = pitch2 * (900.0f + (920.0f - 900.0f) * pitch1);
        waveformOutput = sawtoothWave(phase3) * 5.0f;
    }

    // OG processing
    foldedSample = (foldedSample * distortion) + chaos;
    foldedSample += 1.0f;
    foldedSample = std::clamp(foldedSample, -5.0f, 5.0f);
    const float cutoffFrequency = 8000.0f;
    float alpha = 1.0f / (1.0f + (args.sampleRate / (2.0f * M_PI * cutoffFrequency)));
    static float previousOutput = 0.0f;
    float filteredSample = alpha * foldedSample + (1.0f - alpha) * previousOutput;
    previousOutput = filteredSample;

    // Granular - now using pre-normalized wavetables
    int phaseGranularIndex = static_cast<int>(phaseGranular);
    // Make sure we're within bounds
    phaseGranularIndex = std::clamp(phaseGranularIndex, 0, WAVETABLE_LENGTH - 1);
    float outputSampleGranular = normalizedWavetablesGranular[indexGranular][phaseGranularIndex];

    // PD output
    float pdOutput = waveformOutput * pdDrive;
    pdOutput = std::clamp(pdOutput, -5.0f, 5.0f);

    // Master decay - unchanged
    int range = (int)params[RANGE_PARAM].getValue();
    switch (range) {
        case 0:
            decayTime = 5.0f + (params[TAIL_PARAM].getValue() * 25.0f);
            break;
        case 1:
            decayTime = 30.0f + (params[TAIL_PARAM].getValue() * 170.0f);
            break;
        case 2:
            decayTime = 200.0f + (params[TAIL_PARAM].getValue() * 3800.0f);
            break;
    }
    float decayAlpha = exp(-1.0f / (args.sampleRate * (decayTime / 1000.0f)));
    bool bangState = inputs[BANG_INPUT].getVoltage() > 0.5f;
    if (bangState && !lastBangState) {
        pulseTriggered = true;
        envelopeValue = 1.0f;
        pulseTime = decayTime * (args.sampleRate / 1000.0f);
    }
    lastBangState = bangState;
    if (pulseTriggered) {
        envelopeValue *= decayAlpha;
        if (envelopeValue < 0.01f) {
            envelopeValue = 0.0f;
            pulseTriggered = false;
        }
    } else {
        envelopeValue = 0.0f;
    }

    // Granular output
    float granularOutput = (((outputSampleGranular * 5.f) + 2.5f) * 5.0f) * envelopeValue;
    granularOutput = std::clamp(granularOutput, -5.0f, 5.0f);

    // Output switching
    int ratSelect = params[SELECT_PARAM].getValue();
    switch (ratSelect) {
        case 1: // Muskrat
            outputs[AUDIO_OUTPUT].setVoltage(filteredSample * envelopeValue);
            break;
        case 2: // FM
            outputs[AUDIO_OUTPUT].setVoltage(fmOutput * envelopeValue);
            break;
        case 3: // PD
            outputs[AUDIO_OUTPUT].setVoltage(pdOutput * envelopeValue);
            break;
        case 4: // Granular
            outputs[AUDIO_OUTPUT].setVoltage(granularOutput);
            break;
    }

    // LED
    float ledBrightness = envelopeValue;
    lights[LED_LIGHT].setBrightnessSmooth(ledBrightness, args.sampleTime);
  }

};

struct MuskratWidget : ModuleWidget {
	MuskratWidget(Muskrat* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Muskrat.svg")));
        
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<CKSSThree>(mm2px(Vec(24.513, 20.315)), module, Muskrat::RANGE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(24.478, 108.396)), module, Muskrat::RATSWITCH_PARAM));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.603, 19.421)), module, Muskrat::TAIL_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.334, 41.886)), module, Muskrat::SCRATCH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.346, 64.212)), module, Muskrat::DIG_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(38.321, 86.273)), module, Muskrat::CHEW_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(62, 50)), module, Muskrat::SELECT_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.549, 19.244)), module, Muskrat::BANG_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.545, 41.872)), module, Muskrat::SCRATCH_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.548, 64.291)), module, Muskrat::DIG_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.467, 86.24)), module, Muskrat::CHEW_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.572, 108.196)), module, Muskrat::MUSKRAT_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.325, 108.226)), module, Muskrat::AUDIO_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(38.295, 98.285)), module, Muskrat::LED_LIGHT));
	}
};


Model* modelMuskrat = createModel<Muskrat, MuskratWidget>("Muskrat");
