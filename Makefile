# Makefile for MIDI Looper distingNT Plugin
#
# Usage:
#   make hardware    - Build for distingNT hardware (.o file)
#   make test        - Build for nt_emu testing (.dylib/.so/.dll)
#   make both        - Build both targets
#   make clean       - Remove all build artifacts

# ============================================================================
# PROJECT CONFIGURATION
# ============================================================================

PLUGIN_NAME = midilooper
SOURCES = midilooper.cpp \
          src/quantize.cpp \
          src/midi.cpp \
          src/directions.cpp \
          src/modifiers.cpp \
          src/recording.cpp \
          src/playback.cpp \
          src/generate.cpp \
          src/ui.cpp \
          src/serial.cpp

# Detect platform
UNAME_S := $(shell uname -s)

# Target selection (hardware or test)
TARGET ?= hardware

# ============================================================================
# HARDWARE BUILD (ARM Cortex-M7 for distingNT)
# ============================================================================
ifeq ($(TARGET),hardware)
    CXX = arm-none-eabi-g++
    CFLAGS = -std=c++11 \
             -mcpu=cortex-m7 \
             -mfpu=fpv5-d16 \
             -mfloat-abi=hard \
             -mthumb \
             -Os \
             -Wall \
             -fPIC \
             -fno-rtti \
             -fno-exceptions \
             -DDISTING_HARDWARE
    INCLUDES = -I. -I./src -I./distingNT_API/include
    LDFLAGS = -Wl,--relocatable -nostdlib
    OUTPUT_DIR = plugins
    BUILD_DIR = build
    OUTPUT = $(OUTPUT_DIR)/$(PLUGIN_NAME).o
    OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
    CHECK_CMD = arm-none-eabi-nm $(OUTPUT) | grep ' U '
    SIZE_CMD = arm-none-eabi-size $(OUTPUT)

# ============================================================================
# DESKTOP BUILD (Native for nt_emu VCV Rack testing)
# ============================================================================
else ifeq ($(TARGET),test)
    # macOS
    ifeq ($(UNAME_S),Darwin)
        CXX = clang++
        CFLAGS = -std=c++11 -fPIC -Os -Wall -fno-rtti -fno-exceptions
        LDFLAGS = -dynamiclib -undefined dynamic_lookup
        EXT = dylib
    endif

    # Linux
    ifeq ($(UNAME_S),Linux)
        CXX = g++
        CFLAGS = -std=c++11 -fPIC -Os -Wall -fno-rtti -fno-exceptions
        LDFLAGS = -shared
        EXT = so
    endif

    # Windows (MinGW)
    ifeq ($(OS),Windows_NT)
        CXX = g++
        CFLAGS = -std=c++11 -fPIC -Os -Wall -fno-rtti -fno-exceptions
        LDFLAGS = -shared
        EXT = dll
    endif

    INCLUDES = -I. -I./src -I./distingNT_API/include
    OUTPUT_DIR = plugins
    OUTPUT = $(OUTPUT_DIR)/$(PLUGIN_NAME).$(EXT)
    CHECK_CMD = nm $(OUTPUT) | grep ' U ' || echo "No undefined symbols"
    SIZE_CMD = ls -lh $(OUTPUT)
endif

# ============================================================================
# BUILD RULES
# ============================================================================

all: $(OUTPUT)

# Hardware build with object files
ifeq ($(TARGET),hardware)
$(OUTPUT): $(OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built hardware plugin: $@"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Test build (direct linking)
else ifeq ($(TARGET),test)
$(OUTPUT): $(SOURCES)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@ $(SOURCES)
	@echo "Built test plugin: $@"
endif

# ============================================================================
# CONVENIENCE TARGETS
# ============================================================================

hardware:
	@$(MAKE) TARGET=hardware

push: hardware
	ntpush $(OUTPUT_DIR)/$(PLUGIN_NAME).o

test:
	@$(MAKE) TARGET=test

both: hardware test

check: $(OUTPUT)
	@echo "Checking symbols in $(OUTPUT)..."
	@$(CHECK_CMD) || true

size: $(OUTPUT)
	@echo "Size of $(OUTPUT):"
	@$(SIZE_CMD)

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)
	@echo "Cleaned build and output directories"

help:
	@echo "MIDI Looper - distingNT Plugin Build System"
	@echo "============================================"
	@echo ""
	@echo "Targets:"
	@echo "  hardware    - Build for distingNT hardware (.o)"
	@echo "  test        - Build for nt_emu testing (.dylib/.so/.dll)"
	@echo "  both        - Build both targets"
	@echo "  push        - Build and push to distingNT via MIDI"
	@echo "  check       - Check undefined symbols"
	@echo "  size        - Show plugin size"
	@echo "  clean       - Remove build artifacts"
	@echo ""
	@echo "Testing Workflow:"
	@echo "  1. make test              # Build for nt_emu"
	@echo "  2. Copy to ~/Documents/nt_emu/plugins/"
	@echo "  3. Load in VCV Rack with nt_emu module"
	@echo ""
	@echo "Deployment:"
	@echo "  make push                 # Build and push via MIDI"
	@echo "  -- or --"
	@echo "  1. make hardware"
	@echo "  2. Copy plugins/$(PLUGIN_NAME).o to distingNT SD card"

.PHONY: all hardware test both push check size clean help
