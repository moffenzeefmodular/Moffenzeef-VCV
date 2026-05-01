#include "plugin.hpp"
#include <cstring>
#ifdef METAMODULE
#include "wav/wav_file_stream.hh"
#include "threads/async_thread.hh"
#include "dsp/stream_resampler.hh"
#endif

// stb_vorbis OGG decoder (compiled as C in stb_vorbis.c)
#ifndef METAMODULE
extern "C" {
    int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);
}
#endif

// Base64 encode/decode for saving audio buffer content in patch JSON
static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string b64Encode(const void* data, size_t len) {
    const uint8_t* b = (const uint8_t*)data;
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = (uint32_t)b[i] << 16;
        if (i + 1 < len) v |= (uint32_t)b[i+1] << 8;
        if (i + 2 < len) v |= (uint32_t)b[i+2];
        out += b64chars[(v >> 18) & 0x3f];
        out += b64chars[(v >> 12) & 0x3f];
        out += (i + 1 < len) ? b64chars[(v >> 6) & 0x3f] : '=';
        out += (i + 2 < len) ? b64chars[(v >> 0) & 0x3f] : '=';
    }
    return out;
}
static std::vector<uint8_t> b64Decode(const char* s, size_t slen) {
    static const int8_t T[128] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    };
    std::vector<uint8_t> out;
    out.reserve(slen * 3 / 4);
    for (size_t i = 0; i + 3 < slen; i += 4) {
        uint8_t a = (uint8_t)s[i], b = (uint8_t)s[i+1], c = (uint8_t)s[i+2], d = (uint8_t)s[i+3];
        if (a >= 128 || b >= 128) break;
        uint32_t v = ((uint32_t)T[a] << 18) | ((uint32_t)T[b] << 12)
                   | ((c != '=' && c < 128) ? (uint32_t)T[c] << 6 : 0)
                   | ((d != '=' && d < 128) ? (uint32_t)T[d]       : 0);
        out.push_back((v >> 16) & 0xff);
        if (c != '=') out.push_back((v >> 8) & 0xff);
        if (d != '=') out.push_back(v & 0xff);
    }
    return out;
}

struct ToneParamQuantity : ParamQuantity {
    std::string getDisplayValueString() override {
        float t  = clamp(getValue(), 0.f, 1.f);
        float fc = 100.f * std::pow(200.f, std::sqrt(1.f - t));
        if (fc >= 1000.f)
            return string::f("%.2f kHz", fc / 1000.f);
        return string::f("%.0f Hz", fc);
    }
};

struct Tehom : Module {
    enum ParamId {
        WARBLE_PARAM,
        FILTER_PARAM,
        AMOUNT_PARAM,
        SLEW_PARAM,
        SIZE_PARAM,
        POSITION_PARAM,
        XFADE_PARAM,

        // Source
        SOURCE1_PARAM,
        SOURCE2_PARAM,
        SOURCE3_PARAM,
        SOURCE4_PARAM,

        // Speed
        SPEED1_PARAM,
        SPEED2_PARAM,
        SPEED3_PARAM,
        SPEED4_PARAM,

        // Record
        RECORD1_PARAM,
        RECORD2_PARAM,
        RECORD3_PARAM,
        RECORD4_PARAM,

        // Loop
        LOOP1_PARAM,
        LOOP2_PARAM,
        LOOP3_PARAM,
        LOOP4_PARAM,

        // Play
        PLAY1_PARAM,
        PLAY2_PARAM,
        PLAY3_PARAM,
        PLAY4_PARAM,

		// LED Bezels
		LEDBEZEL1_PARAM,
		LEDBEZEL2_PARAM,
		LEDBEZEL3_PARAM,
		LEDBEZEL4_PARAM,

		// XY
		XPOS_PARAM,
        YPOS_PARAM,

        PARAMS_LEN
    };

    enum InputId {
        AUDIOLEFTIN_INPUT,
        AUDIORIGHTIN_INPUT,
        WARBLECVIN_INPUT,
        FILTERCVIN_INPUT,
        AMOUNTCVIN_INPUT,
        RETURN_INPUT,
        XCVIN_INPUT,
        YCVIN_INPUT,
        SLEWCVIN_INPUT,
        SIZECVIN_INPUT,
        POSITIONCVIN_INPUT,
        XFADECVIN_INPUT,

        // Source CV
        SOURCE1CVIN_INPUT,
        SOURCE2CVIN_INPUT,
        SOURCE3CVIN_INPUT,
        SOURCE4CVIN_INPUT,

        // Speed CV
        SPEED1CVIN_INPUT,
        SPEED2CVIN_INPUT,
        SPEED3CVIN_INPUT,
        SPEED4CVIN_INPUT,

        // Record CV
        RECORD1CVIN_INPUT,
        RECORD2CVIN_INPUT,
        RECORD3CVIN_INPUT,
        RECORD4CVIN_INPUT,

        // Play CV
        PLAY1CVIN_INPUT,
        PLAY2CVIN_INPUT,
        PLAY3CVIN_INPUT,
        PLAY4CVIN_INPUT,

        INPUTS_LEN
    };

    enum OutputId {
        AUDIOLEFTOUT_OUTPUT,
        AUDIORIGHTOUT_OUTPUT,
        SEND_OUTPUT,
        XCVOUT_OUTPUT,
        YCVOUT_OUTPUT,
        OUTPUTS_LEN
    };

  enum LightId {
    BUFFER1LED_LIGHT,
    BUFFER2LED_LIGHT,
    BUFFER3LED_LIGHT,
    BUFFER4LED_LIGHT,
    
    // RECORD lights (red)
    RECORD1_LIGHT,
    RECORD2_LIGHT,
    RECORD3_LIGHT,
    RECORD4_LIGHT,
    
    // PLAY lights (green)
    PLAY1_LIGHT,
    PLAY2_LIGHT,
    PLAY3_LIGHT,
    PLAY4_LIGHT,

    // PLAY lights (blue - reverse mode)
    PLAY1_BLUE_LIGHT,
    PLAY2_BLUE_LIGHT,
    PLAY3_BLUE_LIGHT,
    PLAY4_BLUE_LIGHT,

    // Erase flash (white, overlaid on record buttons)
    RECORD1_FLASH_LIGHT,
    RECORD2_FLASH_LIGHT,
    RECORD3_FLASH_LIGHT,
    RECORD4_FLASH_LIGHT,

    LIGHTS_LEN
};

	Tehom() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

	configParam(XPOS_PARAM, 0.f, 1.f, 0.5f, "X Position");
    configParam(YPOS_PARAM, 0.f, 1.f, 0.5f, "Y Position");

    configParam<ToneParamQuantity>(FILTER_PARAM, 0.f, 1.f, 0.f, "Tone");

	configParam(SLEW_PARAM, 0.f, 1.f, 0.02f, "Slew", "ms", 0.f, 1000.f);

	configParam(LEDBEZEL1_PARAM, 0.f, 1.f, 0.f);
	configParam(LEDBEZEL2_PARAM, 0.f, 1.f, 0.f);
	configParam(LEDBEZEL3_PARAM, 0.f, 1.f, 0.f);
	configParam(LEDBEZEL4_PARAM, 0.f, 1.f, 0.f);
	// LEDBezel spin position should not change on Randomize or Init
	for (int i = 0; i < 4; i++)
		paramQuantities[LEDBEZEL1_PARAM + i]->randomizeEnabled = false;

	for (int i = 0; i < 4; i++) bezelDragging[i].store(false);
	for (int i = 0; i < 4; i++) { scrubInput[i].store(0.f); scrubFresh[i].store(false); }
	for (int i = 0; i < 4; i++) {
		recordState[i].store(false);   playState[i].store(false);
		playReversed[i].store(false);  eraseFlash[i].store(0.f);
		pendingReverseFlip[i].store(false); pendingErase[i].store(false);
		autoPlay[i].store(true);
		continuousRecord[i].store(false); recordMainOutput[i].store(false);
		playCVMode[i].store(0);
	}

	configParam(WARBLE_PARAM, 0.f, 1.f, 0.f, "Warble", "%", 0.f, 100.f);
	configParam(AMOUNT_PARAM, 0.f, 1.f, 0.f, "Noise Amount", "%", 0.f, 100.f);
    configParam(SIZE_PARAM, 0.f, 1.f, 1.f, "Loop Size", "%", 0.f, 100.f);
    configParam(POSITION_PARAM, 0.f, 1.f, 0.f, "Loop Position", "%", 0.f, 100.f);
    configParam(XFADE_PARAM, 0.f, 1.f, 0.f, "Crossfade", "%", 0.f, 100.f);
	
	// Source params
	configParam(SOURCE1_PARAM, -1.f, 1.f, 0.f, "Source", "%", 0.f, 100.f);
	configParam(SOURCE2_PARAM, -1.f, 1.f, 0.f, "Source", "%", 0.f, 100.f);
	configParam(SOURCE3_PARAM, -1.f, 1.f, 0.f, "Source", "%", 0.f, 100.f);
	configParam(SOURCE4_PARAM, -1.f, 1.f, 0.f, "Source", "%", 0.f, 100.f);

	// Pitch params
	configParam(SPEED1_PARAM, 0.f, 1.f, 0.5f, "Speed", "x", 0.f, 2.f);
	configParam(SPEED2_PARAM, 0.f, 1.f, 0.5f, "Speed", "x", 0.f, 2.f);
	configParam(SPEED3_PARAM, 0.f, 1.f, 0.5f, "Speed", "x", 0.f, 2.f);
	configParam(SPEED4_PARAM, 0.f, 1.f, 0.5f, "Speed", "x", 0.f, 2.f);

	// Record switches
	configSwitch(RECORD1_PARAM, 0.f, 1.f, 0.f, "Record");
	configSwitch(RECORD2_PARAM, 0.f, 1.f, 0.f, "Record");
	configSwitch(RECORD3_PARAM, 0.f, 1.f, 0.f, "Record");
	configSwitch(RECORD4_PARAM, 0.f, 1.f, 0.f, "Record");

	// Loop switches
	configSwitch(LOOP1_PARAM, 0.f, 1.f, 1.f, "Looping", {"Off", "On"});
	configSwitch(LOOP2_PARAM, 0.f, 1.f, 1.f, "Looping", {"Off", "On"});
	configSwitch(LOOP3_PARAM, 0.f, 1.f, 1.f, "Looping", {"Off", "On"});
	configSwitch(LOOP4_PARAM, 0.f, 1.f, 1.f, "Looping", {"Off", "On"});

	// Play switches
	configSwitch(PLAY1_PARAM, 0.f, 1.f, 0.f, "Play");
	configSwitch(PLAY2_PARAM, 0.f, 1.f, 0.f, "Play");
	configSwitch(PLAY3_PARAM, 0.f, 1.f, 0.f, "Play");
	configSwitch(PLAY4_PARAM, 0.f, 1.f, 0.f, "Play");

    // Audio inputs
    configInput(AUDIOLEFTIN_INPUT, "Audio Left");
    configInput(AUDIORIGHTIN_INPUT, "Audio Right");

    // Global CV inputs
    configInput(WARBLECVIN_INPUT, "Warble CV");
    configInput(FILTERCVIN_INPUT, "Tone CV");
    configInput(AMOUNTCVIN_INPUT, "Noise Amount CV");
    configInput(RETURN_INPUT, "Noise Return");
    configInput(XCVIN_INPUT, "X CV");
    configInput(YCVIN_INPUT, "Y CV");
    configInput(SLEWCVIN_INPUT, "Slew CV");
    configInput(SIZECVIN_INPUT, "Size CV");
    configInput(POSITIONCVIN_INPUT, "Position CV");
    configInput(XFADECVIN_INPUT, "Crossfade CV");
	
	// Source CV inputs
	configInput(SOURCE1CVIN_INPUT, "Source CV");
	configInput(SOURCE2CVIN_INPUT, "Source CV");
	configInput(SOURCE3CVIN_INPUT, "Source CV");
	configInput(SOURCE4CVIN_INPUT, "Source CV");

	// Pitch CV inputs
	configInput(SPEED1CVIN_INPUT, "Speed CV");
	configInput(SPEED2CVIN_INPUT, "Speed CV");
	configInput(SPEED3CVIN_INPUT, "Speed CV");
	configInput(SPEED4CVIN_INPUT, "Speed CV");

	// Record CV inputs
	configInput(RECORD1CVIN_INPUT, "Record CV");
	configInput(RECORD2CVIN_INPUT, "Record CV");
	configInput(RECORD3CVIN_INPUT, "Record CV");
	configInput(RECORD4CVIN_INPUT, "Record CV");

	// Play CV inputs
	configInput(PLAY1CVIN_INPUT, "Play CV");
	configInput(PLAY2CVIN_INPUT, "Play CV");
	configInput(PLAY3CVIN_INPUT, "Play CV");
	configInput(PLAY4CVIN_INPUT, "Play CV");

