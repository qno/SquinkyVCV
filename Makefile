RACK_DIR ?= ../..
include $(RACK_DIR)/arch.mk

CFLAGS +=
CXXFLAGS +=

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

# mac does not like this argument
ifdef ARCH_WIN
	FLAGS += -fmax-errors=5
endif

#  -flto
FLAGS += -finline-limit=500000 -finline-functions-called-once -flto
LDFLAGS += -flto

export CFLAGS
export CXXFLAGS
export FLAGS
export LDFLAGS
export ASSERTOFF

dep:
	$(MAKE) -C plugin-1 dep

install:
	$(MAKE) -C plugin-1 install

dist:
	$(MAKE) -C plugin-1 dist

include test.mk
