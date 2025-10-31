#include "plugin.hpp"
#include "../res/wavetables/StargazerWavetables.hpp"
#include <vector>
#include "helpers/math_lut.hpp"


// Lookup tables
struct SinTableRange {
	static constexpr float min = -2.f * M_PI;
	static constexpr float max = 2.f * M_PI;
};

const Mapping::LookupTable_t<64, float> Sin =
	Mapping::LookupTable_t<64, float>::generate<SinTableRange>([](float x) { return sinf(x); });

struct SinFilterTableRange {
	static constexpr float min = -1.f * M_PI;
	static constexpr float max = 1.f * M_PI;
};

const Mapping::LookupTable_t<64, float> SinFilter =
	Mapping::LookupTable_t<64, float>::generate<SinFilterTableRange>([](float x) { return sinf(x); });

struct CosTableRange {
	static constexpr float min = -1.f * M_PI;
	static constexpr float max = 1.f * M_PI;
};

const Mapping::LookupTable_t<64, float> Cos =
	Mapping::LookupTable_t<64, float>::generate<CosTableRange>([](float x) { return cosf(x); });

struct Pow2NotesTableRange {
		static constexpr float min = -10.f;
		static constexpr float max = 10.f;
	};
	
const Mapping::LookupTable_t<64, float> Pow2Notes =
		Mapping::LookupTable_t<64, float>::generate<Pow2NotesTableRange>([](float x) { return powf(2.f, x); });
		
struct Pow2ReduxTableRange {
			static constexpr float min = 3.f;
			static constexpr float max = 15.f;
		};
const Mapping::LookupTable_t<64, float> Pow2Redux =
			Mapping::LookupTable_t<64, float>::generate<Pow2ReduxTableRange>([](float x) { return powf(2.f, x); });
			
struct Pow200CutoffTableRange {
			static constexpr float min = 0.f;
			static constexpr float max = 1.f;
			};
const Mapping::LookupTable_t<64, float> Pow200Cutoff =
			Mapping::LookupTable_t<64, float>::generate<Pow200CutoffTableRange>([](float x) { return powf(200.f, x); });
				
struct Stargazer : Module {
	enum ParamId {
        WIDTH_PARAM,
		PITCH_PARAM,
        FM_PARAM,
		GAIN_PARAM,
		SUB_PARAM,
		DETUNE_PARAM,
		VOL_PARAM,
		MAINWAVE_PARAM,
		REDUX_PARAM,
		MIX_PARAM,
		FREQ1_PARAM,
		FREQ2_PARAM,
		ALIAS_PARAM,
		RES2_PARAM,
		RES1_PARAM,
		DEPTH2_PARAM,
		WAVE1_PARAM,
		DEPTH3_PARAM,
		RATE2_PARAM,
		WAVE2_PARAM,
		RATE1_PARAM,
		RATE3_PARAM,
		WAVE3_PARAM,
		DEPTH1_PARAM,
		FILTERMODE1_PARAM,
		FILTERMODE2_PARAM,
		RANGE1_PARAM,
		RANGE2_PARAM,
		RANGE3_PARAM, 
		PARAMS_LEN
	};
	enum InputId {
		PITCHCV_INPUT,
        FMCV_INPUT,
		SUBCV_INPUT,
		WAVECV_INPUT,
		MIXCV_INPUT,
		DETUNECV_INPUT,
		FREQ1CV_INPUT,
		RES1CV_INPUT,
		ALIASCV_INPUT,
		REDUXCV_INPUT,
		FREQ2CV_INPUT,
		RES2CV_INPUT,
		LFO1WAVECV_INPUT,
		LFO1RATECV_INPUT,
		LFO1DEPTHCV_INPUT,
		LFO2WAVECV_INPUT,
		LFO2RATECV_INPUT,
		LFO2DEPTHCV_INPUT,
		LFO3WAVECV_INPUT,
		LFO3RATECV_INPUT,
		LFO3DEPTHCV_INPUT,
        WIDTHCV_INPUT,
		GAINCV_INPUT,
		VOLUMECV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTL_OUTPUT,
        OUTR_OUTPUT,
		LFO1OUT_OUTPUT,
		LFO2OUT_OUTPUT,
		LFO3OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LFO1LEDRED_LIGHT,
		LFO1LEDGREEN_LIGHT,
		LFO2LEDRED_LIGHT,
		LFO2LEDGREEN_LIGHT,
		LFO3LEDRED_LIGHT,
		LFO3LEDGREEN_LIGHT,
		LIGHTS_LEN
	};

