# Makefile for MIDI Looper distingNT Plugin

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
         -fno-exceptions
INCLUDES = -I. -I./src -I./distingNT_API/include
LDFLAGS = -Wl,--relocatable -nostdlib
OUTPUT_DIR = plugins
BUILD_DIR = build
OUTPUT = $(OUTPUT_DIR)/$(PLUGIN_NAME).o
OBJECTS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))
DEV ?= 0

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@mkdir -p $(OUTPUT_DIR)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Built hardware plugin: $@"

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

hardware: all

push: hardware
	ntpush $(DEV) $(OUTPUT_DIR)/$(PLUGIN_NAME).o

check: $(OUTPUT)
	@echo "Checking symbols in $(OUTPUT)..."
	@arm-none-eabi-nm $(OUTPUT) | grep ' U ' || true

size: $(OUTPUT)
	@echo "Size of $(OUTPUT):"
	@arm-none-eabi-size $(OUTPUT)

compile_commands.json: $(SOURCES) Makefile
	@echo '[' > $@
	@sep=""; \
	for src in $(SOURCES); do \
		printf '%s\n  { "directory": "%s", "file": "%s", "arguments": ["clang++", "-std=c++11", "-fPIC", "-Os", "-Wall", "-fno-rtti", "-fno-exceptions", "-I.", "-I./src", "-I./distingNT_API/include", "-c", "%s"] }' \
			"$$sep" "$(CURDIR)" "$$src" "$$src" >> $@; \
		sep=","; \
	done
	@echo '' >> $@
	@echo ']' >> $@
	@echo "Generated $@"

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)
	@echo "Cleaned build and output directories"

.PHONY: all hardware push check size clean compile_commands.json
