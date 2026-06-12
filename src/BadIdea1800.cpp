#include "plugin.hpp"
#include "1800Manifest.hpp"
#include <atomic>
#include <cstring>
#include <cmath>
#ifdef METAMODULE
// Kits are embedded in the plugin's rodata (one shared copy for all instances);
// voices play directly from the embedded float PCM — no file I/O, no vectors.
#include "../res/samples/1800/bi1800_kits_data.hpp"
#else
#include <cstdio>
#include <vector>
#endif

#define TWO_PI (2.0f * M_PI)

struct BadIdea1800 : Module {
    enum ParamId { PARAMS_LEN };
    enum InputId {
        NUM_1_INPUT, NUM_2_INPUT, NUM_3_INPUT,
        NUM_4_INPUT, NUM_5_INPUT, NUM_6_INPUT,
        NUM_7_INPUT, NUM_8_INPUT, NUM_9_INPUT,
        ZERO_INPUT, ASTERISK_INPUT, POUND_INPUT,
        INPUTS_LEN
    };
    enum OutputId { BADIDEA_OUTPUT, OUTPUTS_LEN };
    enum LightId {
        LED_1_LIGHT, LED_2_LIGHT, LED_3_LIGHT,
        LED_4_LIGHT, LED_5_LIGHT, LED_6_LIGHT,
        LED_7_LIGHT, LED_8_LIGHT, LED_9_LIGHT,
        LED_10_LIGHT, LED_ASTERISK_LIGHT, LED_POUND_LIGHT,
        LIGHTS_LEN
    };

    // Voice index -> LED index (panel layout order)
    static constexpr int VOICE_TO_LED[12] = {
        LED_1_LIGHT, LED_2_LIGHT, LED_3_LIGHT,
        LED_4_LIGHT, LED_5_LIGHT, LED_6_LIGHT,
        LED_7_LIGHT, LED_8_LIGHT, LED_9_LIGHT,
        LED_ASTERISK_LIGHT, LED_10_LIGHT, LED_POUND_LIGHT
    };

    // Voice index -> sample slot in kit
    // Enum order: 0-8=buttons 1-9, 9=ZERO, 10=ASTERISK, 11=POUND
    // Hardware .ino: playMem1-9=buttons 1-9, playMem10=*, playMem11=0, playMem12=#
    static constexpr int VOICE_TO_SLOT[12] = {0,1,2,3,4,5,6,7,8,10,9,11};

    static constexpr int NUM_VOICES = 12;

    // DTMF frequencies for original firmware (rows × columns)
    // Order matches input enum: 1,2,3,4,5,6,7,8,9,0,*,#
    static constexpr float DTMF_FREQ[12][2] = {
        {697.f, 1209.f}, {697.f, 1336.f}, {697.f, 1477.f},
        {770.f, 1209.f}, {770.f, 1336.f}, {770.f, 1477.f},
        {852.f, 1209.f}, {852.f, 1336.f}, {852.f, 1477.f},
        {941.f, 1336.f}, {941.f, 1209.f}, {941.f, 1477.f},
    };

    // -1 = original DTMF firmware, 0..N-1 = alt drum machine kits
    int kitIndex = -1;
    std::atomic<int> pendingKit{-2}; // -2 = no pending change

    // Per-slot sample data. Points into embedded rodata (MetaModule) or into the
    // loaded vectors (desktop). Set by loadKit(), read by the voices.
    const float* slotData[NUM_VOICES]  = {};
    uint32_t     slotCount[NUM_VOICES] = {};
    float        slotRate[NUM_VOICES]  = {};
#ifndef METAMODULE
    std::vector<float> kitSamples[NUM_VOICES]; // desktop only: owns the loaded PCM
#endif

    // DTMF phase accumulators (only used when kitIndex == -1)
    float dtmfPhase1[NUM_VOICES] = {};
    float dtmfPhase2[NUM_VOICES] = {};

    struct Voice {
        const float* buf = nullptr;
        uint32_t len = 0;
        uint32_t pos = 0;
        float phase = 0.f;
        float lastSample = 0.f;
        bool playing = false;

        void trigger(const float* b, uint32_t n) {
            buf = b;
            len = n;
            pos = 0;
            phase = 0.f;
            lastSample = 0.f;
            playing = (b && n > 0);
        }

        // ratio = sourceRate / hostRate; sample-and-hold resampling
        float next(float ratio) {
            if (!playing) return 0.f;
            phase += ratio;
            while (phase >= 1.f) {
                if (pos >= len) { playing = false; return lastSample; }
                lastSample = buf[pos++];
                phase -= 1.f;
            }
            return lastSample;
        }
    } voices[NUM_VOICES];