	Stargazer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM, 0.f, 1.f, 0.1f, "Pitch", "hz", 500.f, 1.f); // 1hz - 500hz
        configParam(FM_PARAM, 0.f, 1.f, 0.f, "FM", "%", 0.f, 100.f); // FM Attenuator

		configSwitch(SUB_PARAM, 0.f, 1.f, 0.f, "Sub Oscillator", {"Off", "On"}); // Turn osc2 into sub oscillator (kept for compatibility)
		configParam(MAINWAVE_PARAM, 1.f, 88.f, 0.f, "Wavetable Select"); // 1 - 88 morphing wavetable select
		configParam(MIX_PARAM, 0.f, 1.f, 0.f, "Oscillator 2 Volume", "%", 0.f, 100.f); 
		configParam(DETUNE_PARAM, -1.f, 1.f, 0.f, "Oscillator 2 Detune", "hz", 0.f, 5.f); // +/-5hz 

		configParam(FREQ1_PARAM, 0.f, 1.f, 1.f, "Filter 1 Cutoff", "hz", 200.f, 80.f); // 80hz - 16khz
		configParam(RES1_PARAM, 0.f, 1.f, 0.f, "Filter 1 Resonance", "%", 0.f, 100.f); // Q 1-5

		configSwitch(FILTERMODE1_PARAM, 0.f, 4.f, 0.f, "Filter Mode 1", {"Lowpass", "Bandpass", "Notch", "Highpass", "Off"});
		configSwitch(FILTERMODE2_PARAM, 0.f, 4.f, 0.f, "Filter Mode 2", {"Lowpass", "Bandpass", "Notch", "Highpass", "Off"});

		configParam(ALIAS_PARAM, 1.f, 0.f, 1.f, "Sample Rate", "hz", 878.04f, 20.5f); // 18khz - 1hz 
		configSwitch(REDUX_PARAM, 0.f, 12.f, 0.f, "Bit Depth", {"16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4"});

		configParam(FREQ2_PARAM, 0.f, 1.f, 1.f, "Filter 2 Cutoff", "hz", 200.f, 80.f); // 80hz - 5khz
		configParam(RES2_PARAM, 0.f, 1.f, 0.f, "Filter 2 Resonance", "%", 0.f, 100.f); // Q 1-5

		configSwitch(WAVE1_PARAM, 0.f, 5.f, 0.f, "LFO 1 Waveshape", {"Sine", "Triangle", "Ramp Up", "Ramp Down", "Square", "Random"} );
		configParam(RATE1_PARAM, 0.f, 1.f, 0.f, "LFO 1 Frequency", "hz"); // 0.05hz - 50hz
		configParam(DEPTH1_PARAM, 0.f, 1.f, 0.f, "LFO 1 Depth", "%", 0.f, 100.f);

		configSwitch(WAVE2_PARAM, 0.f, 5.f, 0.f, "LFO 2 Waveshape", {"Sine", "Triangle", "Ramp Up", "Ramp Down", "Square", "Random"});
		configParam(RATE2_PARAM, 0.f, 1.f, 0.f, "LFO 2 Frequency", "hz"); // 0.05hz - 50hz
		configParam(DEPTH2_PARAM, 0.f, 1.f, 0.f, "LFO 2 Depth", "%", 0.f, 100.f);

		configSwitch(WAVE3_PARAM, 0.f, 5.f, 0.f, "LFO 3 Waveshape", {"Sine", "Triangle", "Ramp Up", "Ramp Down", "Square", "Random"});
		configParam(RATE3_PARAM, 0.f, 1.f, 0.f, "LFO 3 Frequency", "hz"); // 0.05hz - 50hz
		configParam(DEPTH3_PARAM, 0.f, 1.f, 0.f, "LFO 3 Depth", "%", 0.f, 100.f);

        configParam(WIDTH_PARAM, 0.f, 1.f, 0.f, "Spread", "%", 0.f, 100.f); // Unity - 100x gain 
		configParam(GAIN_PARAM, 0.f, 1.f, 0.f, "Gain", "x", 100.f, 1.f); // Unity - 100x gain 
		configParam(VOL_PARAM, 0.f, 1.f, 1.f, "Volume", "%", 0.f, 100.f);

		configSwitch(RANGE1_PARAM, 0.f, 2.f, 0.f, "LFO 1 Range", {"Slow", "Medium", "Fast"});
		configSwitch(RANGE2_PARAM, 0.f, 2.f, 2.f, "LFO 2 Range", {"Slow", "Medium", "Fast"});
		configSwitch(RANGE3_PARAM, 0.f, 2.f, 1.f, "LFO 3 Range", {"Slow", "Medium", "Fast"});

		configInput(PITCHCV_INPUT, "1v/Oct");
		configInput(SUBCV_INPUT, "Sub CV");
		configInput(WAVECV_INPUT, "Wave CV");
		configInput(MIXCV_INPUT, "Mix CV");
		configInput(DETUNECV_INPUT, "Detune CV");
		configInput(FREQ1CV_INPUT, "Filter 1 Cutoff CV");
        configInput(FMCV_INPUT, "FM CV");
		configInput(RES1CV_INPUT, "Filter 1 Resonance CV");
		configInput(ALIASCV_INPUT, "Alias CV");
		configInput(REDUXCV_INPUT, "Redux CV");
		configInput(FREQ2CV_INPUT, "Filter 2 Cutoff CV");
		configInput(RES2CV_INPUT, "Filter 2 Resonance CV");
		configInput(LFO1WAVECV_INPUT, "LFO 1 Waveshape CV");
		configInput(LFO1RATECV_INPUT, "LFO 1 Rate CV");
		configInput(LFO1DEPTHCV_INPUT, "LFO 1 Depth CV");
		configInput(LFO2WAVECV_INPUT, "LFO 2 Waveshape CV");
		configInput(LFO2RATECV_INPUT, "LFO 2 Rate CV");
		configInput(LFO2DEPTHCV_INPUT, "LFO 2 Depth CV");
		configInput(LFO3WAVECV_INPUT, "LFO 3 Waveshape CV");
		configInput(LFO3RATECV_INPUT, "LFO 3 Rate CV");
		configInput(LFO3DEPTHCV_INPUT, "LFO 3 Depth CV");
        configInput(WIDTHCV_INPUT, "Spread CV");
		configInput(GAINCV_INPUT, "Gain CV");
		configInput(VOLUMECV_INPUT, "Volume CV");
		configOutput(OUTL_OUTPUT, "Audio Left");
        configOutput(OUTR_OUTPUT, "Audio Right");
		configOutput(LFO1OUT_OUTPUT, "LFO 1");
		configOutput(LFO2OUT_OUTPUT, "LFO 2");
		configOutput(LFO3OUT_OUTPUT, "LFO 3");

		// --- Load wavetables from StargazerWavetables.hpp ---
		// This header defines StargazerWavetables as an array of normalized float samples (-1.0f to 1.0f)
		extern const float StargazerWavetables[]; 
		extern const int StargazerNumTables;
		extern const int StargazerTableSize;

		tableSize = StargazerTableSize;
		numTables = StargazerNumTables;

		wavetables.resize(numTables);
		for (int i = 0; i < numTables; i++) {
			const float* src = &StargazerWavetables[i * tableSize];
			wavetables[i].assign(src, src + tableSize);
		}
		// Initialize all knob parameters so they act as if already touched
