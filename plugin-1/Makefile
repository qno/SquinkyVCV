RACK_DIR ?= ../../..

INCLUDES = ../dsp/generators ../dsp/utils ../dsp/filters
INCLUDES += ../dsp/third-party/falco ../dsp/third-party/kiss_fft130 
INCLUDES += ../dsp/third-party/kiss_fft130/tools ../dsp/third-party/src ../dsp/third-party/midifile
INCLUDES += ../dsp dsp/samp ../dsp/third-party/pugixml
INCLUDES += ../sqsrc/thread ../dsp/fft ../composites
INCLUDES += ../sqsrc/noise ../sqsrc/util ../sqsrc/clock ../sqsrc/grammar ../sqsrc/delay
INCLUDES += ../midi/model ../midi/view ../midi/controller ../util
INCLUDES += src/third-party src/ctrl src/kbd

SOURCES = $(wildcard ../dsp/**/*.cpp)
SOURCES += $(wildcard ../dsp/third-party/falco/*.cpp)
SOURCES += $(wildcard ../dsp/third-party/midifile/*.cpp)
SOURCES += ../dsp/third-party/kiss_fft130/kiss_fft.c
SOURCES += ../dsp/third-party/kiss_fft130/tools/kiss_fftr.c
SOURCES += $(wildcard ../sqsrc/**/*.cpp)
SOURCES += $(wildcard ../midi/**/*.cpp)
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)
SOURCES += $(wildcard src/kbd/*.cpp)

FLAGS += $(foreach INCLUDE,$(PLUGIN_1_INCLUDES),-I$(INCLUDE))
$(info FLAGS="$(FLAGS)")
$(info SOURCES="$(SOURCES)")

# include res and presets folder
DISTRIBUTABLES += $(wildcard ../LICENSE*) res presets

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

$(TARGET) : FLAGS += -D __PLUGIN

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