    bool lastGate[NUM_VOICES] = {};

    BadIdea1800() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configInput(NUM_1_INPUT,    "1 Gate");
        configInput(NUM_2_INPUT,    "2 Gate");
        configInput(NUM_3_INPUT,    "3 Gate");
        configInput(NUM_4_INPUT,    "4 Gate");
        configInput(NUM_5_INPUT,    "5 Gate");
        configInput(NUM_6_INPUT,    "6 Gate");
        configInput(NUM_7_INPUT,    "7 Gate");
        configInput(NUM_8_INPUT,    "8 Gate");
        configInput(NUM_9_INPUT,    "9 Gate");
        configInput(ZERO_INPUT,     "0 Gate");
        configInput(ASTERISK_INPUT, "* Gate");
        configInput(POUND_INPUT,    "# Gate");
        configOutput(BADIDEA_OUTPUT, "Bad Idea 1800-Call-Yer-Mum Audio");
    }

    void clearSlots() {
        for (int i = 0; i < NUM_VOICES; i++) { slotData[i] = nullptr; slotCount[i] = 0; }
        for (auto& v : voices) v.playing = false;
    }

    void loadKit(int idx) {
        if (idx == -1) { // original DTMF firmware
            clearSlots();
            kitIndex = -1;
            return;
        }
        if (idx < 0 || idx >= BADIDEA1800_NUM_KITS) return;

#ifdef METAMODULE
        // Parse the embedded blob header and point the slots into rodata (no copy).
        const uint8_t* blob = BI1800_KIT_DATA[idx];
        if (blob[0] != '1' || blob[1] != '8' || blob[2] != '0' || blob[3] != '0') return;
        uint32_t counts[NUM_VOICES];
        std::memcpy(counts,   blob + 4,            sizeof(counts));   // counts[12]
        std::memcpy(slotRate, blob + 4 + 48,       sizeof(slotRate)); // rates[12]
        // header = magic(4) + counts(48) + rates(48) + names(16*12)
        const float* data = reinterpret_cast<const float*>(blob + 4 + 48 + 48 + 16 * NUM_VOICES);
        uint32_t off = 0;
        for (int i = 0; i < NUM_VOICES; i++) {
            slotData[i]  = data + off;
            slotCount[i] = counts[i];
            off += counts[i];
        }
#else
        std::string path = asset::plugin(pluginInstance,
            "res/samples/1800/" + std::string(BADIDEA1800_KITS[idx].filename) + ".bin");
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) return;
        char magic[4];
        fread(magic, 1, 4, f);
        if (magic[0] != '1' || magic[1] != '8' || magic[2] != '0' || magic[3] != '0') {
            fclose(f); return;
        }
        uint32_t counts[NUM_VOICES];
        fread(counts, sizeof(uint32_t), NUM_VOICES, f);
        fread(slotRate, sizeof(float), NUM_VOICES, f);
        fseek(f, 16 * NUM_VOICES, SEEK_CUR); // skip name table
        for (int i = 0; i < NUM_VOICES; i++) {
            kitSamples[i].resize(counts[i]);
            if (counts[i] > 0)
                fread(kitSamples[i].data(), sizeof(float), counts[i], f);
            slotData[i]  = kitSamples[i].data();
            slotCount[i] = counts[i];
        }
        fclose(f);