for (int i = 0; i < PARAMS_LEN; i++) {
    // Only mark knobs (continuous parameters, not switches)
    if (paramQuantities[i] && !paramQuantities[i]) {
        paramQuantities[i]->setValue(paramQuantities[i]->getValue());
    }
}

	}

	// Main wavetable
	std::vector<std::vector<float>> wavetables; 
	int tableSize = 600;       // samples per frame
	int numTables = 88;        // total tables
	float sampleRate = 44100.f; // default fallback

	// --- Oscillator phase (only one main oscillator now) ---
	float phase1 = 0.f;
	float phase2 = 0.f; 

	// Filter 1 cached parameters
	int lastFilterMode1 = -1;
	float lastCutoffHz = -1.f;
	float lastQ = -1.f;

	// Filter 1 biquad coefficients
	float lp1_b0 = 0.f, lp1_b1 = 0.f, lp1_b2 = 0.f;
	float lp1_a0 = 1.f, lp1_a1 = 0.f, lp1_a2 = 0.f;

	// Optional: normalize gain
	float lp1_normGain = 1.f;

	// Filter 1 state
	float lp1_x1 = 0.f, lp1_x2 = 0.f;
	float lp1_y1 = 0.f, lp1_y2 = 0.f;


	// Filter 2 cached params
	int lastFilterMode2 = -1;
	float lastCutoffHz2 = -1.f;
	float lastQ2 = -1.f;

	// Filter 2 biquad coefficients
	float lp2_b0 = 0.f, lp2_b1 = 0.f, lp2_b2 = 0.f;
	float lp2_a0 = 1.f, lp2_a1 = 0.f, lp2_a2 = 0.f;

	// Normalization gain
	float lp2_normGain = 1.f;

	// Filter 2 state
	float lp2_x1 = 0.f, lp2_x2 = 0.f;
	float lp2_y1 = 0.f, lp2_y2 = 0.f;

	// LFO1 member variables
	float lfo1Phase = 0.f;
	float lfo1StepCounter = 1.f;    // for stepped random waveform
	float lfo1RandValue = 0.f;      // for stepped random waveform
	float lfo1Value = 0.f;

	// LFO2 member variables
	float lfo2Phase = 0.f;
	float lfo2StepCounter = 1.f;    // for stepped random waveform
	float lfo2RandValue = 0.f;      // for stepped random waveform
	float lfo2Value = 0.f;

	// LFO3 member variables
	float lfo3Phase = 0.f;
	float lfo3StepCounter = 1.f;    // for stepped random waveform
	float lfo3RandValue = 0.f;      // for stepped random waveform
	float lfo3Value = 0.f;

	// --- Simple delay state ---
	std::vector<float> delayBuffer;
	int delayIndex = 0;
	float delayMs = 20.f; // adjustable delay time in milliseconds

	// Alias / sample rate reducer
	float lastAliasKnob = -1.f;
	float lastFadeFactor = -1.f;
	float lastAliasRate = -1.f;
	float aliasCounter = 0.f;
	float lastSample = 0.f;

	// Redux bit reduction
	int lastReduxBitDepth = -1;
	float lastMaxVal = -1.f;

	// Final gain stage
	float lastGainControl = -1.f;
	float lastGain = -1.f;

	// LFO2 Tremolo depth
	float lastLFO2Depth = -1.f;

	float oscFreq2 = 0.f; 
	 
	void process(const ProcessArgs& args) override {

	auto processLFO = [&](int rateParam, int depthParam, int waveParam,
	                      int rateCV, int depthCV, int waveCV,
	                      float& phase, float& stepCounter, float& randValue,
	                      int outId, int ledRedId, int ledGreenId,
	                      int rangeParam, // new: LFO range switch
	                      float* outValue = nullptr) {

	    float rate = params[rateParam].getValue();
	    if (inputs[rateCV].isConnected()) rate += inputs[rateCV].getVoltage() / 10.f;
	    rate = std::clamp(rate, 0.f, 1.f);

	    // Determine frequency range based on range switch
	    float minFreq = 0.05f;
	    float maxFreq = 50.f;
	    int range = (int)params[rangeParam].getValue(); // 0,1,2 for three positions
	    switch (range) {
	        case 0: minFreq = 0.01f; maxFreq = 0.1f; break;   // Range1
	        case 1: minFreq = 0.01f; maxFreq = 10.f; break;   // Range2
	        case 2: minFreq = 0.05f; maxFreq = 50.f; break;   // Range3
	    }
	    float freq = minFreq * powf(maxFreq / minFreq, rate);

	// --- Dynamically update knob display based on LFO's range switch ---
	if (paramQuantities.size() > (size_t)rateParam) {
	    auto* q = paramQuantities[rateParam];

	    // Read the LFO's range switch
	    int lfoRange = (int)params[rangeParam].getValue();

	    switch (lfoRange) {
	        case 0: // Slow range
	            q->displayBase = 10.f;
	            q->displayMultiplier = 0.01f; 
	            break;
	        case 1: // Medium range
	            q->displayBase = 200.f;
	            q->displayMultiplier = 0.05f;
	            break;
	        case 2: // Fast range
	            q->displayBase = 1000.f;
	            q->displayMultiplier = 0.05f;
	            break;
	    }
	}

	    // Depth
	    float depth = params[depthParam].getValue();
	    if (inputs[depthCV].isConnected()) depth += inputs[depthCV].getVoltage() / 10.f;
	    depth = std::clamp(depth, 0.f, 1.f);

	    // Phase increment
	    phase += freq * args.sampleTime;
	    if (phase >= 1.f) phase -= 1.f;

	    // Waveform selection
	    int wave = std::clamp((int)roundf(params[waveParam].getValue() +
	                 (inputs[waveCV].isConnected() ? std::clamp(inputs[waveCV].getVoltage(), -5.f, 5.f)/2.f : 0.f)), 0, 5);

	    float value = 0.f;
	    switch (wave) {
	        case 0: value = Sin(2.f * float(M_PI) * phase); break;
	        case 1: value = 1.f - 4.f * fabsf(phase - 0.5f); break;
	        case 2: value = 2.f * phase - 1.f; break;
	        case 3: value = 1.f - 2.f * phase; break;
	        case 4: value = (phase < 0.5f) ? 1.f : -1.f; break;
	        case 5:
	            if (stepCounter >= 1.f) {
	                randValue = 2.f*((float)rand()/RAND_MAX)-1.f;
	                stepCounter -= 1.f;
	            }
	            stepCounter += freq * args.sampleTime * 2.f;
	            value = randValue;
	            break;
	    }

	    value *= depth * 5.f;
	    outputs[outId].setVoltage(value);

	    // LED logic
	    lights[ledGreenId].setBrightnessSmooth(std::clamp(value / 5.f, 0.f, 1.f), args.sampleTime);
	    lights[ledRedId].setBrightnessSmooth(std::clamp(-value / 5.f, 0.f, 1.f), args.sampleTime);

	    if (outValue)
	        *outValue = value;
	};

	processLFO(RATE1_PARAM, DEPTH1_PARAM, WAVE1_PARAM,
	           LFO1RATECV_INPUT, LFO1DEPTHCV_INPUT, LFO1WAVECV_INPUT,
	           lfo1Phase, lfo1StepCounter, lfo1RandValue,
	           LFO1OUT_OUTPUT, LFO1LEDRED_LIGHT, LFO1LEDGREEN_LIGHT,
	           RANGE1_PARAM, &lfo1Value);

	processLFO(RATE2_PARAM, DEPTH2_PARAM, WAVE2_PARAM,
	           LFO2RATECV_INPUT, LFO2DEPTHCV_INPUT, LFO2WAVECV_INPUT,
	           lfo2Phase, lfo2StepCounter, lfo2RandValue,
	           LFO2OUT_OUTPUT, LFO2LEDRED_LIGHT, LFO2LEDGREEN_LIGHT,
	           RANGE2_PARAM, &lfo2Value);

	processLFO(RATE3_PARAM, DEPTH3_PARAM, WAVE3_PARAM,
	           LFO3RATECV_INPUT, LFO3DEPTHCV_INPUT, LFO3WAVECV_INPUT,
	           lfo3Phase, lfo3StepCounter, lfo3RandValue,
	           LFO3OUT_OUTPUT, LFO3LEDRED_LIGHT, LFO3LEDGREEN_LIGHT,
	           RANGE3_PARAM, &lfo3Value);
			   
	// --- START OF AUDIO SECTION ---
	sampleRate = 1.f / args.sampleTime;

// --- Frequency control (1–500 Hz base + 1V/oct CV) ---
float baseFreqParam = params[PITCH_PARAM].getValue();
float baseFreq = 1.f + baseFreqParam * (500.f - 1.f);
float pitchCV = inputs[PITCHCV_INPUT].isConnected() ? inputs[PITCHCV_INPUT].getVoltage() : 0.f;
// FM input (normalized to LFO3)
float fmCV = inputs[FMCV_INPUT].isConnected() ? inputs[FMCV_INPUT].getVoltage() : (lfo3Value * 0.2f);

// FM knob acts as attenuator
fmCV *= params[FM_PARAM].getValue();

// total pitch = 1V/oct + FM
float totalPitchCV = pitchCV + fmCV;

// final frequency (1–500 Hz)
float freqRoot = std::clamp(baseFreq * Pow2Notes(totalPitchCV), 1.f, 500.f);


	// --- Waveform morphing (for the single oscillator) ---
	float waveCV = inputs[WAVECV_INPUT].isConnected() ? inputs[WAVECV_INPUT].getVoltage() : lfo1Value;
	float waveParam = 1.0f + std::clamp((params[MAINWAVE_PARAM].getValue() - 1.0f) / 87.0f + waveCV / 10.0f, 0.f, 1.f) * 87.f;
	float wavePos = std::clamp(waveParam - 1.f, 0.f, numTables - 1.f);
	int i0 = (int)wavePos;
	int i1 = std::min(i0 + 1, numTables - 1);
	float frac = wavePos - i0;

	// --- Helper for wavetable interpolation ---
	auto interpWave = [&](float phase) {
	    float pos = phase * (tableSize - 1);
	    int idx0 = (int)pos;
	    int idx1 = (idx0 + 1) % tableSize;
	    float fracPos = pos - idx0;
	    float s00 = wavetables[i0][idx0];
	    float s01 = wavetables[i0][idx1];
	    float s10 = wavetables[i1][idx0];
	    float s11 = wavetables[i1][idx1];
	    float a = s00 + (s01 - s00) * fracPos;
	    float b = s10 + (s11 - s10) * fracPos;
	    return a + (b - a) * frac;
	};

	// --- Oscillator 1 (main) ---
	phase1 += freqRoot / sampleRate;
	if (phase1 >= 1.f) phase1 -= 1.f;
	float osc1 = interpWave(phase1);


// Detune
float detuneKnob = params[DETUNE_PARAM].getValue() * 5.f;
float detuneCV = 0.f;
if (inputs[DETUNECV_INPUT].isConnected())
detuneCV = inputs[DETUNECV_INPUT].getVoltage();
float detuneCombined = std::clamp(detuneKnob + detuneCV, -5.f, 5.f);
float detune = detuneCombined;  // ±5 Hz total range

bool subActive = (params[SUB_PARAM].getValue() > 0.5f);
	if (inputs[SUBCV_INPUT].isConnected()) {
		if (inputs[SUBCV_INPUT].getVoltage() >= 1.f)
			subActive = !subActive;
	}
		
	// Apply sub mode (after detune)
	if (subActive) {
		oscFreq2 = ((freqRoot * 0.5f) + detune);
	}
	else {
		oscFreq2 = freqRoot + detune; 
	}


// Advance oscillator 2 phase
phase2 += oscFreq2 / sampleRate;
if (phase2 >= 1.f)
    phase2 -= 1.f;

// Use identical wavetable lookup as oscillator 1
float osc2 = interpWave(phase2);

// --- Mix control ---
float mix = params[MIX_PARAM].getValue();
if (inputs[MIXCV_INPUT].isConnected())
    mix += inputs[MIXCV_INPUT].getVoltage() / 10.f;
mix = std::clamp(mix, 0.f, 1.f);

// Crossfade between osc1 and osc2
float oscMix = osc1 + (osc2 * mix);

	// --- Mix / final sample (only osc1 now) ---
	float sample = oscMix;

    // --- Filter 1 cutoff + resonance ---
	float cutoff = params[FREQ1_PARAM].getValue();
	if (inputs[FREQ1CV_INPUT].isConnected()){
    	cutoff += inputs[FREQ1CV_INPUT].getVoltage() / 10.f;
	}
	cutoff = std::clamp(cutoff, 0.f, 1.f);
	float cutoffHz = 80.f * Pow200Cutoff(cutoff);

    float res = params[RES1_PARAM].getValue();
    if (inputs[RES1CV_INPUT].isConnected())
        res += inputs[RES1CV_INPUT].getVoltage() / 10.f;
    res = std::clamp(res, 0.f, 1.f);
    float Q = 1.f + res * 4.f;

// --- Filter 1 biquad with selectable mode + gain normalization ---
float y = sample; // default passthrough
int mode = (int) params[FILTERMODE1_PARAM].getValue(); // 0=LP,1=BP,2=Notch,3=HP,4=Off

// Only recalc coefficients if mode, cutoff, or Q changed
if (mode != 4 && (mode != lastFilterMode1 || cutoffHz != lastCutoffHz || Q != lastQ)) {
    float w0 = 2.f * float(M_PI) * cutoffHz / sampleRate;
    float cos_w0 = Cos(w0);
    float sin_w0 = SinFilter(w0);
    float alpha = sin_w0 / (2.f * Q);

    switch (mode) {
        case 0: // Lowpass
            lp1_b0 = (1.f - cos_w0) / 2.f;
            lp1_b1 = 1.f - cos_w0;
            lp1_b2 = (1.f - cos_w0) / 2.f;
            break;
        case 1: // Bandpass
            lp1_b0 = sin_w0 / 2.f;
            lp1_b1 = 0.f;
            lp1_b2 = -sin_w0 / 2.f;
            break;
        case 2: // Notch
            lp1_b0 = 1.f;
            lp1_b1 = -2.f * cos_w0;
            lp1_b2 = 1.f;
            break;
        case 3: // Highpass
            lp1_b0 = (1.f + cos_w0) / 2.f;
            lp1_b1 = -(1.f + cos_w0);
            lp1_b2 = (1.f + cos_w0) / 2.f;
            break;
    }

    lp1_a0 = 1.f + alpha;
    lp1_a1 = -2.f * cos_w0;
    lp1_a2 = 1.f - alpha;

    // precompute normalization gain per mode
    switch (mode) {
        case 0: lp1_normGain = 1.0f; break; // LP
        case 1: lp1_normGain = 0.5f; break; // BP
        case 2: lp1_normGain = 0.8f; break; // Notch
        case 3: lp1_normGain = 1.0f; break; // HP
    }

    // store current params for caching
    lastFilterMode1 = mode;
    lastCutoffHz = cutoffHz;
    lastQ = Q;
}

// --- biquad process ---
if (mode != 4) {
    y = (lp1_b0 / lp1_a0) * sample
      + (lp1_b1 / lp1_a0) * lp1_x1
      + (lp1_b2 / lp1_a0) * lp1_x2
      - (lp1_a1 / lp1_a0) * lp1_y1
      - (lp1_a2 / lp1_a0) * lp1_y2;

    // shift states
    lp1_x2 = lp1_x1; lp1_x1 = sample;
    lp1_y2 = lp1_y1; lp1_y1 = y;

    // apply normalization
    y *= lp1_normGain;
}

// clamp output
float scaledOutput = std::clamp(y, -10.f, 10.f);

   // --- Alias (sample rate reducer) ---
float aliasKnob = 1.f - params[ALIAS_PARAM].getValue();
if (inputs[ALIASCV_INPUT].isConnected())
    aliasKnob += inputs[ALIASCV_INPUT].getVoltage() / 10.f;
aliasKnob = std::clamp(aliasKnob, 0.f, 1.f);

// Recompute fadeFactor & aliasRate only if aliasKnob changed
if (aliasKnob != lastAliasKnob) {
    lastFadeFactor = (aliasKnob <= 0.05f) ? (aliasKnob / 0.05f) : 1.f;
    lastAliasRate = 25.f + (18000.f - aliasKnob * (18000.f - 1.f));
    lastAliasKnob = aliasKnob;
}

aliasCounter += lastAliasRate * args.sampleTime;
if (aliasCounter >= 1.f) {
    lastSample = scaledOutput;
    aliasCounter -= 1.f;
}
float finalOutput = scaledOutput * (1.f - lastFadeFactor) + lastSample * lastFadeFactor;

// --- Redux bit reduction ---
int reduxBitDepth = 16 - (int)params[REDUX_PARAM].getValue();
reduxBitDepth = std::clamp(reduxBitDepth, 4, 16);
if (inputs[REDUXCV_INPUT].isConnected()) {
    float reduxCV = std::clamp(inputs[REDUXCV_INPUT].getVoltage(), -5.f, 5.f);
    reduxBitDepth = std::clamp((int)roundf(16.f - ((reduxCV + 5.f)/10.f)*(16.f-4.f)), 4, 16);
}
// Only recalc maxVal if reduxBitDepth changed
if (reduxBitDepth != lastReduxBitDepth) {
    lastMaxVal = Pow2Redux(reduxBitDepth - 1) - 1.f;
    lastReduxBitDepth = reduxBitDepth;
}

float normalized = std::clamp(finalOutput / 10.f, -1.f, 1.f);
float quantized = roundf(normalized * lastMaxVal) / lastMaxVal;
finalOutput = quantized * 10.f;


	// --- Filter 2 cutoff + resonance ---
float freq2CV = inputs[FREQ2CV_INPUT].isConnected() ? inputs[FREQ2CV_INPUT].getVoltage() : (lfo3Value * 0.2f);
float cutoff2 = params[FREQ2_PARAM].getValue() + freq2CV / 10.f;
cutoff2 = std::clamp(cutoff2, 0.f, 1.f);
float cutoffHz2 = 80.f * Pow200Cutoff(cutoff2);

float res2 = params[RES2_PARAM].getValue();
if (inputs[RES2CV_INPUT].isConnected())
    res2 += inputs[RES2CV_INPUT].getVoltage() / 10.f;
res2 = std::clamp(res2, 0.f, 1.f);
float Q2 = 1.f + res2 * 4.f;

int mode2 = (int) params[FILTERMODE2_PARAM].getValue(); // 0=LP,1=BP,2=Notch,3=HP,4=Off
float y2 = finalOutput; // local variable, per-sample

// --- Recompute biquad coefficients only if parameters changed ---
if (mode2 != 4 && (mode2 != lastFilterMode2 || cutoffHz2 != lastCutoffHz2 || Q2 != lastQ2)) {
    float w0_2 = 2.f * float(M_PI) * cutoffHz2 / sampleRate;
    float cos_w0_2 = Cos(w0_2);
    float sin_w0_2 = Sin(w0_2);
    float alpha2 = sin_w0_2 / (2.f * Q2);

    switch (mode2) {
        case 0: // Lowpass
            lp2_b0 = (1.f - cos_w0_2) / 2.f;
            lp2_b1 = 1.f - cos_w0_2;
            lp2_b2 = (1.f - cos_w0_2) / 2.f;
            break;
        case 1: // Bandpass
            lp2_b0 = sin_w0_2 / 2.f;
            lp2_b1 = 0.f;
            lp2_b2 = -sin_w0_2 / 2.f;
            break;
        case 2: // Notch
            lp2_b0 = 1.f;
            lp2_b1 = -2.f * cos_w0_2;
            lp2_b2 = 1.f;
            break;
        case 3: // Highpass
            lp2_b0 = (1.f + cos_w0_2) / 2.f;
            lp2_b1 = -(1.f + cos_w0_2);
            lp2_b2 = (1.f + cos_w0_2) / 2.f;
            break;
    }

    lp2_a0 = 1.f + alpha2;
    lp2_a1 = -2.f * cos_w0_2;
    lp2_a2 = 1.f - alpha2;

    // precompute normalization gain
    switch (mode2) {
        case 0: lp2_normGain = 1.0f; break; // LP
        case 1: lp2_normGain = 0.5f; break; // BP
        case 2: lp2_normGain = 0.8f; break; // Notch
        case 3: lp2_normGain = 1.0f; break; // HP
    }

    // cache current params
    lastFilterMode2 = mode2;
    lastCutoffHz2 = cutoffHz2;
    lastQ2 = Q2;
}

// --- Biquad process ---
if (mode2 != 4) {
    y2 = (lp2_b0 / lp2_a0) * finalOutput
       + (lp2_b1 / lp2_a0) * lp2_x1
       + (lp2_b2 / lp2_a0) * lp2_x2
       - (lp2_a1 / lp2_a0) * lp2_y1
       - (lp2_a2 / lp2_a0) * lp2_y2;

    lp2_x2 = lp2_x1; lp2_x1 = finalOutput;
    lp2_y2 = lp2_y1; lp2_y1 = y2;

    y2 *= lp2_normGain;
}

// --- Final output ---
float finalOutput2 = std::clamp(y2 * 0.5f, -10.f, 10.f);

// --- Final gain stage with CV ---
float gainControl = params[GAIN_PARAM].getValue();
if (inputs[GAINCV_INPUT].isConnected())
    gainControl += inputs[GAINCV_INPUT].getVoltage() / 10.f;
gainControl = std::clamp(gainControl, 0.f, 1.f);

// Recompute gain only if control changed
if (gainControl != lastGainControl) {
    lastGain = 19.f + gainControl * 100.f;
    lastGainControl = gainControl;
}

// Apply gain and clip
float clipped = std::clamp(finalOutput2 * lastGain, -10.f, 10.f);


// --- LFO2 Tremolo (pre-volume) ---
float lfo2Depth = params[DEPTH2_PARAM].getValue();
if (inputs[LFO2DEPTHCV_INPUT].isConnected())
    lfo2Depth += inputs[LFO2DEPTHCV_INPUT].getVoltage() / 10.f;
lfo2Depth = std::clamp(lfo2Depth, 0.f, 1.f);

// Only update cached value if it changed
if (lfo2Depth != lastLFO2Depth) {
    lastLFO2Depth = lfo2Depth;
}

// Use lastLFO2Depth in downstream calculations
float tremoloDepth = lastLFO2Depth;

// LFO2 (±5 V) → 0–1 amplitude modulation
float tremolo = 0.5f * (1.f + (lfo2Value / 5.f) * tremoloDepth);
tremolo = std::clamp(tremolo, 0.f, 1.f);

// Stereo width 
int delaySamples = (int)(delayMs * 0.001f * args.sampleRate);
if ((int)delayBuffer.size() < delaySamples + 1)
    delayBuffer.assign(delaySamples + 1, 0.f);

float delayedSample = delayBuffer[delayIndex];
delayBuffer[delayIndex] = clipped;
delayIndex++;
if (delayIndex >= delaySamples)
    delayIndex = 0;

float drySignal = clipped * tremolo;
float wetSignal = delayedSample * tremolo;

float volControl = params[VOL_PARAM].getValue();
if (inputs[VOLUMECV_INPUT].isConnected())
    volControl += inputs[VOLUMECV_INPUT].getVoltage() / 10.f;
volControl = std::clamp(volControl, 0.f, 1.f);
drySignal *= volControl;
wetSignal *= volControl;

float width = params[WIDTH_PARAM].getValue();
if (inputs[WIDTHCV_INPUT].isConnected())
    width += inputs[WIDTHCV_INPUT].getVoltage() / 10.f;
width = std::clamp(width, 0.f, 1.f);

// crossfade wet signal for first 5% of knob, keep overall volume constant
float wetFade = (width <= 0.05f) ? (width / 0.05f) : 1.f;
float scale = (width <= 0.05f) ? 1.f / (0.5f + 0.5f * wetFade) : 1.f;

float leftOut  = scale * ((1.f - width) * 0.5f * (drySignal + wetSignal * wetFade) + width * drySignal);
float rightOut = scale * ((1.f - width) * 0.5f * (drySignal + wetSignal * wetFade) + width * (wetSignal * wetFade));

outputs[OUTL_OUTPUT].setVoltage(leftOut);
outputs[OUTR_OUTPUT].setVoltage(rightOut);
}
};

