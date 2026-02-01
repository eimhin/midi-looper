/*
 * MIDI Looper - distingNT Plugin
 *
 * A MIDI looper plugin for the disting NT.
 */

#include <distingnt/api.h>
#include <cstring>
#include <new>

// ============================================================================
// CONSTANTS
// ============================================================================

// MIDI status bytes
static constexpr uint8_t kMidiNoteOff = 0x80;
static constexpr uint8_t kMidiNoteOn = 0x90;

// Note names for display
static const char* const noteNames[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Channel filter options
static const char* const channelNames[] = {
    "All", "1", "2", "3", "4", "5", "6", "7", "8",
    "9", "10", "11", "12", "13", "14", "15", "16", NULL
};

// Display mode options
static const char* const displayModeNames[] = {
    "Note Name", "Note Number", NULL
};

// ============================================================================
// PARAMETERS
// ============================================================================

enum {
    // Main page parameters
    kParamMidiChannel,      // Channel filter (0=all, 1-16=specific)
    
    // Settings page parameters
    kParamDisplayMode,      // 0=note name, 1=note number
    
    kNumParameters
};

static const _NT_parameter parameters[] = {
    // Main page: MIDI channel filter
    { .name = "MIDI Channel", .min = 0, .max = 16, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = channelNames },
    
    // Settings page: Display mode
    { .name = "Display Mode", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = displayModeNames },
};

// ============================================================================
// PARAMETER PAGES WITH GROUPS
// ============================================================================

// Page 1: Main controls (group 1)
static const uint8_t pageMain[] = { kParamMidiChannel };

// Page 2: Settings (group 2)
static const uint8_t pageSettings[] = { kParamDisplayMode };

// Page definitions with groups
// The .group field creates visual groupings in the UI
static const _NT_parameterPage pages[] = {
    { .name = "Main", .numParams = ARRAY_SIZE(pageMain), .group = 1, .params = pageMain },
    { .name = "Settings", .numParams = ARRAY_SIZE(pageSettings), .group = 2, .params = pageSettings },
};

static const _NT_parameterPages parameterPages = {
    .numPages = ARRAY_SIZE(pages),
    .pages = pages,
};

// ============================================================================
// ALGORITHM DATA STRUCTURES
// ============================================================================

// DTC (Data Tightly Coupled) - Fast access data for step()
struct _midilooper_DTC {
    // Last received MIDI note info
    uint8_t lastNote;       // 0-127
    uint8_t lastVelocity;   // 0-127
    uint8_t lastChannel;    // 0-15 (displayed as 1-16)
    bool gateOn;            // True if note is currently on
    
    // Activity indicator
    uint8_t activityCounter; // Decrements each step for visual feedback
};

// Main algorithm structure
struct _midilooperAlgorithm : public _NT_algorithm {
    _midilooperAlgorithm(_midilooper_DTC* dtc_) : dtc(dtc_) {}
    ~_midilooperAlgorithm() {}
    
    _midilooper_DTC* dtc;
};

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

// Calculate memory requirements for the plugin
void calculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
    req.numParameters = ARRAY_SIZE(parameters);
    req.sram = sizeof(_midilooperAlgorithm);
    req.dram = 0;  // No large buffers needed
    req.dtc = sizeof(_midilooper_DTC);
    req.itc = 0;
}

// Construct the algorithm instance
_NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs,
                         const _NT_algorithmRequirements& req,
                         const int32_t* specifications) {
    _midilooper_DTC* dtc = (_midilooper_DTC*)ptrs.dtc;
    
    // Initialize DTC
    memset(dtc, 0, sizeof(_midilooper_DTC));
    dtc->lastNote = 60;      // Middle C
    dtc->lastVelocity = 0;
    dtc->lastChannel = 0;
    dtc->gateOn = false;
    dtc->activityCounter = 0;
    
    // Construct algorithm in provided SRAM
    _midilooperAlgorithm* pThis = new (ptrs.sram) _midilooperAlgorithm(dtc);

    // Set up parameters and parameter pages
    pThis->parameters = parameters;
    pThis->parameterPages = &parameterPages;

    return pThis;
}

// Handle parameter changes
void parameterChanged(_NT_algorithm* self, int p) {
    // Parameters are read directly from self->v[] in step() and draw()
    // No caching needed for this simple plugin
}

// Audio/CV processing step (called every block)
void step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    _midilooperAlgorithm* pThis = (_midilooperAlgorithm*)self;
    _midilooper_DTC* dtc = pThis->dtc;
    
    // Decrement activity counter for visual feedback decay
    if (dtc->activityCounter > 0) {
        dtc->activityCounter--;
    }
    
    // This plugin doesn't process audio - it just monitors MIDI
    // If you wanted to output CV based on MIDI, you would do it here
}

// ============================================================================
// MIDI HANDLING
// ============================================================================

