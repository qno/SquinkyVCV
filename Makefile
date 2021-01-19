CFLAGS +=
CXXFLAGS +=

PLUGIN_1_INCLUDES = -I./dsp/generators -I./dsp/utils -I./dsp/filters
PLUGIN_1_INCLUDES += -I./dsp/third-party/falco -I./dsp/third-party/kiss_fft130 
PLUGIN_1_INCLUDES += -I./dsp/third-party/kiss_fft130/tools -I./dsp/third-party/src -I./dsp/third-party/midifile
PLUGIN_1_INCLUDES += -I./dsp -I./dsp/samp -I./dsp/third-party/pugixml
PLUGIN_1_INCLUDES += -I./sqsrc/thread -I./dsp/fft -I./composites
PLUGIN_1_INCLUDES += -I./sqsrc/noise -I./sqsrc/util -I./sqsrc/clock -I./sqsrc/grammar -I./sqsrc/delay
PLUGIN_1_INCLUDES += -I./midi/model -I./midi/view -I./midi/controller -I./util
PLUGIN_1_INCLUDES += -I./src/third-party -I.src/ctrl -I./src/kbd

PLUGIN_1_COMMON_SOURCES = $(wildcard dsp/**/*.cpp)
PLUGIN_1_COMMON_SOURCES += $(wildcard dsp/third-party/falco/*.cpp)
PLUGIN_1_COMMON_SOURCES += $(wildcard dsp/third-party/midifile/*.cpp)
PLUGIN_1_COMMON_SOURCES += dsp/third-party/kiss_fft130/kiss_fft.c
PLUGIN_1_COMMON_SOURCES += dsp/third-party/kiss_fft130/tools/kiss_fftr.c
PLUGIN_1_COMMON_SOURCES += $(wildcard sqsrc/**/*.cpp)
PLUGIN_1_COMMON_SOURCES += $(wildcard midi/**/*.cpp)

export PLUGIN_1_INCLUDES
export PLUGIN_1_COMMON_SOURCES

# compile for V1 vs 0.6
FLAGS += -D __V1x
FLAGS += -D _SEQ

# Command line variable to turn on "experimental" modules
ifdef _EXP
	FLAGS += -D _EXP
endif

# Macro to use on any target where we don't normally want asserts
ASSERTOFF = -D NDEBUG

# Make _ASSERT=true will nullify our ASSERTOFF flag, thus allowing them
ifdef _ASSERT
	ASSERTOFF =
endif

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine.
LDFLAGS += -lpthread

# This turns asserts off for make (plugin), not for test or perf
$(TARGET) :  FLAGS += $(ASSERTOFF)

$(TARGET) : FLAGS += -D __PLUGIN

# mac does not like this argument
ifdef ARCH_WIN
	FLAGS += -fmax-errors=5
endif

#  -flto
FLAGS += -finline-limit=500000 -finline-functions-called-once -flto
LDFLAGS += -flto

dep:
	$(MAKE) -C plugin-1 dep

install:
	$(MAKE) -C plugin-1 install

dist:
	$(MAKE) -C plugin-1 dist

include test.mk
