# ==============================================================
# Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2020.1 (64-bit)
# Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
# ==============================================================

# Taken from myproject_prj/solution1/csim/build/csim.mk
CSIM_DESIGN = 1

HLS_SOURCES = $(wildcard *.cpp)

TARGET = csim.exe

ObjDir = obj

### Paths
AUTOPILOT_ROOT := /data/Xilinx/Vivado/2020.1
AUTOPILOT_MACH := lnx64
AUTOPILOT_TOOL := ${AUTOPILOT_ROOT}/${AUTOPILOT_MACH}/tools
AUTOPILOT_TECH := ${AUTOPILOT_ROOT}/common/technology

AP_GCC_PATH := /data/Xilinx/Vivado/2020.1/tps/lnx64/gcc-6.2.0/bin

### Flags
DEBUG = -g -D_DEBUG
DEFINE =

CFLAG += $(DEBUG) $(DEFINE)
IFLAG += -fPIC
IFLAG += -I .
IFLAG += -I "${AUTOPILOT_ROOT}/include"
IFLAG += -Wno-unknown-pragmas -std=c++11 -Wall -Wextra -Wno-unused-label -Wno-unused-parameter
IFLAG += -g
LFLAG += -Wl,--as-needed

### Commands
Echo = @echo
MKDIR = mkdir -p
Make = make

ifdef VERBOSE
  Verb :=
else
  Verb := @
endif

### Rules
OBJECTS := $(basename $(notdir $(HLS_SOURCES)))
OBJECTS := $(OBJECTS:%=$(ObjDir)/%.o)

CC = $(AP_GCC_PATH)/gcc
CXX = $(AP_GCC_PATH)/g++

CXXFLAGS = $(IFLAG) $(CFLAG)
LDFLAGS = $(LFLAG)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(Echo) "   Generating $(TARGET)"
	$(Verb)$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

$(ObjDir)/%.o: %.cpp
	$(Echo) "   Compiling $*.cpp"
	$(Verb)$(MKDIR) $(ObjDir)
	$(Verb)$(CXX) ${CXXFLAGS} -c $< -o $@
