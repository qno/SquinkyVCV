RACK_DIR ?= ../../..

INCLUDES =  ../dsp/generators ../dsp/utils ../dsp/filters
INCLUDES += ../dsp/third-party/falco ../dsp/third-party/kiss_fft130 
INCLUDES += ../dsp/third-party/kiss_fft130/tools ../dsp/third-party/src ../dsp/third-party/midifile
INCLUDES += ../dsp dsp/samp ../dsp/third-party/pugixml
INCLUDES += ../sqsrc/thread ../dsp/fft ../composites
INCLUDES += ../sqsrc/noise ../sqsrc/util ../sqsrc/clock ../sqsrc/grammar ../sqsrc/delay
INCLUDES += ../midi/model ../midi/view ../midi/controller ../util
INCLUDES += src/third-party src/ctrl src/kbd

SRCS =  $(wildcard ../dsp/**/*.cpp)
SRCS += $(wildcard ../dsp/third-party/falco/*.cpp)
SRCS += $(wildcard ../dsp/third-party/midifile/*.cpp)
SRCS += ../dsp/third-party/kiss_fft130/kiss_fft.c
SRCS += ../dsp/third-party/kiss_fft130/tools/kiss_fftr.c
SRCS += $(wildcard ../sqsrc/**/*.cpp)
SRCS += $(wildcard ../midi/**/*.cpp)
SRCS += $(wildcard src/third-party/*.cpp)
SRCS += $(wildcard src/seq/*.cpp)
SRCS += $(wildcard src/kbd/*.cpp)

FLAGS +=  $(foreach INCLUDE,$(INCLUDES),-I$(INCLUDE))
SOURCES = $(foreach SRC,$(SRCS),$(realpath $(SRC)))

$(info FLAGS="$(FLAGS)")
$(info SOURCES="$(SOURCES)")

# include res and presets folder
DISTRIBUTABLES += $(wildcard ../LICENSE*) res presets

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

$(TARGET) : FLAGS += -D __PLUGIN

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
