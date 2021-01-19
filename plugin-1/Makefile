RACK_DIR ?= ../../..

INCLUDES := $(foreach INCLUDE,$(PLUGIN_1_INCLUDES),-I../$(INCLUDE))
INCLUDES += -I./src/third-party -I./src/ctrl -I./src/kbd

FLAGS += $(INCLUDES)
$(info PLUGIN_1_INCLUDES="$(PLUGIN_1_INCLUDES)")
$(info FLAGS="$(FLAGS)")

SOURCES += $(foreach SRC,$(PLUGIN_1_COMMON_SOURCES),../$(SRC))
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)
SOURCES += $(wildcard src/kbd/*.cpp)
$(info PLUGIN_1_COMMON_SOURCES="$(PLUGIN_1_COMMON_SOURCES)")
$(info SOURCES="$(SOURCES)")

# include res and presets folder
DISTRIBUTABLES += $(wildcard ../LICENSE*) res presets

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
