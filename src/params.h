/*
 * MIDI Looper - Parameter Definitions
 * Parameter arrays and page definitions for the distingNT UI
 */

#pragma once

#include "config.h"
#include "types.h"
#include <cstddef> // for NULL

// ============================================================================
// PARAMETER STRING ARRAYS
// ============================================================================

static const char* const recordStrings[] = {"Off", "On", NULL};
static const char* const recTrackStrings[] = {"1", "2", "3", "4", NULL};
static const char* const recModeStrings[] = {"Replace", "Overdub", "Step", NULL};
static const char* const midiDestStrings[] = {"Breakout", "SelectBus", "USB", "Internal", "All", NULL};
static const char* const noYesStrings[] = {"No", "Yes", NULL};
static const char* const divisionStrings[] = {"1", "2", "4", "8", "16", NULL};
static const char* const directionStrings[] = {"Forward",  "Reverse", "Pendulum", "Ping-Pong", "Stride",  "Odd/Even", "Hopscotch",
                                               "Converge", "Diverge", "Brownian", "Random",    "Shuffle", NULL};
static const char* const stepMaskStrings[] = {"All", "Odds", "Evens", "1st Half", "2nd Half", "Sparse", "Dense", "Random", NULL};
static const char* const scaleRootStrings[] = {"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B", NULL};
static const char* const scaleTypeStrings[] = {"Off",     "Ionian",   "Dorian",   "Phrygian",  "Lydian",    "Mixolydian", "Aeolian",
                                               "Locrian", "Harm Min", "Melo Min", "Maj Penta", "Min Penta", NULL};
static const char* const genModeStrings[] = {"New", "Reorder", "Re-pitch", "Invert", NULL};
// clang-format off
static const char* const trigCondStrings[] = {
    "Always",
    "1:2", "2:2",
    "1:3", "2:3", "3:3",
    "1:4", "2:4", "3:4", "4:4",
    "1:5", "2:5", "3:5", "4:5", "5:5",
    "1:6", "2:6", "3:6", "4:6", "5:6", "6:6",
    "1:7", "2:7", "3:7", "4:7", "5:7", "6:7", "7:7",
    "1:8", "2:8", "3:8", "4:8", "5:8", "6:8", "7:8", "8:8",
    "!1:2", "!2:2",
    "!1:3", "!2:3", "!3:3",
    "!1:4", "!2:4", "!3:4", "!4:4",
    "!1:5", "!2:5", "!3:5", "!4:5", "!5:5",
    "!1:6", "!2:6", "!3:6", "!4:6", "!5:6", "!6:6",
    "!1:7", "!2:7", "!3:7", "!4:7", "!5:7", "!6:7", "!7:7",
    "!1:8", "!2:8", "!3:8", "!4:8", "!5:8", "!6:8", "!7:8", "!8:8",
    "First", "!First", "Fill", "!Fill", "Fixed", NULL
};
// clang-format on

// ============================================================================
// PARAMETER DEFINITIONS
// ============================================================================

