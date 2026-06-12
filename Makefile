# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=
# -std=c++17 must be C++-only (the C file stb_vorbis.c can't take it) AND applied
# after Rack's own -std=c++11. EXTRA_CXXFLAGS is appended to CXXFLAGS after c++11,
# so c++17 wins; it is never added to CFLAGS. (see Rack-SDK compile.mk)
EXTRA_CXXFLAGS += -std=c++17

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += src/stb_vorbis.c

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
