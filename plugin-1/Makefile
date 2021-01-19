RACK_DIR ?= ../../..

INCS := $(foreach inc,$(PLUGIN_1_INCLUDES),-I../$(inc))
INCS += -I./src/third-party -I.src/ctrl -I./src/kbd

FLAGS += $(INCS)
$(info PLUGIN_1_INCLUDES="$(PLUGIN_1_INCLUDES)")
$(info INCS="$(INCS)")
$(info FLAGS="$(FLAGS)")

SRCS = $(foreach src,$(PLUGIN_1_COMMON_SOURCES),../$(src))
SOURCES += $(SRCS)
SOURCES += $(wildcard src/third-party/*.cpp)
SOURCES += $(wildcard src/seq/*.cpp)
SOURCES += $(wildcard src/kbd/*.cpp)
$(info PLUGIN_1_COMMON_SOURCES="$(PLUGIN_1_COMMON_SOURCES)")
$(info SRCS="$(SRCS)")
$(info SOURCES="$(SOURCES)")

# include res and presets folder
DISTRIBUTABLES += $(wildcard ../LICENSE*) res presets

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
