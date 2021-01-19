RACK_DIR ?= ../../..
FLAGS += $(PLUGIN_1_INCLUDES)

# Add .cpp and .c files to the build
SOURCES += $(PLUGIN_1_COMMON_SOURCES)
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)
SOURCES += $(wildcard src/kbd/*.cpp)

# include res and presets folder
DISTRIBUTABLES += $(wildcard ../LICENSE*) res presets

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

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