struct StargazerWidget : ModuleWidget {
	StargazerWidget(Stargazer* module) {
		setModule(module);
	setPanel(createPanel(
		asset::plugin(pluginInstance, "res/panels/Stargazer.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<NineMMKnob>(mm2px(Vec(25.744, 70.211)), module, Stargazer::FILTERMODE1_PARAM));
		addParam(createParamCentered<NineMMKnob>(mm2px(Vec(112.163, 71.889)), module, Stargazer::FILTERMODE2_PARAM));
        addParam(createParamCentered<NineMMKnob>(mm2px(Vec(47.715, 8.739)), module, Stargazer::FM_PARAM));
        addParam(createParamCentered<NineMMKnob>(mm2px(Vec(78.869, 10.580)), module, Stargazer::WIDTH_PARAM));

        addParam(createParamCentered<StargazerDavies>(mm2px(Vec(26.918, 12.382)), module, Stargazer::PITCH_PARAM));
		addParam(createParamCentered<StargazerDavies>(mm2px(Vec(96.236, 14.975)), module, Stargazer::GAIN_PARAM));
		addParam(createParamCentered<StargazerDavies>(mm2px(Vec(10.681, 32.195)), module, Stargazer::MAINWAVE_PARAM));
		addParam(createParamCentered<StargazerDavies>(mm2px(Vec(122.509, 27.037)), module, Stargazer::VOL_PARAM));

		addParam(createParamCentered<TwoPos>(mm2px(Vec(40.613, 16.470)), module, Stargazer::SUB_PARAM));

		addParam(createParamCentered<ThreePos>(mm2px(Vec(13.450, 113.833)), module, Stargazer::RANGE1_PARAM));
		addParam(createParamCentered<ThreePos>(mm2px(Vec(67.963, 101.533)), module, Stargazer::RANGE2_PARAM));
		addParam(createParamCentered<ThreePos>(mm2px(Vec(102.129, 113.242)), module, Stargazer::RANGE3_PARAM));


		addParam(createParamCentered<MediumKnob>(mm2px(Vec(55.727, 21.557)), module, Stargazer::DETUNE_PARAM));
		addParam(createParamCentered<MediumKnob>(mm2px(Vec(35.809, 36.884)), module, Stargazer::MIX_PARAM));
		addParam(createParamCentered<MediumKnob>(mm2px(Vec(78.258, 33.499)), module, Stargazer::REDUX_PARAM));

		addParam(createParamCentered<Davies1900hLargeBlackKnob>(mm2px(Vec(16.256, 53.107)), module, Stargazer::FREQ1_PARAM));
		addParam(createParamCentered<Davies1900hLargeBlackKnob>(mm2px(Vec(96.873, 53.957)), module, Stargazer::FREQ2_PARAM));

		addParam(createParamCentered<BefacoBigKnobBlack>(mm2px(Vec(60.422, 55.071)), module, Stargazer::ALIAS_PARAM));

		addParam(createParamCentered<StargazerTinyKnob>(mm2px(Vec(124.763, 61.375)), module, Stargazer::RES2_PARAM));
		addParam(createParamCentered<StargazerTinyKnob>(mm2px(Vec(40.412, 65.557)), module, Stargazer::RES1_PARAM));

		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(93.262, 76.91)), module, Stargazer::DEPTH2_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(15.131, 83.658)), module, Stargazer::WAVE1_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(122.023, 89.045)), module, Stargazer::DEPTH3_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(78.326, 89.859)), module, Stargazer::RATE2_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(54.962, 91.29)), module, Stargazer::WAVE2_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(23.225, 102.295)), module, Stargazer::RATE1_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(108.733, 102.098)), module, Stargazer::RATE3_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(88.822, 109.405)), module, Stargazer::WAVE3_PARAM));
		addParam(createParamCentered<StargazerLFOKnob>(mm2px(Vec(40.523, 112.668)), module, Stargazer::DEPTH1_PARAM));
		
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 12.963)), module, Stargazer::PITCHCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.55, 12.963)), module, Stargazer::FMCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.591, 12.963)), module, Stargazer::SUBCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 24.709)), module, Stargazer::WAVECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 24.709)), module, Stargazer::MIXCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.591, 24.709)), module, Stargazer::DETUNECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 36.493)), module, Stargazer::FREQ1CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 36.493)), module, Stargazer::RES1CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.591, 36.493)), module, Stargazer::ALIASCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 48.277)), module, Stargazer::REDUXCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 48.277)), module, Stargazer::FREQ2CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.591, 48.277)), module, Stargazer::RES2CV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.489, 60.158)), module, Stargazer::WIDTHCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 60.158)), module, Stargazer::GAINCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.804, 60.158)), module, Stargazer::VOLUMECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 77.49)), module, Stargazer::LFO1WAVECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 77.49)), module, Stargazer::LFO2WAVECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.615, 77.49)), module, Stargazer::LFO3WAVECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 90.238)), module, Stargazer::LFO1RATECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 90.238)), module, Stargazer::LFO2RATECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.615, 90.238)), module, Stargazer::LFO3RATECV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 102.986)), module, Stargazer::LFO1DEPTHCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 102.986)), module, Stargazer::LFO2DEPTHCV_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(156.615, 102.986)), module, Stargazer::LFO3DEPTHCV_INPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(138.373, 115.734)), module, Stargazer::LFO1OUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(147.477, 115.734)), module, Stargazer::LFO2OUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(156.615, 115.734)), module, Stargazer::LFO3OUT_OUTPUT));

        addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(110.173, 6.085)), module, Stargazer::OUTL_OUTPUT));
        addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(121.641, 8.975)), module, Stargazer::OUTR_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(29.232, 91.869)), module, Stargazer::LFO1LEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(29.232, 91.869)), module, Stargazer::LFO1LEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(69.881, 80.617)), module, Stargazer::LFO2LEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(69.881, 80.617)), module, Stargazer::LFO2LEDGREEN_LIGHT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.456, 105.787)), module, Stargazer::LFO3LEDRED_LIGHT));
		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(121.456, 105.787)), module, Stargazer::LFO3LEDGREEN_LIGHT));
	}
};


Model* modelStargazer = createModel<Stargazer, StargazerWidget>("Stargazer");