    // Outputs
    configOutput(AUDIOLEFTOUT_OUTPUT, "Audio Left");
    configOutput(AUDIORIGHTOUT_OUTPUT, "Audio Right");
    configOutput(SEND_OUTPUT, "Noise Send");
    configOutput(XCVOUT_OUTPUT, "X CV");
    configOutput(YCVOUT_OUTPUT, "Y CV");

    loadOggFiles();
    resizeBuffers(44100.f);  // Ensure buffers are allocated before first process() to avoid audio-thread malloc

#ifdef METAMODULE
    wavThread.start([this]() {
        // Load/change file when process() requests it
        int toLoad = wavLoadIdx.load(std::memory_order_acquire);
        if (toLoad >= 0) {
            wavLoadIdx.store(-1, std::memory_order_relaxed);
            wavStream.unload();
            const char* fname = oggFilenames[toLoad];
            const char* base  = strrchr(fname, '/');
            base = base ? base + 1 : fname;
            std::string name(base);
            auto dot = name.rfind('.');
            if (dot != std::string::npos) name = name.substr(0, dot);
            wavStream.load(std::string("sdc:/") + name + ".wav");
            wavResampler.set_sample_rate_in_out(wavStream.wav_sample_rate(), currentSampleRate);
            wavResampler.set_num_channels(wavStream.num_channels());
            wavResampler.flush();
        }
        // Seek to start when audio thread signals EOF (looping)
        if (wavSeekNeeded.load(std::memory_order_acquire)) {
            wavSeekNeeded.store(false, std::memory_order_relaxed);
            wavStream.seek_frame_in_file(0);
        }
        // Keep buffer filled
        if (wavStream.is_loaded() && !wavStream.is_eof() && wavStream.frames_available() < 8192)
            wavStream.read_frames_from_file();
    });
#endif
}

void eraseBuffer(int i) {
    if (i < 0 || i > 3) return;
    if (!bufL[i].empty()) std::fill(bufL[i].begin(), bufL[i].end(), 0.f);
    if (!bufR[i].empty()) std::fill(bufR[i].begin(), bufR[i].end(), 0.f);
    writePos[i] = 0;
    readPos[i] = 0.f;
    recordedLength[i] = 0;
    hasContent[i] = false;
    recordState[i].store(false);
    playState[i].store(false);
    playGain[i]  = 0.f;
    recGain[i]   = 0.f;
    pendingReverseFlip[i].store(false);
    pendingScrubTransition[i]  = false;
    pendingPlayStart[i]        = false;
    eraseFlash[i].store(0.4f);
}

void resizeBuffers(float sampleRate) {
    currentSampleRate = sampleRate;
    bufferSize = (int)(bufferDuration * sampleRate);
    for (int i = 0; i < 4; i++) {
        bufL[i].assign(bufferSize, 0.f);
        bufR[i].assign(bufferSize, 0.f);
        writePos[i] = 0;
        readPos[i] = 0.f;
        recordedLength[i] = 0;
        hasContent[i] = false;
        recordState[i].store(false);
        playState[i].store(false);
    }
}

void onSampleRateChange(const SampleRateChangeEvent& e) override {
    resizeBuffers(e.sampleRate);
#ifdef METAMODULE
    if (wavStream.is_loaded())
        wavResampler.set_sample_rate_in_out(wavStream.wav_sample_rate(), e.sampleRate);
#endif
    // Apply any buffer data deferred from dataFromJson (runs before sample rate is known)
    for (int i = 0; i < 4; i++) {
        if (!pendingBuf[i].pending) continue;
        // Always restore playback/record states — resizeBuffers() clears them.
        recordState[i].store(pendingBuf[i].recordState);
        playState[i].store(pendingBuf[i].playState);
        playReversed[i].store(pendingBuf[i].playReversed);
        // Restore buffer data only when it was actually persisted.
        int len = std::min(pendingBuf[i].len, bufferSize);
        if (len > 0 && (int)pendingBuf[i].L.size() >= len && (int)pendingBuf[i].R.size() >= len) {
            std::copy(pendingBuf[i].L.begin(), pendingBuf[i].L.begin() + len, bufL[i].begin());
            std::copy(pendingBuf[i].R.begin(), pendingBuf[i].R.begin() + len, bufR[i].begin());
            recordedLength[i] = len;
            hasContent[i]     = pendingBuf[i].hasContent;
            readPos[i]        = clamp(pendingBuf[i].readPos, 0.f, (float)(len - 1));
        }
        pendingBuf[i] = PendingBuf{};  // clear
    }
}

void onReset(const ResetEvent& e) override {
    // Preserve bezel spin positions — they should not reset on Init
    float bezelPos[4];
    for (int i = 0; i < 4; i++)
        bezelPos[i] = params[LEDBEZEL1_PARAM + i].getValue();
    Module::onReset(e);
    for (int i = 0; i < 4; i++)
        params[LEDBEZEL1_PARAM + i].setValue(bezelPos[i]);
    bufferDuration = 2.f;
    if (currentSampleRate > 0.f) resizeBuffers(currentSampleRate);
    for (int i = 0; i < 4; i++) {
        eraseBuffer(i);
        autoPlay[i].store(true);
        playCVMode[i].store(0);
        playReversed[i].store(false);
        continuousRecord[i].store(false);
        recordMainOutput[i].store(false);
    }
    xyFinalX.store(0.5f);
    xyFinalY.store(0.5f);
    xyResetPending.store(true);
    showCrosshairs    = false;
    persist           = true;
    xyPadPansAudio.store(false);
    wanderMode.store(0);
    wanderTimer.store(0.f);
    wanderSpeed.store(0.15f);
    wanderAngle.store(0.f);
    mediaTypeIndex.store(0);
    filterStateL      = 0.f;
    filterStateR      = 0.f;
    oggPlayPos        = 0.f;
    currentOggIdx     = -1;
}

json_t* dataToJson() override {
    json_t* root = json_object();

    // Settings
    json_object_set_new(root, "bufferDuration",   json_real(bufferDuration));
    json_object_set_new(root, "showCrosshairs",   json_boolean(showCrosshairs));
    json_object_set_new(root, "persist",          json_boolean(persist));
    json_object_set_new(root, "persistBuffers",   json_boolean(persistBuffers));
    json_object_set_new(root, "mediaTypeIndex",   json_integer(mediaTypeIndex.load()));
    json_object_set_new(root, "xyPadPansAudio",   json_boolean(xyPadPansAudio.load()));
    json_object_set_new(root, "cursorStyle",      json_integer(cursorStyle));
    json_object_set_new(root, "wanderMode",       json_integer(wanderMode.load()));

    // Per-channel toggles
    json_t* ap = json_array(), *pcvm = json_array(), *cr = json_array(), *rmo = json_array();
    json_t* rs = json_array(), *pst = json_array(), *prev = json_array();
    for (int i = 0; i < 4; i++) {
        json_array_append_new(ap,   json_boolean(autoPlay[i].load()));
        json_array_append_new(pcvm, json_integer(playCVMode[i].load()));
        json_array_append_new(cr,   json_boolean(continuousRecord[i].load()));
        json_array_append_new(rmo,  json_boolean(recordMainOutput[i].load()));
        json_array_append_new(rs,   json_boolean(recordState[i].load()));
        json_array_append_new(pst,  json_boolean(playState[i].load()));
        json_array_append_new(prev, json_boolean(playReversed[i].load()));
    }
    json_object_set_new(root, "autoPlay",           ap);
    json_object_set_new(root, "playCVMode",         pcvm);
    json_object_set_new(root, "continuousRecord",   cr);
    json_object_set_new(root, "recordMainOutput",   rmo);
    json_object_set_new(root, "recordState",        rs);
    json_object_set_new(root, "playState",          pst);
    json_object_set_new(root, "playReversed",       prev);

    // Audio buffers
    json_t* bufs = json_array();
    for (int i = 0; i < 4; i++) {
        json_t* ch = json_object();
        json_object_set_new(ch, "hasContent",     json_boolean(hasContent[i]));
        json_object_set_new(ch, "recordedLength", json_integer(recordedLength[i]));
        json_object_set_new(ch, "readPos",        json_real(readPos[i]));
        if (persistBuffers && hasContent[i] && recordedLength[i] > 0) {
            int len = recordedLength[i];
            json_object_set_new(ch, "bufL", json_string(b64Encode(bufL[i].data(), len * sizeof(float)).c_str()));
            json_object_set_new(ch, "bufR", json_string(b64Encode(bufR[i].data(), len * sizeof(float)).c_str()));
        }
        json_array_append_new(bufs, ch);
    }
    json_object_set_new(root, "buffers", bufs);

    return root;
}

void dataFromJson(json_t* root) override {
    // Settings
    json_t* bd = json_object_get(root, "bufferDuration");
    if (bd) { bufferDuration = (float)json_real_value(bd); if (currentSampleRate > 0.f) resizeBuffers(currentSampleRate); }
    json_t* sc = json_object_get(root, "showCrosshairs");
    if (sc) showCrosshairs = json_boolean_value(sc);
    json_t* ps = json_object_get(root, "persist");
    if (ps) persist = json_boolean_value(ps);
    json_t* pb = json_object_get(root, "persistBuffers");
    if (pb) persistBuffers = json_boolean_value(pb);
    json_t* xyp = json_object_get(root, "xyPadPansAudio");
    if (xyp) xyPadPansAudio.store(json_boolean_value(xyp));
    json_t* cs = json_object_get(root, "cursorStyle");
    if (cs) cursorStyle = (int)json_integer_value(cs);
    json_t* wm = json_object_get(root, "wanderMode");
    if (wm) wanderMode.store((int)json_integer_value(wm));
    json_t* mti = json_object_get(root, "mediaTypeIndex");
    if (mti) mediaTypeIndex.store(clamp((int)json_integer_value(mti), 0, 7));

    // Per-channel toggles
    json_t* ap = json_object_get(root, "autoPlay");
    if (ap)   for (int i = 0; i < 4; i++) { json_t* v = json_array_get(ap,   i); if (v) autoPlay[i].store(json_boolean_value(v)); }
    json_t* pcvm = json_object_get(root, "playCVMode");
    if (pcvm) for (int i = 0; i < 4; i++) { json_t* v = json_array_get(pcvm, i); if (v) playCVMode[i].store((int)json_integer_value(v)); }
    json_t* crj = json_object_get(root, "continuousRecord");
    if (crj)  for (int i = 0; i < 4; i++) { json_t* v = json_array_get(crj,  i); if (v) continuousRecord[i].store(json_boolean_value(v)); }
    json_t* rmoj = json_object_get(root, "recordMainOutput");
    if (rmoj) for (int i = 0; i < 4; i++) { json_t* v = json_array_get(rmoj, i); if (v) recordMainOutput[i].store(json_boolean_value(v)); }
    // Read per-channel runtime states into pendingBuf so they survive the resizeBuffers()
    // call that onSampleRateChange issues after dataFromJson. Writing directly to the atomics
    // here would be wiped by resizeBuffers() before process() ever sees them.
    json_t* rsj   = json_object_get(root, "recordState");
    json_t* pstj  = json_object_get(root, "playState");
    json_t* prevj = json_object_get(root, "playReversed");
    for (int i = 0; i < 4; i++) {
        if (rsj)   { json_t* v = json_array_get(rsj,   i); if (v) pendingBuf[i].recordState  = json_boolean_value(v); }
        if (pstj)  { json_t* v = json_array_get(pstj,  i); if (v) pendingBuf[i].playState    = json_boolean_value(v); }
        if (prevj) { json_t* v = json_array_get(prevj, i); if (v) pendingBuf[i].playReversed = json_boolean_value(v); }
        pendingBuf[i].pending = true;  // always apply after resizeBuffers()
    }

    // Audio buffers
    json_t* bufs = json_object_get(root, "buffers");
    if (bufs) {
        for (int i = 0; i < 4; i++) {
            json_t* ch = json_array_get(bufs, i);
            if (!ch) continue;
            json_t* v;
            bool hc  = false; v = json_object_get(ch, "hasContent");     if (v) hc = json_boolean_value(v);
            int  len = 0;     v = json_object_get(ch, "recordedLength"); if (v) len = (int)json_integer_value(v);
            float rp = 0.f;  v = json_object_get(ch, "readPos");        if (v) rp  = (float)json_real_value(v);

            json_t* jbl = json_object_get(ch, "bufL");
            json_t* jbr = json_object_get(ch, "bufR");

            if (!hc || len <= 0 || !jbl || !jbr) continue;

            auto decL = b64Decode(json_string_value(jbl), strlen(json_string_value(jbl)));
            auto decR = b64Decode(json_string_value(jbr), strlen(json_string_value(jbr)));
            int floatBytes = len * (int)sizeof(float);

            if ((int)decL.size() < floatBytes || (int)decR.size() < floatBytes) continue;

            std::vector<float> fL(len), fR(len);
            memcpy(fL.data(), decL.data(), floatBytes);
            memcpy(fR.data(), decR.data(), floatBytes);

            // Buffer data deferred: onSampleRateChange fires after dataFromJson and would
            // wipe a direct copy via resizeBuffers. Apply in onSampleRateChange instead.
            pendingBuf[i].L          = std::move(fL);
            pendingBuf[i].R          = std::move(fR);
            pendingBuf[i].len        = len;
            pendingBuf[i].hasContent = hc;
            pendingBuf[i].readPos    = rp;
            pendingBuf[i].pending      = true;
        }
    }
}