// Track parameter macro - generates 31 parameters per track
// DEF_ENABLED: default for Enabled (1 for track 1, 0 for others)
// DEF_CHANNEL: default MIDI channel (1-4 for tracks 1-4)
// clang-format off
#define TRACK_PARAMS(DEF_ENABLED, DEF_CHANNEL) \
    {.name = "Enabled", .min = 0, .max = 1, .def = DEF_ENABLED, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings}, \
    {.name = "Length", .min = 1, .max = MAX_STEPS, .def = 16, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Division", .min = 1, .max = 16, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Direction", .min = 0, .max = 11, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = directionStrings}, \
    {.name = "Stride Size", .min = 2, .max = 16, .def = 2, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Velocity", .min = -64, .max = 64, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Humanize", .min = 0, .max = 100, .def = 0, .unit = kNT_unitMs, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Channel", .min = 1, .max = 16, .def = DEF_CHANNEL, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Destination", .min = 0, .max = 4, .def = 3, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = midiDestStrings}, \
    {.name = "Stability", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Motion", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Randomness", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Gravity", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Anchor", .min = 1, .max = MAX_STEPS, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Pedal", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Pedal Step", .min = 1, .max = MAX_STEPS, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "No Repeat", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings}, \
    {.name = "Step Mask", .min = 0, .max = 7, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = stepMaskStrings}, \
    {.name = "Oct Up", .min = 0, .max = 4, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Oct Down", .min = 0, .max = 4, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Oct Prob", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Oct Bypass", .min = 0, .max = 64, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Bypass Offset", .min = -24, .max = 24, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Step Prob", .min = 0, .max = 100, .def = 100, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Step Cond", .min = 0, .max = 75, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = trigCondStrings}, \
    {.name = "Cond Stp A", .min = 0, .max = MAX_STEPS, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Cond A", .min = 0, .max = 75, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = trigCondStrings}, \
    {.name = "Prob A", .min = 0, .max = 100, .def = 100, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Cond Stp B", .min = 0, .max = MAX_STEPS, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL}, \
    {.name = "Cond B", .min = 0, .max = 75, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = trigCondStrings}, \
    {.name = "Prob B", .min = 0, .max = 100, .def = 100, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
// clang-format on

// Parameter definitions
static const _NT_parameter parameters[] = {
    // Routing parameters (0-1)
    NT_PARAMETER_CV_INPUT("Run", 0, 1)   // default bus 1, 0 = none allowed
    NT_PARAMETER_CV_INPUT("Clock", 0, 2) // default bus 2, 0 = none allowed

    // Global parameters (2-10)
    {.name = "Record", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recordStrings},
    {.name = "Rec Track", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recTrackStrings},
    {.name = "Division", .min = 0, .max = 4, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = divisionStrings},
    {.name = "Rec Mode", .min = 0, .max = 2, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recModeStrings},
    {.name = "Rec Snap", .min = 50, .max = 100, .def = 75, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "MIDI In Ch", .min = 0, .max = 16, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL},
    {.name = "Panic On Wrap", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings},
    {.name = "Scale Root", .min = 0, .max = 11, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = scaleRootStrings},
    {.name = "Scale", .min = 0, .max = 11, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = scaleTypeStrings},
    {.name = "Clear Track", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings},
    {.name = "Clear All", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings},

    // Generate parameters (12-19)
    {.name = "Generate", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings},
    {.name = "Gen Mode", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = genModeStrings},
    {.name = "Density", .min = 1, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "Bias", .min = 0, .max = 127, .def = 60, .unit = kNT_unitMIDINote, .scaling = 0, .enumStrings = NULL},
    {.name = "Range", .min = 0, .max = 48, .def = 12, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL},
    {.name = "Note Rand", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "Vel Var", .min = 0, .max = 100, .def = 20, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "Ties", .min = 0, .max = 100, .def = 20, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "Gate Rand", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL},
    {.name = "Fill", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings},

    // Track parameters - PARAMS_PER_TRACK per track
    TRACK_PARAMS(1, 2) // Track 1: enabled by default, channel 2
    TRACK_PARAMS(0, 3) // Track 2: disabled by default, channel 3
    TRACK_PARAMS(0, 4) // Track 3: disabled by default, channel 4
    TRACK_PARAMS(0, 5) // Track 4: disabled by default, channel 5
};

#undef TRACK_PARAMS

// ============================================================================
// PARAMETER PAGES
// ============================================================================

// Page 0: Routing (Input bus selection)
static const uint8_t pageRouting[] = {kParamRunInput, kParamClockInput};

// Page 1: Global (Recording)
static const uint8_t pageGlobal[] = {kParamRecord, kParamRecTrack, kParamRecDivision, kParamRecMode, kParamRecSnap, kParamClearTrack, kParamClearAll, kParamFill};

// Page 2: MIDI Config
static const uint8_t pageMidiConfig[] = {kParamMidiInCh, kParamPanicOnWrap, kParamScaleRoot, kParamScaleType};

// Page 3: Generate
static const uint8_t pageGenerate[] = {kParamGenerate,    kParamGenMode,     kParamGenDensity, kParamGenBias,
                                       kParamGenRange,    kParamGenNoteRand, kParamGenVelVar,  kParamGenTies,
                                       kParamGenGateRand};

// ============================================================================
// DYNAMIC PAGE BUILDING (for specification-based track count)
// ============================================================================

static const char* const trackPageNames[] = {"Track 1", "Track 2", "Track 3", "Track 4"};

// Build track page indices into the provided array
// Returns the number of indices written (PARAMS_PER_TRACK)
static inline int buildTrackPageIndices(uint8_t* indices, int track) {
    int base = kGlobalParamCount + (track * PARAMS_PER_TRACK);
    for (int i = 0; i < PARAMS_PER_TRACK; i++) {
        indices[i] = base + i;
    }
    return PARAMS_PER_TRACK;
}

// Calculate total parameters for a given track count
static inline int calcTotalParams(int numTracks) { return GLOBAL_PARAMS + (PARAMS_PER_TRACK * numTracks); }
