CC ?= cc
.DEFAULT_GOAL := all
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Werror -MMD -MP
FP_CFLAGS = -frounding-math -ffp-contract=off
LDLIBS += -lm
CPPFLAGS ?= -Isrc
CLANG ?= clang
ifeq ($(OS),Windows_NT)
LDLIBS += -lws2_32
endif
LD_LLD ?= ld.lld
LLVM_OBJCOPY ?= llvm-objcopy
PYTHON ?= python3

SDL2_CFLAGS := $(shell (sdl2-config --cflags 2>/dev/null || pkg-config --cflags sdl2 2>/dev/null) | head -1)
SDL2_LIBS := $(shell (sdl2-config --libs 2>/dev/null || pkg-config --libs sdl2 2>/dev/null) | head -1)
ifneq ($(strip $(SDL2_LIBS)),)
CPPFLAGS += -DR3K_HAVE_SDL2 $(SDL2_CFLAGS)
LDLIBS += $(SDL2_LIBS)
endif

DEPS = $(HOST_OBJS:.o=.d)

-include $(DEPS)

.PHONY: all mips-tool check-mips-tool
all: mips-tool


mips-tool: src/mips_tool.c src/r3000.c src/r3010.c src/jit.c src/jit.h
	$(CC) $(CFLAGS) -Isrc -o $@ src/mips_tool.c src/r3000.c src/r3010.c src/jit.c $(LDLIBS)

check-mips-tool: mips-tool
	./mips-tool --version
	./mips-tool --help