// Current state of record/play toggles — written by audio thread, read by UI widgets
std::atomic<bool> recordState[4];
std::atomic<bool> playState[4];

// Previous button value for rising-edge detection
bool lastRecordButton[4] = {false, false, false, false};
bool lastPlayButton[4] = {false, false, false, false};

// Previous CV state for rising-edge detection
bool lastRecordCV[4] = {false, false, false, false};
bool lastPlayCV[4] = {false, false, false, false};

float ledSpinSpeed[4] = {0.3f, 0.25f, 0.2f, 0.15f};

std::atomic<bool>  bezelDragging[4];
std::atomic<float> scrubInput[4];        // UI writes combined drag delta each event
std::atomic<bool>  scrubFresh[4];        // UI sets true each drag event; audio clears
float scrubTarget[4]         = {};       // audio-only: target velocity, held between UI events
float scrubVelocity[4]       = {};       // audio-only: slewed toward scrubTarget each sample
bool  prevBezelDragging[4]   = {};       // audio-only: previous drag state for transition detection
bool  wasPlayingOnScrubStart[4] = {};    // audio-only: playState at drag start, restored on release
std::atomic<bool> playReversed[4];

std::atomic<float> eraseFlash[4];
float playGain[4]  = {};  // per-channel de-click ramp — audio-thread only
float recGain[4]   = {};  // per-channel record crossfade — ramps 0→1 on record start, 1→0 on stop
std::atomic<bool> pendingReverseFlip[4];  // written by UI button, consumed by process()
bool pendingScrubTransition[4]   = {};    // audio-thread only
bool pendingPlayStart[4]         = {};    // audio-thread only: defers readPos reset until playGain == 0
std::atomic<bool> autoPlay[4];
std::atomic<bool> continuousRecord[4];
std::atomic<bool> recordMainOutput[4];
std::atomic<bool> pendingErase[4];  // UI sets true; process() performs the erase

// XY Pad display settings (UI state, reset on Init)
bool showCrosshairs  = false;
bool persist         = true;   // cursor trails
bool persistBuffers  = false;  // save audio buffer data with patch (can stall UI on large buffers)
std::atomic<bool> xyPadPansAudio{false};
int  cursorStyle     = 0; // 0=Fish, 1=Circle

std::atomic<int> playCVMode[4]; // 0=Play/Stop, 1=Retrigger, 2=Forward/Reverse

std::atomic<bool> xyDragging{false};
std::atomic<float> xyFinalX{0.5f};
std::atomic<float> xyFinalY{0.5f};
std::atomic<bool> xyResetPending{false};

// Wander animation — written by both process() and UI menu; must be atomic
std::atomic<int>   wanderMode{0};      // 0=Off, 1=Slow, 2=Medium, 3=Fast
std::atomic<float> wanderTargetX{0.5f};
std::atomic<float> wanderTargetY{0.5f};
std::atomic<float> wanderTimer{0.f};    // seconds until next target
std::atomic<float> wanderSpeed{0.15f};  // current swim speed (units/sec), slowly varies
std::atomic<float> wanderAngle{0.f};    // current heading in radians
int wanderPhase{0};                     // counts samples; wander runs every WANDER_SUBDIV samples

// Warble LFO phases (per channel, advance while playing)
float wowPhase[4]     = {};
float flutterPhase[4] = {};

// Cached per-session constants — recomputed only when sample rate changes
float cachedSampleTime  = 0.f;
float cachedScrubDecay  = 0.f;
float cachedScrubSlew   = 0.f;
float cachedRampRate    = 0.f;
float cachedFadeLen     = 220.f;  // 5 ms at 44100 Hz — for write-head proximity fade

// Cached filter coefficients — recomputed only when the filter param changes
float cachedFilterT     = -1.f;  // -1 forces first computation
float cachedFilterAlpha =  0.f;
float cachedFilterSc    =  5.f;
float cachedFilterGain  =  1.f;

// Audio buffers
float bufferDuration = 2.f;   // seconds, user-selectable
float currentSampleRate = 44100.f;
int bufferSize = 0;
std::vector<float> bufL[4];
std::vector<float> bufR[4];
int writePos[4] = {};
double readPos[4] = {};
int recordedLength[4] = {};
bool hasContent[4] = {};

// Deferred buffer data loaded from JSON — applied once sample rate is known
struct PendingBuf {
    std::vector<float> L, R;
    int len = 0;
    bool hasContent = false;
    bool recordState = false;
    bool playState = false;
    bool playReversed = false;
    float readPos = 0.f;
    bool pending = false;
} pendingBuf[4];

// OGG media loops
struct OggLoop {
    std::vector<float> L, R;
    int numSamples = 0;
    int nativeSampleRate = 44100;
    bool loaded = false;
} oggLoops[8];

float    oggPlayPos    = 0.f;
int      currentOggIdx = -1;
std::atomic<int>  mediaTypeIndex{0};

#ifdef METAMODULE
MetaModule::WavFileStream  wavStream{256 * 1024};
MetaModule::StreamResampler wavResampler{2};
std::atomic<int>  wavLoadIdx{-2};      // -2=needs init, 0-7=load this file, -1=loaded/idle
std::atomic<bool> wavSeekNeeded{false};
MetaModule::AsyncThread    wavThread{this};
#endif

// One-pole lowpass filter state (applied to main audio before noise mix)
float    filterStateL  = 0.f;
float    filterStateR  = 0.f;

// Per-channel correction filter states for Record Main Output (avoids pitch compounding)
float    corrFilterStateL[4] = {};
float    corrFilterStateR[4] = {};

const char* const oggFilenames[8] = {
    "res/audio/MicPre.ogg",
    "res/audio/ReelToReel.ogg",
    "res/audio/Cassette.ogg",
    "res/audio/VHS.ogg",
    "res/audio/VinylClean.ogg",
    "res/audio/VinylDirty.ogg",
    "res/audio/Film8mm.ogg",
    "res/audio/Film16mm.ogg",
};

void loadOggFiles() {
#ifndef METAMODULE
    for (int i = 0; i < 8; i++) {
        std::string path = asset::plugin(pluginInstance, oggFilenames[i]);
        int ch = 0, sr = 0;
        short* data = nullptr;
        int n = stb_vorbis_decode_filename(path.c_str(), &ch, &sr, &data);
        if (n <= 0 || !data) continue;
        oggLoops[i].L.resize(n);
        oggLoops[i].R.resize(n);
        oggLoops[i].nativeSampleRate = sr;
        oggLoops[i].numSamples = n;
        for (int s = 0; s < n; s++) {
            oggLoops[i].L[s] = data[s * ch + 0] / 32768.f;
            oggLoops[i].R[s] = (ch >= 2) ? data[s * ch + 1] / 32768.f : oggLoops[i].L[s];
        }
        free(data);
        oggLoops[i].loaded = true;
    }
#endif
}

void processBypass(const ProcessArgs& args) override {
    float inL = inputs[AUDIOLEFTIN_INPUT].getVoltage();
    float inR = inputs[AUDIORIGHTIN_INPUT].isConnected() ? inputs[AUDIORIGHTIN_INPUT].getVoltage() : inL;
    outputs[AUDIOLEFTOUT_OUTPUT].setVoltage(inL);
    outputs[AUDIORIGHTOUT_OUTPUT].setVoltage(inR);
}

