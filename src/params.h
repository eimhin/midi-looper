/*
 * MIDI Looper - Parameter Definitions
 * Parameter arrays and page definitions for the distingNT UI
 */

#pragma once

#include "config.h"
#include "types.h"
#include <cstddef>  // for NULL

// ============================================================================
// PARAMETER STRING ARRAYS
// ============================================================================

static const char* const recordStrings[] = { "Off", "On", NULL };
static const char* const recTrackStrings[] = { "1", "2", "3", "4", NULL };
static const char* const recModeStrings[] = { "Replace", "Overdub", NULL };
static const char* const midiDestStrings[] = { "Breakout", "SelectBus", "USB", "Internal", "All", NULL };
static const char* const noYesStrings[] = { "No", "Yes", NULL };
static const char* const divisionStrings[] = { "1", "2", "4", "8", "16", NULL };
static const char* const directionStrings[] = {
    "Forward", "Reverse", "Pendulum", "Ping-Pong", "Stride", "Odd/Even",
    "Hopscotch", "Converge", "Diverge", "Brownian", "Random", "Shuffle", NULL
};
static const char* const stepMaskStrings[] = {
    "All", "Odds", "Evens", "1st Half", "2nd Half", "Sparse", "Dense", "Random", NULL
};
static const char* const readModeStrings[] = {
    "Single", "Arp N", "Growing", "Shrinking", "Pedal Read", NULL
};

// ============================================================================
// PARAMETER DEFINITIONS
// ============================================================================

// Track parameter macro - generates 19 parameters per track
// DEF_ENABLED: default for Enabled (1 for track 1, 0 for others)
// DEF_CHANNEL: default MIDI channel (1-4 for tracks 1-4)
#define TRACK_PARAMS(DEF_ENABLED, DEF_CHANNEL) \
    { .name = "Enabled", .min = 0, .max = 1, .def = DEF_ENABLED, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings }, \
    { .name = "Length", .min = 1, .max = MAX_STEPS, .def = 16, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Division", .min = 0, .max = 4, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = divisionStrings }, \
    { .name = "Direction", .min = 0, .max = 11, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = directionStrings }, \
    { .name = "Stride Size", .min = 2, .max = 16, .def = 2, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Velocity", .min = -64, .max = 64, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Humanize", .min = 0, .max = 100, .def = 0, .unit = kNT_unitMs, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Channel", .min = 1, .max = 16, .def = DEF_CHANNEL, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Stability", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Motion", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Randomness", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Gravity", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Anchor", .min = 1, .max = MAX_STEPS, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Pedal", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
    { .name = "Pedal Step", .min = 1, .max = MAX_STEPS, .def = 1, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
    { .name = "No Repeat", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings }, \
    { .name = "Step Mask", .min = 0, .max = 7, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = stepMaskStrings }, \
    { .name = "Read Mode", .min = 0, .max = 4, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = readModeStrings }, \
    { .name = "Read Window", .min = 1, .max = 8, .def = 2, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },

// Parameter definitions (85 total)
static const _NT_parameter parameters[] = {
    // Global parameters (0-8)
    { .name = "Record", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recordStrings },
    { .name = "Rec Track", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recTrackStrings },
    { .name = "Rec Mode", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = recModeStrings },
    { .name = "Rec Snap", .min = 50, .max = 100, .def = 75, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL },
    { .name = "MIDI In Ch", .min = 0, .max = 16, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },
    { .name = "MIDI Out Dest", .min = 0, .max = 4, .def = 2, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = midiDestStrings },
    { .name = "Panic On Wrap", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings },
    { .name = "Clear Track", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings },
    { .name = "Clear All", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = noYesStrings },

    // Track parameters (9-84) - 19 params per track
    TRACK_PARAMS(1, 1)  // Track 1: enabled by default, channel 1
    TRACK_PARAMS(0, 2)  // Track 2: disabled by default, channel 2
    TRACK_PARAMS(0, 3)  // Track 3: disabled by default, channel 3
    TRACK_PARAMS(0, 4)  // Track 4: disabled by default, channel 4
};

#undef TRACK_PARAMS

// ============================================================================
// PARAMETER PAGES
// ============================================================================

// Page 1: Global (Recording)
static const uint8_t pageGlobal[] = {
    kParamRecord, kParamRecTrack, kParamRecMode, kParamRecSnap
};

// Page 2: MIDI Config
static const uint8_t pageMidiConfig[] = {
    kParamMidiInCh, kParamMidiOutDest, kParamPanicOnWrap, kParamClearTrack, kParamClearAll
};

// ============================================================================
// DYNAMIC PAGE BUILDING (for specification-based track count)
// ============================================================================

static const char* const trackPageNames[] = { "Track 1", "Track 2", "Track 3", "Track 4" };

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
static inline int calcTotalParams(int numTracks) {
    return GLOBAL_PARAMS + (PARAMS_PER_TRACK * numTracks);
}