// Called for each incoming MIDI message
// byte0 = status byte (message type + channel)
// byte1 = first data byte (note number for note on/off)
// byte2 = second data byte (velocity for note on/off)
void midiMessage(_NT_algorithm* self, uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    _midilooperAlgorithm* pThis = (_midilooperAlgorithm*)self;
    _midilooper_DTC* dtc = pThis->dtc;
    
    // Extract status (upper nibble) and channel (lower nibble)
    uint8_t status = byte0 & 0xF0;
    uint8_t channel = byte0 & 0x0F;
    
    // Get channel filter setting (0 = all channels)
    int channelFilter = pThis->v[kParamMidiChannel];
    
    // Check if we should process this channel
    // channelFilter 0 = all, 1-16 = specific channel (stored as 0-15 internally)
    if (channelFilter != 0 && channel != (channelFilter - 1)) {
        return;  // Skip messages not matching our filter
    }
    
    // Handle note on/off messages
    if (status == kMidiNoteOn) {
        uint8_t note = byte1;
        uint8_t velocity = byte2;
        
        if (velocity > 0) {
            // Note on
            dtc->lastNote = note;
            dtc->lastVelocity = velocity;
            dtc->lastChannel = channel;
            dtc->gateOn = true;
            dtc->activityCounter = 30;  // Flash activity indicator
        } else {
            // Note on with velocity 0 = note off
            if (note == dtc->lastNote) {
                dtc->gateOn = false;
            }
        }
    }
    else if (status == kMidiNoteOff) {
        uint8_t note = byte1;
        
        // Only turn off gate if it's the same note
        if (note == dtc->lastNote) {
            dtc->gateOn = false;
        }
    }
}

// ============================================================================
// CUSTOM UI
// ============================================================================

// Draw custom UI - called when screen needs updating
// Returns true to hide standard parameter display
bool draw(_NT_algorithm* self) {
    _midilooperAlgorithm* pThis = (_midilooperAlgorithm*)self;
    _midilooper_DTC* dtc = pThis->dtc;
    
    // Title
    NT_drawText(10, 8, "MIDI LOOPER", 15, kNT_textLeft, kNT_textNormal);
    
    // Get display mode setting
    int displayMode = pThis->v[kParamDisplayMode];
    
    // Build note display string
    char noteStr[16];
    if (displayMode == 0) {
        // Note name mode: "C4", "F#5", etc.
        int noteName = dtc->lastNote % 12;
        int octave = (dtc->lastNote / 12) - 1;  // MIDI octave convention
        // Build string manually: copy note name, then append octave
        const char* name = noteNames[noteName];
        int i = 0;
        while (*name) noteStr[i++] = *name++;
        // Convert octave to string (handles -1 to 9)
        if (octave < 0) {
            noteStr[i++] = '-';
            octave = -octave;
        }
        noteStr[i++] = '0' + octave;
        noteStr[i] = '\0';
    } else {
        // Note number mode: "60", "127", etc.
        NT_intToString(noteStr, dtc->lastNote);
    }
    
    // Draw note (large, centered)
    NT_drawText(128, 24, noteStr, 15, kNT_textCentre, kNT_textLarge);
    
    // Draw velocity bar
    NT_drawText(10, 44, "Vel:", 10, kNT_textLeft, kNT_textTiny);
    int velBarWidth = (dtc->lastVelocity * 180) / 127;
    NT_drawShapeI(kNT_box, 40, 42, 220, 50, 8);  // Outline
    if (velBarWidth > 0) {
        NT_drawShapeI(kNT_rectangle, 41, 43, 41 + velBarWidth, 49, 
                      dtc->gateOn ? 15 : 10);  // Filled bar (brighter when gate on)
    }
    
    // Draw velocity number
    char velStr[8];
    NT_intToString(velStr, dtc->lastVelocity);
    NT_drawText(225, 44, velStr, 12, kNT_textLeft, kNT_textTiny);
    
    // Draw channel
    char chanStr[16];
    chanStr[0] = 'C'; chanStr[1] = 'h'; chanStr[2] = ':'; chanStr[3] = ' ';
    NT_intToString(chanStr + 4, dtc->lastChannel + 1);
    NT_drawText(10, 56, chanStr, 12, kNT_textLeft, kNT_textTiny);
    
    // Gate indicator
    if (dtc->gateOn) {
        NT_drawShapeI(kNT_rectangle, 240, 56, 250, 66, 15);  // Filled square when on
        NT_drawText(200, 58, "GATE", 15, kNT_textLeft, kNT_textTiny);
    } else {
        NT_drawShapeI(kNT_box, 240, 56, 250, 66, 8);  // Empty box when off
        NT_drawText(200, 58, "gate", 8, kNT_textLeft, kNT_textTiny);
    }
    
    // Activity flash
    if (dtc->activityCounter > 0) {
        int brightness = (dtc->activityCounter > 15) ? 15 : dtc->activityCounter;
        NT_drawShapeI(kNT_rectangle, 5, 5, 8, 15, brightness);
    }
    
    return true;  // Hide standard parameter line
}

// ============================================================================
// FACTORY DEFINITION
// ============================================================================

static const _NT_factory factory = {
    .guid = NT_MULTICHAR('M', 'i', 'L', 'p'),  // Unique 4-char identifier
    .name = "MIDI Looper",
    .description = "MIDI looper for recording and playback",
    .numSpecifications = 0,
    .specifications = NULL,
    .calculateStaticRequirements = NULL,
    .initialise = NULL,
    .calculateRequirements = calculateRequirements,
    .construct = construct,
    .parameterChanged = parameterChanged,
    .step = step,
    .draw = draw,
    .midiRealtime = NULL,
    .midiMessage = midiMessage,  // Register MIDI callback
    .tags = kNT_tagUtility,
    .hasCustomUi = NULL,
    .customUi = NULL,
    .setupUi = NULL,
};

// ============================================================================
// PLUGIN ENTRY POINT
// ============================================================================

// Main entry point called by distingNT to discover plugin factories
uintptr_t pluginEntry(_NT_selector selector, uint32_t data) {
    switch (selector) {
        case kNT_selector_version:
            return kNT_apiVersion9;
        case kNT_selector_numFactories:
            return 1;  // This plugin provides 1 algorithm
        case kNT_selector_factoryInfo:
            return (uintptr_t)((data == 0) ? &factory : NULL);
    }
    return 0;
}