#endif
        kitIndex = idx;
        for (auto& v : voices) v.playing = false;
    }

    void selectKit(int idx) {
        pendingKit.store(idx);
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "kit", json_integer(kitIndex));
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* k = json_object_get(rootJ, "kit");
        if (k) pendingKit.store((int)json_integer_value(k)); // applied in process()
    }

    void process(const ProcessArgs& args) override {
        int pending = pendingKit.exchange(-2);
        if (pending != -2) loadKit(pending);

        float totalSum = 0.f;

        if (kitIndex == -1) {
            // Original firmware: DTMF tone generation
            const float dt = 1.f / args.sampleRate;
            for (int i = 0; i < NUM_VOICES; i++) {
                dtmfPhase1[i] += DTMF_FREQ[i][0] * TWO_PI * dt;
                dtmfPhase2[i] += DTMF_FREQ[i][1] * TWO_PI * dt;
                if (dtmfPhase1[i] >= TWO_PI) dtmfPhase1[i] -= TWO_PI;
                if (dtmfPhase2[i] >= TWO_PI) dtmfPhase2[i] -= TWO_PI;

                float inputV = inputs[NUM_1_INPUT + i].getVoltage();
                bool gateHigh = (inputV > 0.5f);

                float out = 0.f;
                if (gateHigh) {
                    out = (sinf(dtmfPhase1[i]) + sinf(dtmfPhase2[i])) * inputV * 2.5f;
                    out *= (1.f / 48.f);
                }
                totalSum += out;

                lights[VOICE_TO_LED[i]].setBrightnessSmooth(gateHigh ? 1.f : 0.f, args.sampleTime);
            }
            outputs[BADIDEA_OUTPUT].setVoltage(clamp(totalSum, -5.f, 5.f));
        } else {
            // Alt firmware: sample playback
            for (int i = 0; i < NUM_VOICES; i++) {
                bool gateHigh = inputs[NUM_1_INPUT + i].getVoltage() > 0.5f;
                int slot = VOICE_TO_SLOT[i];

                if (gateHigh && !lastGate[i])
                    voices[i].trigger(slotData[slot], slotCount[slot]);
                lastGate[i] = gateHigh;

                float ratio = slotRate[slot] / args.sampleRate;
                totalSum += voices[i].next(ratio);

                float brightness = voices[i].playing ? (gateHigh ? 1.f : 0.3f) : 0.f;
                lights[VOICE_TO_LED[i]].setBrightnessSmooth(brightness, args.sampleTime);
            }
            outputs[BADIDEA_OUTPUT].setVoltage(clamp(totalSum * 25.f / NUM_VOICES, -5.f, 5.f));
        }
    }
};

constexpr int BadIdea1800::VOICE_TO_LED[12];
constexpr int BadIdea1800::VOICE_TO_SLOT[12];
constexpr float BadIdea1800::DTMF_FREQ[12][2];

struct BadIdea1800Widget : ModuleWidget {
    BadIdea1800Widget(BadIdea1800* module) {
        setModule(module);
        setPanel(createPanel(
            asset::plugin(pluginInstance, "res/panels/BadIdea1800.svg"),
            asset::plugin(pluginInstance, "res/panels/BadIdea1800-dark.svg")
        ));

        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.502,  22.904)), module, BadIdea1800::NUM_1_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.595, 22.904)), module, BadIdea1800::NUM_2_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(52.555, 22.94)),  module, BadIdea1800::NUM_3_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.541,  44.903)), module, BadIdea1800::NUM_4_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.559, 44.845)), module, BadIdea1800::NUM_5_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(52.576, 44.957)), module, BadIdea1800::NUM_6_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.541,  66.899)), module, BadIdea1800::NUM_7_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.575, 66.881)), module, BadIdea1800::NUM_8_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(52.553, 66.92)),  module, BadIdea1800::NUM_9_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(30.576, 88.861)), module, BadIdea1800::ZERO_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.52,   88.824)), module, BadIdea1800::ASTERISK_INPUT));
        addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(52.555, 88.86)),  module, BadIdea1800::POUND_INPUT));

        addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(52.517, 110.859)), module, BadIdea1800::BADIDEA_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.569,  11.873)), module, BadIdea1800::LED_1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.52,  11.96)),  module, BadIdea1800::LED_2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.553, 11.885)), module, BadIdea1800::LED_3_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.54,   33.92)),  module, BadIdea1800::LED_4_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.539, 33.884)), module, BadIdea1800::LED_5_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.536, 33.902)), module, BadIdea1800::LED_6_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.56,   55.921)), module, BadIdea1800::LED_7_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.558, 55.903)), module, BadIdea1800::LED_8_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.534, 55.936)), module, BadIdea1800::LED_9_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(8.541,  77.917)), module, BadIdea1800::LED_ASTERISK_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(30.575, 77.897)), module, BadIdea1800::LED_10_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(52.556, 77.898)), module, BadIdea1800::LED_POUND_LIGHT));
    }

    void appendContextMenu(Menu* menu) override {
        BadIdea1800* module = dynamic_cast<BadIdea1800*>(this->module);
        if (!module) return;

        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Firmware"));

        menu->addChild(createCheckMenuItem("Original Firmware", "",
            [=]() { return module->kitIndex == -1; },
            [=]() { module->selectKit(-1); }
        ));

        menu->addChild(createSubmenuItem("Alt Firmware", "", [=](Menu* submenu) {
            for (int i = 0; i < BADIDEA1800_NUM_KITS; i++) {
                int capturedI = i;
                submenu->addChild(createCheckMenuItem(BADIDEA1800_KITS[i].displayName, "",
                    [=]() { return module->kitIndex == capturedI; },
                    [=]() { module->selectKit(capturedI); }
                ));
            }
        }));
    }
};

Model* modelBadIdea1800 = createModel<BadIdea1800, BadIdea1800Widget>("BadIdea1800");
