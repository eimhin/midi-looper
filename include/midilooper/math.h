/*
 * MIDI Looper - Math Utilities
 * Basic math functions and safe array access helpers
 */

#pragma once

#include "midilooper/config.h"
#include "midilooper/types.h"

// ============================================================================
// DEBUG BOUNDS CHECKING
// ============================================================================

// Define MIDILOOPER_DEBUG to enable bounds checking assertions
// #define MIDILOOPER_DEBUG

#ifdef MIDILOOPER_DEBUG
    static inline void debugLog(const char* msg, int value, int max) {
        (void)msg; (void)value; (void)max;
    }

    #define ASSERT_BOUNDS(idx, max, name) \
        do { \
            if ((idx) < 0 || (idx) >= (max)) { \
                debugLog("Bounds error in " name ": ", (idx), (max)); \
            } \
        } while(0)

    #define ASSERT_RANGE(val, min, max, name) \
        do { \
            if ((val) < (min) || (val) > (max)) { \
                debugLog("Range error in " name ": ", (val), (max)); \
            } \
        } while(0)
#else
    #define ASSERT_BOUNDS(idx, max, name) ((void)0)
    #define ASSERT_RANGE(val, min, max, name) ((void)0)
#endif

// ============================================================================
// MATH UTILITIES
// ============================================================================

static inline int clamp(int x, int lo, int hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static inline float clampf(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

// ============================================================================
// SAFE ARRAY ACCESS HELPERS
// ============================================================================

static inline int safeStepIndex(int idx) {
    if (idx < 0) return 0;
    if (idx >= MAX_STEPS) return MAX_STEPS - 1;
    return idx;
}

static inline int safeTrackIndex(int idx) {
    if (idx < 0) return 0;
    if (idx >= MAX_TRACKS) return MAX_TRACKS - 1;
    return idx;
}

static inline int safeNoteIndex(int idx) {
    if (idx < 0) return 0;
    if (idx >= 128) return 127;
    return idx;
}

static inline int safeDelayedNoteIndex(int idx) {
    if (idx < 0) return 0;
    if (idx >= MAX_DELAYED_NOTES) return MAX_DELAYED_NOTES - 1;
    return idx;
}
