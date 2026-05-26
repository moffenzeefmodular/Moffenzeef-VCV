# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### VCV Rack
```bash
make          # build plugin .so/.dylib
make dist     # create distributable ZIP
```
Requires `RACK_DIR` pointing to the VCV Rack SDK (defaults to `../..`).

### MetaModule
```bash
cmake -B build -DMETAMODULE_SDK_DIR=../metamodule-plugin-sdk
cmake --build build -j
```
Output goes to `metamodule-plugins/`. SDK path can be set via the `METAMODULE_SDK_DIR` env var or cmake variable. The SDK is a git submodule at `metamodule-plugin-sdk/`.

## Architecture

This is a **dual-target VCV Rack plugin**: the same C++ source compiles for both VCV Rack (desktop) and MetaModule (ARM hardware). MetaModule-specific code is gated with `#ifdef METAMODULE`. All modules are registered in `src/plugin.cpp`. New modules need entries in `plugin.json` (VCV Rack + MetaModule) and `plugin-mm.json` (MetaModule only — see `MetaModuleIncludedModules`).

- `src/` — All module source files (one `.cpp` per module) plus shared `plugin.hpp` / `plugin.cpp`
- `res/` — SVG assets for VCV Rack (panels, components)
- `assets/` — PNG assets for MetaModule rendering (same naming convention as `res/`)
- `metamodule-plugin-sdk/docs/` — Key reference docs: `tips.md`, `async-threads.md`, `rack-adaptor.md`

## MetaModule Porting Rules

### Lights and Buttons
MetaModule renders params and lights in separate layers. A light added as a sibling to a button will render *beneath* it. To get a light that shows through a button:

- Use `VCVLightBezel<TLightBase>` as the button base class — it embeds the light as a child.
- Set the light's `firstLightId` via `createLightParamCentered<MyWidget>(pos, module, PARAM_ID, FIRST_LIGHT_ID)`.
- Register the pair with `addLightSwitch(widget, widget->getLight())` under `#ifdef METAMODULE` (VCV Rack uses a plain `addParam`).

```cpp
// Correct pattern (see Kleztizer.cpp for reference, Tehom.cpp for multi-color):
auto* btn = createLightParamCentered<VCVLightBezel<WhiteLight>>(pos, module, PARAM_ID, LIGHT_ID);
#ifdef METAMODULE
addLightSwitch(btn, btn->getLight());
#else
addParam(btn);
#endif
```

**Never** pass a separately-created light widget to `addLightSwitch` — it must be embedded in (a child of) the button.

### Multi-color lights on buttons
Define a custom light type with multiple `addBaseColor()` calls, then use it as the `VCVLightBezel` template argument. Light IDs **must be consecutive** — interleave enum entries if you have multiple channels:

```cpp
struct GreenBlueLight : GrayModuleLightWidget {
    GreenBlueLight() { addBaseColor(SCHEME_GREEN); addBaseColor(SCHEME_BLUE); }
};

// Enum: PLAY1_LIGHT, PLAY1_BLUE_LIGHT, PLAY2_LIGHT, PLAY2_BLUE_LIGHT, ...
// Access: lights[PLAY1_LIGHT + i*2] and lights[PLAY1_BLUE_LIGHT + i*2]
```

### Volume / Level LEDs
Drive level LEDs from signal sources that are gated by the active/playing state (e.g. `sampL`/`sampR` in Tehom, which are multiplied by `playGain`). Do **not** use crossfade mix outputs that include live input passthrough — those stay non-zero even when a channel is stopped.

### Widget draw order
`addChild` appends to the end of the child list (drawn last = on top). `addChildBottom` prepends (drawn first = behind everything else). Use `addChildBottom` for background elements that should never obscure interactive widgets or LEDs.

### No allocations in `process()`
MetaModule has no heap allocator available on the audio thread. All buffers must be pre-allocated in the module constructor or as member arrays. Use `AsyncThread` (from the MetaModule SDK) for any file I/O or heavy work:

```cpp
// See Tehom.cpp for a complete AsyncThread example (WAV file streaming)
```

### SVG loading
`APP->window->loadSvg()` (used in some `plugin.hpp` toggle switch constructors) is a VCV Rack API. If those switches are used on MetaModule, wrap in `#ifndef METAMODULE` or switch to `Svg::load()`.

## Key Reference: Kleztizer.cpp
The cleanest example of correct MetaModule-compatible light-param usage. When in doubt about how to wire up buttons with lights, check that file first.