void process(const ProcessArgs& args) override {
    constexpr auto rlx = std::memory_order_relaxed;

    // Load shared XY state once at the top to avoid repeated atomic loads and
    // cache-line traffic between the audio thread and the UI thread.
    float xyX = xyFinalX.load(rlx);
    float xyY = xyFinalY.load(rlx);

    // Session-constant coefficients — recomputed only when sample rate changes.
    // std::exp is expensive; caching avoids calling it every sample.
    if (args.sampleTime != cachedSampleTime) {
        cachedSampleTime = args.sampleTime;
        cachedScrubDecay = std::exp(-args.sampleTime / 1.f);
        cachedScrubSlew  = 1.f - cachedScrubDecay;
        cachedRampRate    = args.sampleTime / 0.005f;
        cachedFadeLen     = args.sampleRate * 0.005f;  // 5 ms in samples — for write-head proximity fade
        cachedFilterT    = -1.f;  // force filter coefficient recomputation at new sample rate
    }
    const float scrubDecay = cachedScrubDecay;
    const float scrubSlew  = cachedScrubSlew;
    const float rampRate   = cachedRampRate;

    // Deferred buffer erases requested from the UI thread (right-click on record button).
    // Performed here on the audio thread to avoid races with the audio buffers.
    for (int i = 0; i < 4; i++) {
        if (pendingErase[i].exchange(false, rlx))
            eraseBuffer(i);
    }

    // --- RECORD toggles ---
    for (int i = 0; i < 4; i++) {
        bool btn = params[RECORD1_PARAM + i].getValue() > 0.5f;
        bool btnRising = btn && !lastRecordButton[i];
        lastRecordButton[i] = btn;

        bool cv = inputs[RECORD1CVIN_INPUT + i].getVoltage() > 1.f;
        bool cvRising = cv && !lastRecordCV[i];
        lastRecordCV[i] = cv;

        if (btnRising || cvRising) {
            bool wasRecording = recordState[i].load(rlx);
            recordState[i].store(!wasRecording, rlx);
            if (!wasRecording && !hasContent[i]) {
                writePos[i] = 0;
                recordedLength[i] = 0;
            }
            // Auto-play when recording manually stopped — use pending system so readPos
            // is never reset while playGain is still non-zero from a prior ramp-down.
            if (wasRecording && autoPlay[i].load(rlx) && hasContent[i] && !playState[i].load(rlx)) {
                if (playGain[i] <= 0.f) {
                    playState[i].store(true, rlx);
                    readPos[i] = playReversed[i].load(rlx) ? (float)(std::max(1, recordedLength[i]) - 1) : 0.f;
                } else {
                    pendingPlayStart[i] = true;
                }
            }
        }

        lights[RECORD1_LIGHT + i].setBrightnessSmooth(recordState[i].load(rlx) ? 1.f : 0.f, args.sampleTime);

        // Erase flash — white light fades out over 0.4s
        {
            float ef = eraseFlash[i].load(rlx);
            if (ef > 0.f) {
                ef -= args.sampleTime;
                eraseFlash[i].store(ef, rlx);
                lights[RECORD1_FLASH_LIGHT + i].setBrightness(clamp(ef / 0.4f, 0.f, 1.f));
            }
            // When ef is already 0 no store or setBrightness needed — skip every sample
        }
    }

    // --- PLAY toggles ---
    for (int i = 0; i < 4; i++) {
        bool btn = params[PLAY1_PARAM + i].getValue() > 0.5f;
        bool btnRising = btn && !lastPlayButton[i];
        lastPlayButton[i] = btn;

        bool cv = inputs[PLAY1CVIN_INPUT + i].getVoltage() > 1.f;
        bool cvRising = cv && !lastPlayCV[i];
        lastPlayCV[i] = cv;

        // Helper: start playback — defers readPos reset if gain is still non-zero to avoid click.
        // Only call when intending to START (not stop) playback.
        auto startPlay = [&]() {
            if (playGain[i] <= 0.f) {
                // Gain is already silent — safe to start directly, ramp will go 0→1
                playState[i].store(true, rlx);
                readPos[i] = playReversed[i].load(rlx) ? (float)(std::max(1, recordedLength[i]) - 1) : 0.f;
            } else {
                // Gain is still non-zero from a previous ramp — defer until zero crossing
                // so the readPos jump cannot create a discontinuity in the output.
                pendingPlayStart[i] = true;
            }
        };
        auto stopPlay = [&]() {
            pendingPlayStart[i] = false;   // cancel any deferred start
            playState[i].store(false, rlx);
        };

        if (cvRising) {
            int cvMode = playCVMode[i].load(rlx);
            if (cvMode == 1) {
                // Retrigger: reset playhead to start (or end if reversed), ensure playing
                startPlay();
            } else if (cvMode == 2) {
                // Forward/Reverse toggle
                pendingReverseFlip[i].store(!pendingReverseFlip[i].load(rlx), rlx);
            } else {
                // Play/Stop toggle (default)
                if (!playState[i].load(rlx) && !pendingPlayStart[i])
                    startPlay();
                else
                    stopPlay();
            }
        }
        if (btnRising) {
            if (!playState[i].load(rlx) && !pendingPlayStart[i])
                startPlay();
            else
                stopPlay();
        }

        bool pState = playState[i].load(rlx);
        bool pRev   = playReversed[i].load(rlx);
        lights[PLAY1_LIGHT + i].setBrightnessSmooth((pState && !pRev) ? 1.f : 0.f, args.sampleTime);
        lights[PLAY1_BLUE_LIGHT + i].setBrightnessSmooth((pState && pRev) ? 1.f : 0.f, args.sampleTime);
    }

    // --- BEZEL SPINNING / VINYL SCRUB ---
    for (int i = 0; i < 4; i++) {
        bool isDragging = bezelDragging[i].load();

        // Detect drag start: remember whether we were playing, and queue a gain ramp-down-then-up
        if (!prevBezelDragging[i] && isDragging) {
            wasPlayingOnScrubStart[i] = playState[i].load(rlx);
            if (wasPlayingOnScrubStart[i])
                pendingScrubTransition[i] = true;  // ramp gain to 0, then ramp back in scrub mode
        }

        if (isDragging) {
            // Vinyl scrub: UI events update the target; between events the target decays
            // so the record slows when the hand stops. scrubVelocity slews toward the target
            // every sample for smooth pitch ramp-up/ramp-down.
            if (scrubFresh[i].exchange(false))
                scrubTarget[i] = scrubInput[i].load() * 0.25f;  // right/up = forward in buffer
            else
                scrubTarget[i] *= scrubDecay;  // 1 s decay when idle

            // Per-sample slew toward target (1 s) — smooth vinyl acceleration / deceleration
            scrubVelocity[i] += (scrubTarget[i] - scrubVelocity[i]) * scrubSlew;

            // Drive bezel visual from slewed velocity so animation matches what you hear
            {
                float bezelDelta = scrubVelocity[i] * 0.42f * args.sampleTime;
                float bv = params[LEDBEZEL1_PARAM + i].getValue() + bezelDelta;
                if (bv > 1.f) bv -= 1.f;
                if (bv < 0.f) bv += 1.f;
                params[LEDBEZEL1_PARAM + i].setValue(bv);
            }

            if (hasContent[i] && recordedLength[i] >= 2) {
                // Scrub within the current loop window so a small loop can be scrubbed around
                int len = recordedLength[i];
                float sp = clamp(params[SIZE_PARAM].getValue()
                    + clamp(inputs[SIZECVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
                float pp = clamp(params[POSITION_PARAM].getValue()
                    + clamp(inputs[POSITIONCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
                int minLoopSize = std::min(100, len);
                int loopSize  = clamp((int)(sp * sp * sp * (float)len), minLoopSize, len);
                int loopStart = clamp((int)(pp * (float)(len - loopSize)), 0, len - loopSize);
                int loopEnd   = loopStart + loopSize; (void)loopEnd;

                readPos[i] += scrubVelocity[i];
                // Wrap within loop window using fmod — O(1) regardless of velocity magnitude,
                // so a tiny loop window with a large scrubVelocity never spins the audio thread.
                double offset = std::fmod(readPos[i] - (double)loopStart, (double)loopSize);
                if (offset < 0.0) offset += (double)loopSize;
                readPos[i] = (double)loopStart + offset;
            }
        } else {
            // Detect drag release: clamp readPos into loop window and restore playState
            if (prevBezelDragging[i] && hasContent[i] && recordedLength[i] >= 2) {
                int len = recordedLength[i];
                float sp = clamp(params[SIZE_PARAM].getValue()
                    + clamp(inputs[SIZECVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
                float pp = clamp(params[POSITION_PARAM].getValue()
                    + clamp(inputs[POSITIONCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
                int minLoopSize = std::min(100, len);
                int loopSize  = clamp((int)(sp * sp * sp * (float)len), minLoopSize, len);
                int loopStart = clamp((int)(pp * (float)(len - loopSize)), 0, len - loopSize);
                int loopEnd   = loopStart + loopSize;
                if (readPos[i] < (float)loopStart) readPos[i] = (float)loopStart;
                if (readPos[i] >= (float)loopEnd)  readPos[i] = (float)(loopEnd - 1);
                if (wasPlayingOnScrubStart[i])
                    playState[i].store(true, rlx);
            }
            scrubTarget[i]   = 0.f;
            scrubVelocity[i] = 0.f;
            // Normal spin while playing
            if (playState[i].load(rlx)) {
                float knobOctaves = std::log2f(std::max(params[SPEED1_PARAM + i].getValue() * 2.f, 0.05f));
                float totalOctaves = clamp(knobOctaves + inputs[SPEED1CVIN_INPUT + i].getVoltage(), std::log2f(0.05f), 1.f);
                float pitchVal = std::pow(2.f, totalOctaves) * 0.5f;
                float spinSpeed = 0.02f + pitchVal * 0.8f;
                float spinDir = playReversed[i].load(rlx) ? -1.f : 1.f;
                float newValue = params[LEDBEZEL1_PARAM + i].getValue() + spinDir * spinSpeed * args.sampleTime;
                if (newValue > 1.f) newValue -= 1.f;
                if (newValue < 0.f) newValue += 1.f;
                params[LEDBEZEL1_PARAM + i].setValue(newValue);
            }
        }

        prevBezelDragging[i] = isDragging;
    }

    // === AUDIO DSP ===
    float inL = inputs[AUDIOLEFTIN_INPUT].getVoltage();
    float inR = inputs[AUDIORIGHTIN_INPUT].isConnected() ? inputs[AUDIORIGHTIN_INPUT].getVoltage() : inL;

    // XY mixer — 2x2 bilinear: each corner owns one channel at full volume
    // ch1=top-left, ch2=top-right, ch3=bottom-left, ch4=bottom-right
    // xyFinalX=0 → left, xyFinalX=1 → right; xyFinalY=1 → top, xyFinalY=0 → bottom
    float vol[4] = {
        (1.f - xyX) * xyY,         // ch1: top-left
        xyX * xyY,                   // ch2: top-right
        (1.f - xyX) * (1.f - xyY),  // ch3: bottom-left
        xyX * (1.f - xyY),           // ch4: bottom-right
    };

    // Warble (wow + flutter)
    float warble     = clamp(params[WARBLE_PARAM].getValue() + clamp(inputs[WARBLECVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
    float warbleFade = clamp(warble / 0.05f, 0.f, 1.f); // fade in over first 5% of knob
    float wowRate     = 0.5f + warble * 1.5f;  // 0.5–2 Hz
    float flutterRate = 5.f  + warble * 10.f;  // 5–15 Hz
    float wowDepth     = warbleFade * warble * warble * 0.042f;
    float flutterDepth = warbleFade * warble * warble * 0.011f;

    // Per-channel: read loop buffer (with pitch), source-crossfade with input for output + recording.
    // source left (t=0) = hear/record input; source right (t=1) = hear/record loop (sound on sound).
    // bilinear vol[] always sums to 1.0, so unity gain is preserved across XY positions.
    float outL = 0.f, outR = 0.f;
    float loopOutL = 0.f, loopOutR = 0.f;  // loop-only portion (filtered separately)
    float chanMixL[4] = {}, chanMixR[4] = {};
    float recMixL[4]  = {}, recMixR[4]  = {}; // recording mix: loop component read at 1:1 (no pitch shift)

    // Global loop window params
    float sizeParam  = clamp(params[SIZE_PARAM].getValue() + clamp(inputs[SIZECVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
    float posParam   = clamp(params[POSITION_PARAM].getValue() + clamp(inputs[POSITIONCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
    float xfadeParam = std::max(0.01f, clamp(params[XFADE_PARAM].getValue() + clamp(inputs[XFADECVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f));

    for (int i = 0; i < 4; i++) {
        float sampL = 0.f, sampR = 0.f;
        bool isDraggingNow = bezelDragging[i].load();

        if (hasContent[i] && bufferSize > 0 && (playState[i].load(rlx) || isDraggingNow || playGain[i] > 0.f)) {
            int len = recordedLength[i];
            if (len >= 2) {
                if (isDraggingNow) {
                    // Scrub read: use full buffer at current readPos (already updated by scrub code above)
                    int rp0 = clamp((int)readPos[i], 0, len - 1);
                    int rp1 = clamp(rp0 + 1, 0, len - 1);
                    float frac = readPos[i] - (float)rp0;
                    sampL = bufL[i][rp0] + (bufL[i][rp1] - bufL[i][rp0]) * frac;
                    sampR = bufR[i][rp0] + (bufR[i][rp1] - bufR[i][rp0]) * frac;
                    // Apply xfade envelope at loop boundaries during scrub to eliminate clicks
                    if (xfadeParam > 0.f) {
                        int minLoopSize = std::min(100, len);
                        int loopSize  = clamp((int)(sizeParam * sizeParam * sizeParam * (float)len), minLoopSize, len);
                        int loopStart = clamp((int)(posParam * (float)(len - loopSize)), 0, len - loopSize);
                        float posInLoop = readPos[i] - (float)loopStart;
                        float fadeLen   = xfadeParam * (float)(loopSize / 2);
                        float gain = 1.f;
                        if (posInLoop < fadeLen)
                            gain = posInLoop / fadeLen;
                        else if (posInLoop > (float)loopSize - fadeLen)
                            gain = ((float)loopSize - posInLoop) / fadeLen;
                        sampL *= clamp(gain, 0.f, 1.f);
                        sampR *= clamp(gain, 0.f, 1.f);
                    }
                } else {
                    // Normal playback with loop window
                    // Compute loop window (strictly within recorded content)
                    int minLoopSize = std::min(100, len);
                    int loopSize    = clamp((int)(sizeParam * sizeParam * sizeParam * (float)len), minLoopSize, len);
                    int maxStart    = len - loopSize; // always >= 0 since loopSize <= len
                    int loopStart   = clamp((int)(posParam * (float)maxStart), 0, maxStart);
                    int loopEnd     = loopStart + loopSize; // always <= len, exact

                    // Clamp readPos into window (handles init and window changes mid-playback)
                    if (readPos[i] < (float)loopStart) readPos[i] = (float)loopStart;
                    if (readPos[i] >= (float)loopEnd)  readPos[i] = (float)(loopEnd - 1);

                    int rp0 = clamp((int)readPos[i], loopStart, loopEnd - 1);
                    int rp1 = clamp(rp0 + 1, loopStart, loopEnd - 1);
                    float frac = readPos[i] - (float)rp0;
                    sampL = bufL[i][rp0] + (bufL[i][rp1] - bufL[i][rp0]) * frac;
                    sampR = bufR[i][rp0] + (bufR[i][rp1] - bufR[i][rp0]) * frac;

                    // XFade envelope: linear fade zones at loop edges, shrinking toward edges as xfade decreases.
                    // xfade=0: no fade (gain=1 everywhere); xfade=1: fade spans full half-loop each side (triangle).
                    if (xfadeParam > 0.f) {
                        float posInLoop = readPos[i] - (float)loopStart;
                        float fadeLen   = xfadeParam * (float)(loopSize / 2);
                        float gain = 1.f;
                        if (posInLoop < fadeLen)
                            gain = posInLoop / fadeLen;
                        else if (posInLoop > (float)loopSize - fadeLen)
                            gain = ((float)loopSize - posInLoop) / fadeLen;
                        sampL *= clamp(gain, 0.f, 1.f);
                        sampR *= clamp(gain, 0.f, 1.f);
                    }

                    float knobOctaves = std::log2f(std::max(params[SPEED1_PARAM + i].getValue() * 2.f, 0.05f));
                    float totalOctaves = clamp(knobOctaves + inputs[SPEED1CVIN_INPUT + i].getVoltage(), std::log2f(0.05f), 1.f);
                    float speed = std::pow(2.f, totalOctaves);

                    // Wow and flutter: only compute sin when warble is non-zero.
                    // Use conditional subtract instead of fmod — far cheaper since increments
                    // are tiny (<<1) and phase only wraps once every thousands of samples.
                    if (wowDepth > 0.f || flutterDepth > 0.f) {
                        wowPhase[i] += wowRate * args.sampleTime;
                        if (wowPhase[i] >= 1.f) wowPhase[i] -= 1.f;
                        flutterPhase[i] += flutterRate * args.sampleTime;
                        if (flutterPhase[i] >= 1.f) flutterPhase[i] -= 1.f;
                        float warbleMod = std::sin(2.f * float(M_PI) * wowPhase[i])     * wowDepth
                                        + std::sin(2.f * float(M_PI) * flutterPhase[i]) * flutterDepth;
                        speed *= (1.f + warbleMod);
                    }

                    float dir = playReversed[i].load(rlx) ? -1.f : 1.f;
                    readPos[i] += dir * speed;

                    bool looping = params[LOOP1_PARAM + i].getValue() > 0.5f;
                    if (looping) {
                        if (readPos[i] >= (float)loopEnd)   readPos[i] -= (float)loopSize;
                        if (readPos[i] < (float)loopStart)  readPos[i] += (float)loopSize;
                    } else {
                        if (readPos[i] >= (float)loopEnd || readPos[i] < (float)loopStart) {
                            playState[i].store(false, rlx);
                            // Do NOT reset readPos here — playGain is still > 0 and will ramp down over 5ms.
                            // Lines 972-973 clamp readPos to the boundary each iteration, so the fade-out
                            // reads from the loop edge rather than jumping to a discontinuous position.
                        }
                    }
                }
            }
        }

        // De-click ramp: ~2ms fade on any state transition.
        // Pending flags force gain to 0 first; the action fires at zero-crossing, then gain ramps back up.
        {
            bool hasPending = pendingReverseFlip[i].load(rlx) || pendingScrubTransition[i]
                           || pendingPlayStart[i];
            float target = hasPending ? 0.f : ((playState[i].load(rlx) || isDraggingNow) ? 1.f : 0.f);
            // clamp(delta, -r, r) moves toward target by at most rampRate per sample,
            // and stops exactly at target — no overshoot or oscillation at 0 or 1.
            playGain[i] = clamp(playGain[i] + clamp(target - playGain[i], -rampRate, rampRate), 0.f, 1.f);

            // At zero-crossing: execute deferred actions then let gain ramp back up
            if (playGain[i] <= 0.f) {
                if (pendingReverseFlip[i].load(rlx)) {
                    playReversed[i].store(!playReversed[i].load(rlx), rlx);
                    pendingReverseFlip[i].store(false, rlx);
                }
                if (pendingScrubTransition[i])
                    pendingScrubTransition[i] = false;
                if (pendingPlayStart[i]) {
                    // Safe to set readPos now — gain is guaranteed 0, so no discontinuity
                    playState[i].store(true, rlx);
                    readPos[i] = playReversed[i].load(rlx) ? (float)(std::max(1, recordedLength[i]) - 1) : 0.f;
                    pendingPlayStart[i] = false;
                }
            }
        }
        sampL *= playGain[i];
        sampR *= playGain[i];

        // Write-head proximity fade: in continuous record, the write head advances forward while
        // reversed playback retreats backward — they cross periodically. At the crossing, the
        // read head encounters freshly overwritten buffer content (written with recGain=1, no blend),
        // which can differ sharply from the old content → audible click.
        // Fix: fade to 0 within a 5ms radius of the write head so the crossing is silent.
        if (recordState[i].load(rlx) && hasContent[i] && recordedLength[i] > 0) {
            float dist = std::abs(readPos[i] - (float)writePos[i]);
            // Wrap: choose the shorter arc on the circular buffer
            dist = std::min(dist, (float)recordedLength[i] - dist);
            const float fadeLen = cachedFadeLen;
            if (dist < fadeLen) {
                float blend = dist / fadeLen;
                sampL *= blend;
                sampR *= blend;
            }
        }

        float srcParam = clamp(params[SOURCE1_PARAM + i].getValue() + clamp(inputs[SOURCE1CVIN_INPUT + i].getVoltage(), -5.f, 5.f) / 5.f, -1.f, 1.f);
        float t = (srcParam + 1.f) * 0.5f; // 0 = input, 1 = loop
        chanMixL[i] = inL * (1.f - t) + sampL * t;
        chanMixR[i] = inR * (1.f - t) + sampR * t;

        // Recording mix: blend input with raw buffer at 1:1 so pitch doesn't compound on re-record.
        // Using writePos % recordedLength maps the write head into the existing loop without pitch shift.
        {
            float rawBufL = 0.f, rawBufR = 0.f;
            if (hasContent[i] && recordedLength[i] > 0) {
                int readIdx = writePos[i] % recordedLength[i];
                rawBufL = bufL[i][readIdx];
                rawBufR = bufR[i][readIdx];
            }
            recMixL[i] = inL * (1.f - t) + rawBufL * t;
            recMixR[i] = inR * (1.f - t) + rawBufR * t;
        }

        if (xyPadPansAudio.load(rlx)) {
            // X=0 → 100% left, X=1 → 100% right, X=0.5 → center (constant-power pan law)
            float panL = std::cos(xyX * float(M_PI) * 0.5f);
            float panR = std::sin(xyX * float(M_PI) * 0.5f);
            outL += chanMixL[i] * vol[i] * panL;
            outR += chanMixR[i] * vol[i] * panR;
            loopOutL += sampL * t * vol[i] * panL;
            loopOutR += sampR * t * vol[i] * panR;
        } else {
            outL += chanMixL[i] * vol[i];
            outR += chanMixR[i] * vol[i];
            loopOutL += sampL * t * vol[i];
            loopOutR += sampR * t * vol[i];
        }

        float level = (std::abs(chanMixL[i]) + std::abs(chanMixR[i])) * 0.5f * vol[i];
        lights[BUFFER1LED_LIGHT + i].setBrightnessSmooth(level, args.sampleTime * 20.f);
    }

    outL = clamp(outL, -5.f, 5.f);
    outR = clamp(outR, -5.f, 5.f);

    // === LOWPASS FILTER then SOFT CLIP — loop portions only, input passes through unaffected ===
    // Coefficients (pow/sqrt/exp) are cached and only recomputed when t changes.
    float filterAlpha = cachedFilterAlpha; // also used in Record Main Output correction below
    {
        float t = clamp(params[FILTER_PARAM].getValue()
            + clamp(inputs[FILTERCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
        if (t != cachedFilterT) {
            cachedFilterT = t;
            // Inverted: knob down = open (20 kHz), knob up = closed (100 Hz)
            float fc = 100.f * std::pow(200.f, std::sqrt(1.f - t));
            cachedFilterAlpha = 1.f - std::exp(-2.f * float(M_PI) * fc / args.sampleRate);
            float scAmount = clamp((t - 0.4f) / 0.6f, 0.f, 1.f);
            cachedFilterSc   = 5.f - 4.5f * scAmount;
            cachedFilterGain = 1.f + t * 0.4f;
            filterAlpha = cachedFilterAlpha;
        }
        filterStateL += filterAlpha * (loopOutL - filterStateL);
        filterStateR += filterAlpha * (loopOutR - filterStateR);

        // Post-filter soft clip: knob past 40% slides sc from 5 (clean) down to 0.5 (heavy saturation)
        float processedL = std::tanh(filterStateL / cachedFilterSc) * cachedFilterSc;
        float processedR = std::tanh(filterStateR / cachedFilterSc) * cachedFilterSc;

        // Linear gain compensation: as filter closes, boost loop level slightly (up to +3dB at full)
        processedL *= cachedFilterGain;
        processedR *= cachedFilterGain;

        // Substitute: remove original loop from combined, add filtered+soft-clipped loop
        outL = outL - loopOutL + processedL;
        outR = outR - loopOutR + processedR;
    }

    // === MEDIA (OGG LOOPS) — mixed in after XY mixer ===
    int mediaIdx = mediaTypeIndex.load(rlx);

    if (mediaIdx != currentOggIdx) {
        currentOggIdx = mediaIdx;
        oggPlayPos = 0.f;
#ifdef METAMODULE
        wavLoadIdx.store(mediaIdx, std::memory_order_release);
#endif
    }

    // Compute amount first so we can gate the OGG read when nothing uses it.
    float amount = clamp(params[AMOUNT_PARAM].getValue()
        + clamp(inputs[AMOUNTCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f, 0.f, 1.f);
    float amountLog = amount * amount;  // logarithmic taper

    bool preFader      = true;
    bool sendConnected = outputs[SEND_OUTPUT].isConnected();
    bool returnConnected = inputs[RETURN_INPUT].isConnected();

    // Only read OGG when something will actually use it: pre-fader send needs raw OGG,
    // post-fader paths need it when amount > 0 and no return is replacing it.
    // This avoids ~2 cache-cold array reads per sample when the module is idle.
    float mediaL = 0.f, mediaR = 0.f;
    bool needOgg = sendConnected || (!returnConnected && amountLog > 0.f);
    if (needOgg) {
#ifdef METAMODULE
        if (wavStream.is_loaded()) {
            auto [l, r] = wavResampler.process_stereo([this]{ return wavStream.pop_sample(); });
            mediaL = l * 5.f;
            mediaR = r * 5.f;
            if (wavStream.is_eof() && wavStream.frames_available() == 0) {
                wavStream.reset_playback_to_frame(0);
                wavSeekNeeded.store(true, std::memory_order_release);
            }
        }
#else
        OggLoop& loop = oggLoops[mediaIdx];
        if (loop.loaded && loop.numSamples > 1) {
            float advance = (float)loop.nativeSampleRate / args.sampleRate;
            int i0 = (int)oggPlayPos;
            int i1 = (i0 + 1) % loop.numSamples;
            float frac = oggPlayPos - (float)i0;
            mediaL = (loop.L[i0] + (loop.L[i1] - loop.L[i0]) * frac) * 5.f;
            mediaR = (loop.R[i0] + (loop.R[i1] - loop.R[i0]) * frac) * 5.f;
            oggPlayPos += advance;
            if (oggPlayPos >= (float)loop.numSamples) oggPlayPos -= (float)loop.numSamples;
        }
#endif
    }

    // Pre-fader send: raw media before amount knob
    if (preFader)
        outputs[SEND_OUTPUT].setVoltage(mediaL * 2.5f);

    // Return replaces media (both L and R) before amount knob
    if (returnConnected) {
        float ret = inputs[RETURN_INPUT].getVoltage() * 0.4f;
        mediaL = ret;
        mediaR = ret;
    }

    float noiseMixL = mediaL * amountLog * 0.5f;
    float noiseMixR = mediaR * amountLog * 0.5f;

    // Post-fader send: after amount knob
    if (!preFader)
        outputs[SEND_OUTPUT].setVoltage((noiseMixL + noiseMixR) * 0.5f);

    outL = clamp(outL + noiseMixL, -5.f, 5.f);
    outR = clamp(outR + noiseMixR, -5.f, 5.f);

    outputs[AUDIOLEFTOUT_OUTPUT].setVoltage(outL);
    outputs[AUDIORIGHTOUT_OUTPUT].setVoltage(outR);

    for (int i = 0; i < 4; i++) {
        if (bufferSize == 0) continue;

        // recGain crossfades the buffer write: ramps 0→1 when recording starts, 1→0 when it stops.
        // Writing bufL[i][writePos] = existing * (1-recGain) + newContent * recGain means the
        // read head sees a smooth blend instead of a hard seam — eliminating sound-on-sound clicks.
        float recTarget = recordState[i].load(rlx) ? 1.f : 0.f;
        recGain[i] = clamp(recGain[i] + clamp(recTarget - recGain[i], -rampRate, rampRate), 0.f, 1.f);

        if (recGain[i] <= 0.f) continue;  // not recording and fully faded out

        // Compute new content to write
        float newL, newR;
        if (recordMainOutput[i].load(rlx)) {
            // Record final output with pitch-compounding correction:
            // filter the per-channel correction with the same coefficient as the main filter
            // so the recorded signal is truly post-filter/noise but without pitch accumulation.
            float corrL = (recMixL[i] - chanMixL[i]) * vol[i];
            float corrR = (recMixR[i] - chanMixR[i]) * vol[i];
            if (xyPadPansAudio.load(rlx)) {
                float panL = std::cos(xyX * float(M_PI) * 0.5f);
                float panR = std::sin(xyX * float(M_PI) * 0.5f);
                corrL *= panL;
                corrR *= panR;
            }
            corrFilterStateL[i] += filterAlpha * (corrL - corrFilterStateL[i]);
            corrFilterStateR[i] += filterAlpha * (corrR - corrFilterStateR[i]);
            newL = clamp(outL + corrFilterStateL[i], -5.f, 5.f);
            newR = clamp(outR + corrFilterStateR[i], -5.f, 5.f);
        } else {
            // Default: record recMixL/R — input blended with raw buffer at 1:1 (no pitch compounding)
            newL = recMixL[i];
            newR = recMixR[i];
        }

        // Crossfade write: blend existing buffer content with new content using recGain.
        // When recGain < 1 (fade-in or fade-out), existing content is preserved proportionally,
        // making the transition inaudible when the read head later passes this position.
        float rg = recGain[i];
        bufL[i][writePos[i]] = bufL[i][writePos[i]] * (1.f - rg) + newL * rg;
        bufR[i][writePos[i]] = bufR[i][writePos[i]] * (1.f - rg) + newR * rg;

        writePos[i]++;
        if (recordedLength[i] < bufferSize) recordedLength[i]++;
        hasContent[i] = true;

        if (writePos[i] >= bufferSize) {
            writePos[i] = 0;
            if (recordState[i].load(rlx)) {  // only act if actively recording, not just in fade-out
                if (continuousRecord[i].load(rlx)) {
                    // Keep recording — wrap and overwrite from the start
                } else {
                    recordState[i].store(false, rlx);
                    if (autoPlay[i].load(rlx) && !playState[i].load(rlx)) {
                        if (playGain[i] <= 0.f) {
                            playState[i].store(true, rlx);
                            readPos[i] = 0.f;
                        } else {
                            pendingPlayStart[i] = true;
                        }
                    }
                }
            }
        }
    }

    // Wander animation — fish-in-tank swim behaviour.
    // Subsampled: runs every WANDER_SUBDIV samples instead of every sample.
    // Uses dt = sampleTime * WANDER_SUBDIV for time-scaled updates; Brownian
    // drift amplitude is scaled by sqrt(WANDER_SUBDIV) for correct diffusion.
    // This saves ~3 RNG calls + sqrt + atan2 + cos + sin per 32 samples.
    constexpr int WANDER_SUBDIV = 32;
    int wMode = wanderMode.load(rlx);
    if (wMode > 0 && !xyDragging.load() && ++wanderPhase >= WANDER_SUBDIV) {
        wanderPhase = 0;
        const float dt = args.sampleTime * WANDER_SUBDIV;

        float cx = params[XPOS_PARAM].getValue();
        float cy = params[YPOS_PARAM].getValue();

        // Per-mode tuning: driftAmp (chaos), speedMin/Max (units/sec), targetInterval (sec)
        float modeDrift, speedMin, speedMax, targetInterval;
        if (wMode == 1) {        // Slow — lazy, dreamy drift
            modeDrift      = 0.5f;   speedMin = 0.04f;  speedMax = 0.10f;  targetInterval = 4.f;
        } else if (wMode == 2) { // Medium — steady swim
            modeDrift      = 1.5f;   speedMin = 0.14f;  speedMax = 0.26f;  targetInterval = 2.5f;
        } else {                      // Fast — darting, erratic
            modeDrift      = 3.5f;   speedMin = 0.28f;  speedMax = 0.50f;  targetInterval = 1.2f;
        }

        // Snapshot wander floats into locals for this audio cycle
        float wTargX = wanderTargetX.load(rlx);
        float wTargY = wanderTargetY.load(rlx);
        float wTimer = wanderTimer.load(rlx);
        float wSpeed = wanderSpeed.load(rlx);
        float wAngle = wanderAngle.load(rlx);

        // Pick a loose target area every 2–5 s, kept away from edges
        wTimer -= dt;
        if (wTimer <= 0.f) {
            wTargX = 0.15f + random::uniform() * 0.70f;
            wTargY = 0.15f + random::uniform() * 0.70f;
            wTimer = targetInterval * (0.6f + random::uniform() * 0.8f);
        }

        // Brownian angular drift — scaled by sqrt(WANDER_SUBDIV) so diffusion
        // per unit time matches the original per-sample version exactly.
        float driftAmp = modeDrift / std::sqrt(args.sampleRate) * std::sqrt((float)WANDER_SUBDIV);
        wAngle += (random::uniform() * 2.f - 1.f) * driftAmp;

        // Weak, lazy pull toward target — low gain so it never dominates the drift
        float dx = wTargX - cx;
        float dy = wTargY - cy;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.06f) {
            float targetAngle = std::atan2(dy, dx);
            float diff = targetAngle - wAngle;
            while (diff >  float(M_PI)) diff -= 2.f * float(M_PI);
            while (diff < -float(M_PI)) diff += 2.f * float(M_PI);
            wAngle += diff * 0.7f * dt;
        }

        // Speed random-walks within mode range
        wSpeed += (random::uniform() * 2.f - 1.f) * 0.15f * dt;
        wSpeed  = clamp(wSpeed, speedMin, speedMax);

        float vx = std::cos(wAngle) * wSpeed;
        float vy = std::sin(wAngle) * wSpeed;

        // Use CV-offset effective position for wall sensing so wander steers away
        // from the boundary the fish actually occupies, not just the raw param value
        float cvOffX = inputs[XCVIN_INPUT].isConnected() ? clamp(inputs[XCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f : 0.f;
        float cvOffY = inputs[YCVIN_INPUT].isConnected() ? clamp(inputs[YCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f : 0.f;
        float ex = clamp(cx + cvOffX, 0.f, 1.f);
        float ey = clamp(cy + cvOffY, 0.f, 1.f);

        // Wall repulsion based on effective position
        const float margin = 0.1f;
        if (ex < margin)         vx += (1.f - ex / margin) * 0.5f;
        if (ex > 1.f - margin)   vx -= (1.f - (1.f - ex) / margin) * 0.5f;
        if (ey < margin)         vy += (1.f - ey / margin) * 0.5f;
        if (ey > 1.f - margin)   vy -= (1.f - (1.f - ey) / margin) * 0.5f;

        // Hard reflect at the actual boundary — prevents the fish fighting the clamp
        float nx = cx + vx * dt;
        float ny = cy + vy * dt;
        if (nx <= 0.f || nx >= 1.f) vx = -vx;
        if (ny <= 0.f || ny >= 1.f) vy = -vy;

        // Sync heading to actual velocity direction
        float vLen = std::sqrt(vx * vx + vy * vy);
        if (vLen > 0.001f)
            wAngle = std::atan2(vy, vx);

        params[XPOS_PARAM].setValue(clamp(nx, 0.f, 1.f));
        params[YPOS_PARAM].setValue(clamp(ny, 0.f, 1.f));

        // Write wander state back
        wanderTargetX.store(wTargX, rlx);
        wanderTargetY.store(wTargY, rlx);
        wanderTimer.store(wTimer, rlx);
        wanderSpeed.store(wSpeed, rlx);
        wanderAngle.store(wAngle, rlx);
    }

    // XY Pad — param + CV offset (CV bypassed while dragging), then slewed
    float targetX, targetY;
    if (!xyDragging.load()) {
        float cvX = inputs[XCVIN_INPUT].isConnected() ? clamp(inputs[XCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f : 0.f;
        float cvY = inputs[YCVIN_INPUT].isConnected() ? clamp(inputs[YCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f : 0.f;
        targetX = clamp(params[XPOS_PARAM].getValue() + cvX, 0.f, 1.f);
        targetY = clamp(params[YPOS_PARAM].getValue() + cvY, 0.f, 1.f);
    } else {
        targetX = params[XPOS_PARAM].getValue();
        targetY = params[YPOS_PARAM].getValue();
    }

    float slewParam = clamp(params[SLEW_PARAM].getValue() + (inputs[SLEWCVIN_INPUT].isConnected() ? clamp(inputs[SLEWCVIN_INPUT].getVoltage(), -5.f, 5.f) / 10.f : 0.f), 0.f, 1.f);
    float alpha = (slewParam < args.sampleTime) ? 1.f : clamp(args.sampleTime / slewParam, 0.f, 1.f);
    xyX += (targetX - xyX) * alpha;
    xyY += (targetY - xyY) * alpha;
    xyFinalX.store(xyX, rlx);
    xyFinalY.store(xyY, rlx);

    outputs[XCVOUT_OUTPUT].setVoltage((xyX - 0.5f) * 10.f);
    outputs[YCVOUT_OUTPUT].setVoltage((xyY - 0.5f) * 10.f);
}
};


struct ReverseMenuItem : ui::MenuItem {
	Tehom* tehom;
	int channel;
	void onAction(const ActionEvent& e) override {
		tehom->pendingReverseFlip[channel] = !tehom->pendingReverseFlip[channel];
	}
	void step() override {
		rightText = CHECKMARK(tehom->playReversed[channel]);
		ui::MenuItem::step();
	}
};

struct SpinningBezelWidget : LEDBezelSilver {
	Tehom* tehom = nullptr;
	int channel = 0;
	ui::Tooltip* tooltip = nullptr;
	bool isHovered = false;
	float lastParamValue = -1.f;

	void onDragStart(const DragStartEvent& e) override {
		if (tehom) tehom->bezelDragging[channel].store(true);
		// don't call base — suppress knob drag behaviour
	}

	void onDragMove(const DragMoveEvent& e) override {
		if (!tehom) return;
		// Combine X and Y: right/up = forward, left/down = backward.
		// Screen Y is flipped, so subtract mouseDelta.y (up = negative screen Y = forward).
		float dragDelta = e.mouseDelta.x - e.mouseDelta.y;
		tehom->scrubInput[channel].store(dragDelta);
		tehom->scrubFresh[channel].store(true);
		// Bezel visual is driven by scrubVelocity in process() so it matches
		// the slewed audio — do not update the param directly here.
	}

	void onDragEnd(const DragEndEvent& e) override {
		if (tehom) {
			tehom->scrubInput[channel].store(0.f);
			tehom->scrubFresh[channel].store(true);   // signal velocity = 0 on release
			tehom->bezelDragging[channel].store(false);
		}
		// don't call base — suppress knob drag behaviour
	}

	void onEnter(const EnterEvent& e) override {
		isHovered = true;
		// suppress default tooltip
	}

	void onLeave(const LeaveEvent& e) override {
		isHovered = false;
		if (tooltip) {
			APP->scene->removeChild(tooltip);
			delete tooltip;
			tooltip = nullptr;
		}
	}

	void step() override {
		LEDBezelSilver::step();

		// Mark the SvgKnob's internal FramebufferWidget dirty only when the param
		// value has changed — avoids a full SVG re-render every frame when idle.
		float v = getParamQuantity() ? getParamQuantity()->getValue() : 0.f;
		if (v != lastParamValue) {
			lastParamValue = v;
			for (widget::Widget* child : children) {
				if (auto* fb = dynamic_cast<widget::FramebufferWidget*>(child)) {
					fb->dirty = true;
					break;
				}
			}
		}

		bool shouldShow = isHovered && tehom && tehom->bezelDragging[channel].load();
		if (shouldShow && !tooltip) {
			tooltip = new ui::Tooltip;
			tooltip->text = "Scrub";
			APP->scene->addChild(tooltip);
		} else if (!shouldShow && tooltip) {
			APP->scene->removeChild(tooltip);
			delete tooltip;
			tooltip = nullptr;
		}
	}

	void appendContextMenu(Menu* menu) override {
		LEDBezelSilver::appendContextMenu(menu);
		menu->addChild(new ui::MenuSeparator);
		auto* item = new ReverseMenuItem;
		item->text = "Reverse";
		item->tehom = tehom;
		item->channel = channel;
		menu->addChild(item);
	}
};

struct RecordWidget : LEDBezel {
	Tehom* tehom = nullptr;
	int channel = 0;
	ui::Tooltip* tooltip = nullptr;
	bool isHovered = false;

	void onButton(const ButtonEvent& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS) {
			if (tehom) tehom->pendingErase[channel].store(true);
			e.consume(this);
			return;
		}
		LEDBezel::onButton(e);
	}

	void onEnter(const EnterEvent& e) override { isHovered = true; }

	void onLeave(const LeaveEvent& e) override {
		isHovered = false;
		if (tooltip) {
			APP->scene->removeChild(tooltip);
			delete tooltip;
			tooltip = nullptr;
		}
	}

	void step() override {
		LEDBezel::step();
		if (!tehom) return;

		bool flashing = tehom->eraseFlash[channel].load() > 0.f;
		std::string desiredText = flashing ? "Erased" :
		                          (isHovered ? (tehom->recordState[channel].load() ? "Recording..." : "Record") : "");

		if (desiredText.empty()) {
			if (tooltip) {
				APP->scene->removeChild(tooltip);
				delete tooltip;
				tooltip = nullptr;
			}
		} else {
			if (!tooltip) {
				tooltip = new ui::Tooltip;
				APP->scene->addChild(tooltip);
			}
			tooltip->text = desiredText;
		}
	}

};

struct PlayTooltip : ui::Tooltip {
	Tehom* tehom;
	int channel;
	void step() override {
		if (tehom) {
			if (!tehom->playState[channel].load())
				text = "Stop";
			else if (tehom->playReversed[channel].load())
				text = "Reverse";
			else
				text = "Forward";
		}
		ui::Tooltip::step();
	}
};

struct ReversePlayWidget : LEDBezel {
	Tehom* tehom = nullptr;
	int channel = 0;
	ui::Tooltip* tooltip = nullptr;

	void onButton(const ButtonEvent& e) override {
		if (e.button == GLFW_MOUSE_BUTTON_RIGHT && e.action == GLFW_PRESS) {
			if (tehom)
				tehom->pendingReverseFlip[channel].store(!tehom->pendingReverseFlip[channel].load());
			e.consume(this);
			return;
		}
		LEDBezel::onButton(e);
	}

	void onEnter(const EnterEvent& e) override {
		auto* tip = new PlayTooltip;
		tip->tehom = tehom;
		tip->channel = channel;
		APP->scene->addChild(tip);
		tooltip = tip;
	}

	void onLeave(const LeaveEvent& e) override {
		if (tooltip) {
			APP->scene->removeChild(tooltip);
			delete tooltip;
			tooltip = nullptr;
		}
	}
};


////////////////////////////////
// XY PAD DISPLAY
////////////////////////////////
struct QuadLooperXYDisplay : Widget {
    Tehom *module;
    Vec dragPos;
    bool dragging = false;
    double lastClickTime = -1.0;

    struct TrailPoint { Vec pos; float alpha; float angle; bool facingRight; };
    std::vector<TrailPoint> trail;
    Vec lastTrailPos = {-1.f, -1.f};  // sentinel: uninitialized

    std::shared_ptr<window::Svg> fishSvg;
    bool lastFishDark = false;

    // Fish orientation state
    float prevPx      = -1.f, prevPy = -1.f;
    float smoothVx    = 0.f,  smoothVy = 0.f;
    float fishAngle   = 0.f;
    bool  fishFacingRight = false;
    Vec   rawMouseDelta   = {0.f, 0.f};  // accumulated raw mouse delta between draw calls

    void updateFromPos() {
        dragPos.x = clamp(dragPos.x, 0.f, box.size.x);
        dragPos.y = clamp(dragPos.y, 0.f, box.size.y);

        float x = dragPos.x / box.size.x;
        float y = 1.f - (dragPos.y / box.size.y);

        module->params[Tehom::XPOS_PARAM].setValue(x);
        module->params[Tehom::YPOS_PARAM].setValue(y);
    }

    void onButton(const event::Button &e) override {
        if (!module) return;
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_PRESS) {
#ifndef METAMODULE
            // Double-click snaps cursor to center
            double now = glfwGetTime();
            if (now - lastClickTime < 0.3) {
                lastClickTime = -1.0;
                dragPos.x = box.size.x * 0.5f;
                dragPos.y = box.size.y * 0.5f;
                updateFromPos();
                // Reset fish to centered, facing right
                fishFacingRight = true;
                fishAngle       = 0.f;
                smoothVx        = 0.f;
                smoothVy        = 0.f;
                prevPx          = -1.f;
                prevPy          = -1.f;
                rawMouseDelta   = {0.f, 0.f};
                e.consume(this);
                return;
            }
            lastClickTime = now;
#endif

            dragging = true;
            module->xyDragging.store(true);

            // Snap param to current CV-modulated position so cursor doesn't jump
            dragPos.x = module->xyFinalX.load(std::memory_order_relaxed) * box.size.x;
            dragPos.y = (1.f - module->xyFinalY.load(std::memory_order_relaxed)) * box.size.y;

            // Move to clicked position
            dragPos = e.pos;
            dragPos.x = clamp(dragPos.x, 0.f, box.size.x);
            dragPos.y = clamp(dragPos.y, 0.f, box.size.y);

            updateFromPos();
            e.consume(this);
        }
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && e.action == GLFW_RELEASE) {
            dragging = false;
            module->xyDragging.store(false);
        }
    }

    void onDragMove(const event::DragMove &e) override {
        if (!module || !dragging) return;

        Vec delta = e.mouseDelta.div(getAbsoluteZoom());
        rawMouseDelta = rawMouseDelta.plus(delta);

        dragPos = dragPos.plus(delta);
        dragPos.x = clamp(dragPos.x, 0.f, box.size.x);
        dragPos.y = clamp(dragPos.y, 0.f, box.size.y);

        updateFromPos();
    }

    void onDragEnd(const event::DragEnd &e) override {
        if (module) module->xyDragging.store(false);
        dragging = false;
    }

    // Helper: draw fish SVG centered at (cx, cy), rotated by angle, optionally flipped, scaled to fishW x fishH
    void drawFish(const DrawArgs& args, float cx, float cy,
                  float fishW, float fishH, float fishSvgW,
                  float angle, bool facingRight, float alpha) {
        if (fishW <= 0.f) return;
        nvgSave(args.vg);
        if (alpha < 1.f) nvgGlobalAlpha(args.vg, alpha);
        nvgTranslate(args.vg, cx, cy);
        nvgRotate(args.vg, angle);
        if (facingRight) nvgScale(args.vg, -1.f, 1.f);
        nvgTranslate(args.vg, -fishW * 0.5f, -fishH * 0.5f);
        nvgScale(args.vg, fishW / fishSvgW, fishW / fishSvgW);
#ifndef METAMODULE
        window::svgDraw(args.vg, fishSvg->handle);
#endif
        nvgRestore(args.vg);
    }

    void step() override {
        Widget::step();
        bool dark = settings::preferDarkPanels;
        if (!fishSvg || dark != lastFishDark) {
            fishSvg = window::Svg::load(asset::plugin(pluginInstance, dark
                ? "res/components/Fish-Dark.svg"
                : "res/components/Fish.svg"));
            lastFishDark = dark;
        }
    }

    void draw(const DrawArgs &args) override {
        float radius = 6.f;
        bool dark = settings::preferDarkPanels;

        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, radius);
        nvgFillColor(args.vg, dark ? nvgRGB(255, 255, 255) : nvgRGB(18, 18, 18));
        nvgFill(args.vg);

        if (!module) return;

        bool useFish = (module->cursorStyle == 0) && fishSvg && fishSvg->handle;
        float fishW = 0.f, fishH = 0.f, fishSvgW = 0.f;
#ifndef METAMODULE
        if (useFish) {
            fishSvgW = fishSvg->handle->width;
            fishW    = radius * 2.5f;
            fishH    = fishSvg->handle->height * (fishW / fishSvgW);
        }
#else
        useFish = false;
#endif

        float px = module->xyFinalX.load(std::memory_order_relaxed) * box.size.x;
        float py = (1.f - module->xyFinalY.load(std::memory_order_relaxed)) * box.size.y;

        // Clamp cursor within bounds
        if (useFish) {
            float ca = std::abs(std::cos(fishAngle));
            float sa = std::abs(std::sin(fishAngle));
            float halfBoundW = (fishW * 0.5f) * ca + (fishH * 0.5f) * sa;
            float halfBoundH = (fishW * 0.5f) * sa + (fishH * 0.5f) * ca;
            px = clamp(px, halfBoundW, box.size.x - halfBoundW);
            py = clamp(py, halfBoundH, box.size.y - halfBoundH);
        } else {
            px = clamp(px, radius, box.size.x - radius);
            py = clamp(py, radius, box.size.y - radius);
        }

        // --- Fish orientation: velocity tracking ---
        if (module->xyResetPending.exchange(false)) {
            fishFacingRight = true;
            fishAngle       = 0.f;
            smoothVx        = 0.f;
            smoothVy        = 0.f;
            prevPx          = -1.f;
            prevPy          = -1.f;
            rawMouseDelta   = {0.f, 0.f};
        }
        if (useFish) {
            float dvx, dvy;
            if (dragging && (rawMouseDelta.x != 0.f || rawMouseDelta.y != 0.f)) {
                // Use raw mouse input for immediate, slew-independent direction
                dvx = rawMouseDelta.x;
                dvy = rawMouseDelta.y;
            } else {
                if (prevPx < 0.f) { prevPx = px; prevPy = py; }
                dvx = px - prevPx;
                dvy = py - prevPy;
            }
            // Always drain rawMouseDelta so it can't accumulate noise at walls
            rawMouseDelta = {0.f, 0.f};
            prevPx = px; prevPy = py;

            // Lighter smoothing so fast moves and CV/wander motion register quickly
            float emaK = dragging ? 0.55f : 0.50f;
            smoothVx = smoothVx * emaK + dvx * (1.f - emaK);
            smoothVy = smoothVy * emaK + dvy * (1.f - emaK);

            float speed = std::sqrt(smoothVx * smoothVx + smoothVy * smoothVy);
            float speedThresh = dragging ? 0.4f : 0.15f;
            if (speed > speedThresh) {
                // Flip hysteresis: wider dead zone prevents oscillation near vx=0
                bool newFacing = fishFacingRight;
                if (smoothVx >  0.15f) newFacing = true;
                if (smoothVx < -0.15f) newFacing = false;

                // If facing flipped, rotate fishAngle by π so the fish doesn't visually jump
                if (newFacing != fishFacingRight) {
                    fishAngle += float(M_PI);
                    while (fishAngle >  float(M_PI)) fishAngle -= 2.f * float(M_PI);
                    while (fishAngle < -float(M_PI)) fishAngle += 2.f * float(M_PI);
                    fishFacingRight = newFacing;
                }

                // Target angle: align fish head with velocity direction
                float targetAngle;
                if (fishFacingRight) {
                    targetAngle = std::atan2(smoothVy, smoothVx);
                } else {
                    targetAngle = std::atan2(smoothVy, smoothVx) - float(M_PI);
                    while (targetAngle >  float(M_PI)) targetAngle -= 2.f * float(M_PI);
                    while (targetAngle < -float(M_PI)) targetAngle += 2.f * float(M_PI);
                }

                // Smooth angle (shortest arc)
                float diff = targetAngle - fishAngle;
                while (diff >  float(M_PI)) diff -= 2.f * float(M_PI);
                while (diff < -float(M_PI)) diff += 2.f * float(M_PI);
                float lerpK = dragging ? 0.45f : 0.35f;
                fishAngle += diff * lerpK;
            }
        } else {
            // Reset when switching away from fish cursor
            prevPx = -1.f; prevPy = -1.f;
            smoothVx = 0.f; smoothVy = 0.f;
            fishAngle = 0.f; fishFacingRight = false;
        }

        // --- Trail ---
        if (module->persist) {
            Vec cur(px, py);
            if (lastTrailPos.x < 0.f) lastTrailPos = cur;
            Vec delta = cur.minus(lastTrailPos);
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            int steps = std::max(1, (int)(dist / 5.f));
            for (int s = 0; s < steps; s++) {
                float t = (float)s / (float)steps;
                Vec p = lastTrailPos.plus(delta.mult(t));
                trail.push_back({p, 1.f, fishAngle, fishFacingRight});
            }
            lastTrailPos = cur;
            if (trail.size() > 80)
                trail.erase(trail.begin(), trail.begin() + (int)(trail.size() - 80));

            for (auto& tp : trail) {
                if (useFish) {
                    float tW = fishW * tp.alpha;
                    float tH = fishH * tp.alpha;
                    drawFish(args, tp.pos.x, tp.pos.y, tW, tH, fishSvgW,
                             tp.angle, tp.facingRight, tp.alpha * 0.7f);
                } else {
                    float r = radius * tp.alpha;
                    if (r > 0.f) {
                        nvgBeginPath(args.vg);
                        nvgCircle(args.vg, tp.pos.x, tp.pos.y, r);
                        nvgFillColor(args.vg, dark ? nvgRGBAf(0.f, 0.f, 0.f, tp.alpha * 0.7f) : nvgRGBAf(1.f, 1.f, 1.f, tp.alpha * 0.7f));
                        nvgFill(args.vg);
                    }
                }
                tp.alpha -= 0.12f;
            }
            trail.erase(
                std::remove_if(trail.begin(), trail.end(), [](const TrailPoint& tp) { return tp.alpha <= 0.f; }),
                trail.end()
            );
        } else if (!trail.empty()) {
            trail.clear();
            lastTrailPos = {-1.f, -1.f};
        }

        // --- Crosshairs ---
        if (module->showCrosshairs) {
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, px, 0);
            nvgLineTo(args.vg, px, box.size.y);
            nvgMoveTo(args.vg, 0, py);
            nvgLineTo(args.vg, box.size.x, py);
            nvgStrokeColor(args.vg, dark ? nvgRGB(200, 200, 200) : nvgRGB(60, 60, 60));
            nvgStroke(args.vg);
        }

        // --- Cursor ---
        if (useFish) {
            drawFish(args, px, py, fishW, fishH, fishSvgW, fishAngle, fishFacingRight, 1.f);
        } else {
            nvgBeginPath(args.vg);
            nvgCircle(args.vg, px, py, radius);
            nvgFillColor(args.vg, dark ? nvgRGB(30, 30, 30) : nvgRGB(230, 230, 230));
            nvgFill(args.vg);
        }
    }
};

// Custom channel colors: Cyan, Magenta, Orange, Purple
struct ChanLight0 : GrayModuleLightWidget { ChanLight0() { addBaseColor(SCHEME_CYAN); } };
struct ChanLight1 : GrayModuleLightWidget { ChanLight1() { addBaseColor(SCHEME_PURPLE); } };
struct ChanLight2 : GrayModuleLightWidget { ChanLight2() { addBaseColor(SCHEME_ORANGE); } };
struct ChanLight3 : GrayModuleLightWidget { ChanLight3() { addBaseColor(SCHEME_RED); } };
struct TehomWidget : ModuleWidget {
	TehomWidget(Tehom* module) {
		setModule(module);
		setPanel(createPanel(
			asset::plugin(pluginInstance, "res/panels/Tehom-Vanilla.svg"),
			asset::plugin(pluginInstance, "res/panels/Tehom-Vanilla-Dark.svg")
		));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // === 40x40 XY Pad, centered ===
        auto xy = new QuadLooperXYDisplay();
        xy->module = module;

        float squareSize = 40.f; // 40 x 40 mm
        float xOffset = (152.4f - squareSize) / 2.f; // center horizontally
        float yOffset = (128.5f - squareSize) / 2.f; // center vertically

        xy->box.pos = mm2px(Vec(xOffset, yOffset));
        xy->box.size = mm2px(Vec(squareSize, squareSize));
        addChild(xy);

		// 2 Position switches 
		addParam(createParamCentered<CKSS>(mm2px(Vec(26.901, 54.204)), module, Tehom::LOOP1_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(125.316, 54.204)), module, Tehom::LOOP2_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(26.901, 115.975)), module, Tehom::LOOP3_PARAM));
		addParam(createParamCentered<CKSS>(mm2px(Vec(125.316, 115.975)), module, Tehom::LOOP4_PARAM));

		// LEDBezel buttons with lights (RECORD = red, PLAY = green)
		{
			Vec recPos[4] = {
				mm2px(Vec(14.738, 54.176)),
				mm2px(Vec(113.152, 54.176)),
				mm2px(Vec(14.738, 115.947)),
				mm2px(Vec(113.152, 115.947)),
			};
			for (int i = 0; i < 4; i++) {
				auto* rec = createParamCentered<RecordWidget>(recPos[i], module, Tehom::RECORD1_PARAM + i);
				rec->tehom = module;
				rec->channel = i;
				addParam(rec);
				addChild(createLightCentered<LEDBezelLight<RedLight>>(recPos[i], module, Tehom::RECORD1_LIGHT + i));
				addChild(createLightCentered<LEDBezelLight<WhiteLight>>(recPos[i], module, Tehom::RECORD1_FLASH_LIGHT + i));
			}
		}

		{
			Vec playPos[4] = {
				mm2px(Vec(39.026, 54.176)),
				mm2px(Vec(137.441, 54.176)),
				mm2px(Vec(39.026, 115.947)),
				mm2px(Vec(137.441, 115.947)),
			};
			for (int i = 0; i < 4; i++) {
				auto* play = createParamCentered<ReversePlayWidget>(playPos[i], module, Tehom::PLAY1_PARAM + i);
				play->tehom = module;
				play->channel = i;
				addParam(play);
				addChild(createLightCentered<LEDBezelLight<GreenLight>>(playPos[i], module, Tehom::PLAY1_LIGHT + i));
				addChild(createLightCentered<LEDBezelLight<BlueLight>>(playPos[i], module, Tehom::PLAY1_BLUE_LIGHT + i));
			}
		}

		// Small knobs 
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(60.972, 36.448)), module, Tehom::WARBLE_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(71.215, 36.448)), module, Tehom::FILTER_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(81.457, 36.448)), module, Tehom::AMOUNT_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(60.972, 114.136)), module, Tehom::SLEW_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(71.215, 114.136)), module, Tehom::SIZE_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(81.457, 114.136)), module, Tehom::POSITION_PARAM));
		addParam(createParamCentered<TehomSmallKnob>(mm2px(Vec(91.8, 114.136)), module, Tehom::XFADE_PARAM));

		// Big Knobs
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(14.738, 39.243)), module, Tehom::SOURCE1_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(113.152, 39.243)), module, Tehom::SOURCE2_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(14.738, 101.014)), module, Tehom::SOURCE3_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(113.152, 101.014)), module, Tehom::SOURCE4_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(38.86, 39.243)), module, Tehom::SPEED1_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(137.275, 39.243)), module, Tehom::SPEED2_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(38.86, 101.014)), module, Tehom::SPEED3_PARAM));
		addParam(createParamCentered<TehomKnob>(mm2px(Vec(137.275, 101.014)), module, Tehom::SPEED4_PARAM));
	
		// Audio inputs
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(60.972, 11.851)), module, Tehom::AUDIOLEFTIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(71.215, 11.851)), module, Tehom::AUDIORIGHTIN_INPUT));

		// Inputs 
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(60.972, 26.591)), module, Tehom::WARBLECVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(71.215, 26.591)), module, Tehom::FILTERCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(81.457, 26.591)), module, Tehom::AMOUNTCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(91.627, 36.448)), module, Tehom::RETURN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(60.968, 90.436)), module, Tehom::XCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(71.215, 90.436)), module, Tehom::YCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(60.972, 106.395)), module, Tehom::SLEWCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(71.215, 106.395)), module, Tehom::SIZECVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(81.457, 106.395)), module, Tehom::POSITIONCVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(91.8, 106.395)), module, Tehom::XFADECVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(9.434, 23.541)), module, Tehom::SOURCE1CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(44.33, 23.541)), module, Tehom::SPEED1CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(20.788, 23.541)), module, Tehom::RECORD1CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(32.932, 23.541)), module, Tehom::PLAY1CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(107.848, 23.541)), module, Tehom::SOURCE2CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(142.745, 23.541)), module, Tehom::SPEED2CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(119.202, 23.541)), module, Tehom::RECORD2CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(131.347, 23.541)), module, Tehom::PLAY2CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(9.434, 85.312)), module, Tehom::SOURCE3CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(44.33, 85.312)), module, Tehom::SPEED3CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(20.788, 85.312)), module, Tehom::RECORD3CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(32.932, 85.312)), module, Tehom::PLAY3CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(107.848, 85.312)), module, Tehom::SOURCE4CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(142.745, 85.312)), module, Tehom::SPEED4CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(119.202, 85.312)), module, Tehom::RECORD4CVIN_INPUT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(131.347, 85.312)), module, Tehom::PLAY4CVIN_INPUT));

		// Outputs
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(81.457, 11.912)), module, Tehom::AUDIOLEFTOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.8, 11.912)), module, Tehom::AUDIORIGHTOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.627, 26.591)), module, Tehom::SEND_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(81.41, 90.353)), module, Tehom::XCVOUT_OUTPUT));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(91.8, 90.669)), module, Tehom::YCVOUT_OUTPUT));

		// LED Bezels
		for (int i = 0; i < 4; i++) {
			Vec positions[4] = {
				mm2px(Vec(26.901, 10.166)),
				mm2px(Vec(125.316, 10.166)),
				mm2px(Vec(26.901, 71.937)),
				mm2px(Vec(125.316, 71.937)),
			};
			auto* bezel = createParamCentered<SpinningBezelWidget>(positions[i], module, Tehom::LEDBEZEL1_PARAM + i);
			bezel->tehom = module;
			bezel->channel = i;
			addParam(bezel);
		}

		// Lights — each channel has a distinct color, brightness driven by audio output
		addChild(createLightCentered<LargeLight<ChanLight0>>(mm2px(Vec(26.9, 10.166)), module, Tehom::BUFFER1LED_LIGHT));
		addChild(createLightCentered<LargeLight<ChanLight1>>(mm2px(Vec(125.315, 10.166)), module, Tehom::BUFFER2LED_LIGHT));
		addChild(createLightCentered<LargeLight<ChanLight2>>(mm2px(Vec(26.9, 71.937)), module, Tehom::BUFFER3LED_LIGHT));
		addChild(createLightCentered<LargeLight<ChanLight3>>(mm2px(Vec(125.315, 71.937)), module, Tehom::BUFFER4LED_LIGHT));
	}
    void appendContextMenu(Menu* menu) override {
        ModuleWidget::appendContextMenu(menu);
        auto* tehom = dynamic_cast<Tehom*>(module);
        if (!tehom) return;

        // GUI section
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("HINT: hold control (command on Mac) to keep menu open"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("GUI"));
        menu->addChild(createSubmenuItem("XY Pad", "", [=](Menu* subMenu) {
            subMenu->addChild(createMenuLabel("Behavior"));
            subMenu->addChild(createCheckMenuItem("Draw Crosshairs", "",
                [=]() { return tehom->showCrosshairs; },
                [=]() { tehom->showCrosshairs = !tehom->showCrosshairs; }
            ));
            subMenu->addChild(createCheckMenuItem("Cursor Trails", "",
                [=]() { return tehom->persist; },
                [=]() { tehom->persist = !tehom->persist; }
            ));
            subMenu->addChild(new MenuSeparator);
            subMenu->addChild(createMenuLabel("Cursor"));
            subMenu->addChild(createCheckMenuItem("Fish", "",
                [=]() { return tehom->cursorStyle == 0; },
                [=]() { tehom->cursorStyle = 0; }
            ));
            subMenu->addChild(createCheckMenuItem("Circle", "",
                [=]() { return tehom->cursorStyle == 1; },
                [=]() { tehom->cursorStyle = 1; }
            ));
            subMenu->addChild(new MenuSeparator);
            subMenu->addChild(createMenuLabel("Wander Speed"));
            subMenu->addChild(createCheckMenuItem("Off", "",
                [=]() { return tehom->wanderMode.load() == 0; },
                [=]() { tehom->wanderMode.store(0); }
            ));
            const char* wanderNames[] = {"Slow", "Medium", "Fast"};
            for (int m = 1; m <= 3; m++) {
                subMenu->addChild(createCheckMenuItem(wanderNames[m - 1], "",
                    [=]() { return tehom->wanderMode.load() == m; },
                    [=]() {
                        int prev = tehom->wanderMode.load();
                        tehom->wanderMode.store(m);
                        if (prev == 0) {
                            tehom->wanderTargetX.store(tehom->xyFinalX.load());
                            tehom->wanderTargetY.store(tehom->xyFinalY.load());
                            tehom->wanderTimer.store(0.f);
                            tehom->wanderAngle.store(random::uniform() * 2.f * float(M_PI));
                            tehom->wanderSpeed.store(0.15f);
                        }
                    }
                ));
            }
        }));

        // Global section
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Global"));
        menu->addChild(createCheckMenuItem("Save Buffers With Patch", "",
            [=]() { return tehom->persistBuffers; },
            [=]() { tehom->persistBuffers = !tehom->persistBuffers; }
        ));
        menu->addChild(createCheckMenuItem("XY Pad Pans Audio", "",
            [=]() { return tehom->xyPadPansAudio.load(); },
            [=]() { tehom->xyPadPansAudio.store(!tehom->xyPadPansAudio.load()); }
        ));
        menu->addChild(createSubmenuItem("Buffer Size", "", [=](Menu* subMenu) {
            const float durations[] = {1.f, 2.f, 5.f, 10.f, 20.f, 30.f, 60.f};
            const char* labels[]    = {"1 second", "2 seconds", "5 seconds", "10 seconds", "20 seconds", "30 seconds", "1 minute"};
            for (int j = 0; j < 7; j++) {
                float dur = durations[j];
                subMenu->addChild(createCheckMenuItem(
                    labels[j], "",
                    [=]() { return tehom->bufferDuration == dur; },
                    [=]() { tehom->bufferDuration = dur; tehom->resizeBuffers(tehom->currentSampleRate); }
                ));
            }
        }));
        // Noise section
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Noise"));
        {
            const char* mediaNames[] = {"Mic Preamp", "Reel To Reel", "Cassette", "VHS", "Vinyl Clean", "Vinyl Dirty", "8mm Film", "16mm Film"};
            menu->addChild(createSubmenuItem("Media Type", "", [=](Menu* subMenu) {
                for (int m = 0; m < 8; m++) {
                    subMenu->addChild(createCheckMenuItem(mediaNames[m], "",
                        [=]() { return tehom->mediaTypeIndex.load() == m; },
                        [=]() { tehom->mediaTypeIndex.store(m); }
                    ));
                }
            }));
        }

        // Channels section
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Channels"));
        const std::vector<std::string> modeNames = {"Play/Stop", "Retrigger", "Forward/Reverse"};
        for (int i = 0; i < 4; i++) {
            menu->addChild(createSubmenuItem("Channel " + std::to_string(i + 1), "", [=](Menu* subMenu) {
                subMenu->addChild(createMenuLabel("Recording"));
                subMenu->addChild(createCheckMenuItem(
                    "Continuous Record", "",
                    [=]() { return tehom->continuousRecord[i].load(); },
                    [=]() { tehom->continuousRecord[i].store(!tehom->continuousRecord[i].load()); }
                ));
                subMenu->addChild(createCheckMenuItem(
                    "Record Source: Main Output", "",
                    [=]() { return tehom->recordMainOutput[i].load(); },
                    [=]() { tehom->recordMainOutput[i].store(!tehom->recordMainOutput[i].load()); }
                ));
                subMenu->addChild(new MenuSeparator);
                subMenu->addChild(createMenuLabel("Auto-Play"));
                subMenu->addChild(createCheckMenuItem(
                    "Auto-Play when recording complete", "",
                    [=]() { return tehom->autoPlay[i].load(); },
                    [=]() { tehom->autoPlay[i].store(!tehom->autoPlay[i].load()); }
                ));
                subMenu->addChild(new MenuSeparator);
                subMenu->addChild(createMenuLabel("Play CV Mode"));
                for (int m = 0; m < 3; m++) {
                    subMenu->addChild(createCheckMenuItem(
                        modeNames[m], "",
                        [=]() { return tehom->playCVMode[i].load() == m; },
                        [=]() { tehom->playCVMode[i].store(m); }
                    ));
                }
            }));
        }
    }
};
Model* modelTehom = createModel<Tehom, TehomWidget>("Tehom");