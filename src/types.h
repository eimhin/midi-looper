/*
 * MIDI Looper - Types and Constants
 * Data structures, constants, and parameter enums
 */

#pragma once

#include <distingnt/api.h>
#include <cstdint>
#include "config.h"

// ============================================================================
// MIDI CONSTANTS
// ============================================================================

// MIDI status bytes
static constexpr uint8_t kMidiNoteOff = 0x80;
static constexpr uint8_t kMidiNoteOn = 0x90;
static constexpr uint8_t kMidiCC = 0xB0;

// Transport state machine
enum TransportState {
    TRANSPORT_STOPPED = 0,
    TRANSPORT_RUNNING = 1,
    TRANSPORT_RECORDING = 2  // implies running
};

// ============================================================================
// TRANSPORT STATE MACHINE TRANSITIONS
// ============================================================================
//
// Valid state transitions:
//
//                     ┌─────────────────┐
//                     │    STOPPED      │
//                     └────────┬────────┘
//                              │ Start (gate rising edge)
//                              ▼
//                     ┌─────────────────┐
//         ┌──────────►│    RUNNING      │◄──────────┐
//         │           └────────┬────────┘           │
//         │                    │ RecordBegin        │ RecordEnd
//         │                    ▼                    │
//         │           ┌─────────────────┐           │
//         │           │   RECORDING     │───────────┘
//         │           └────────┬────────┘
//         │                    │
//         └────────────────────┘
//               Stop (gate falling edge from any state)
//

// Transport state query helpers
static inline bool transportIsRunning(TransportState state) {
    return state != TRANSPORT_STOPPED;
}

static inline bool transportIsRecording(TransportState state) {
    return state == TRANSPORT_RECORDING;
}

// State transition: Start playback (gate rising edge)
// Valid from: any state (resets playback position)
static inline TransportState transportTransition_Start(TransportState current) {
    (void)current;  // Valid from any state - always resets to RUNNING
    return TRANSPORT_RUNNING;
}

// State transition: Stop playback (gate falling edge)
// Valid from: any state
static inline TransportState transportTransition_Stop(TransportState current) {
    (void)current;  // Valid from any state
    return TRANSPORT_STOPPED;
}

// State transition: Begin recording (record parameter enabled)
// Valid from: RUNNING only
static inline TransportState transportTransition_RecordBegin(TransportState current) {
    DEBUG_ASSERT(current == TRANSPORT_RUNNING, "RecordBegin requires RUNNING state");
    return (current == TRANSPORT_RUNNING) ? TRANSPORT_RECORDING : current;
}

// State transition: End recording (record parameter disabled)
// Valid from: RECORDING only
static inline TransportState transportTransition_RecordEnd(TransportState current) {
    DEBUG_ASSERT(current == TRANSPORT_RECORDING, "RecordEnd requires RECORDING state");
    return (current == TRANSPORT_RECORDING) ? TRANSPORT_RUNNING : current;
}

// Direction constants (0-indexed to match parameter values)
static constexpr int DIR_FORWARD = 0;
static constexpr int DIR_REVERSE = 1;
static constexpr int DIR_PENDULUM = 2;
static constexpr int DIR_PINGPONG = 3;
static constexpr int DIR_STRIDE = 4;
static constexpr int DIR_ODD_EVEN = 5;
static constexpr int DIR_HOPSCOTCH = 6;
static constexpr int DIR_CONVERGE = 7;
static constexpr int DIR_DIVERGE = 8;
static constexpr int DIR_BROWNIAN = 9;
static constexpr int DIR_RANDOM = 10;
static constexpr int DIR_SHUFFLE = 11;

// Step mask patterns (0-indexed to match parameter values)
static constexpr int MASK_ALL = 0;
static constexpr int MASK_ODDS = 1;
static constexpr int MASK_EVENS = 2;
static constexpr int MASK_FIRST_HALF = 3;
static constexpr int MASK_SECOND_HALF = 4;
static constexpr int MASK_SPARSE = 5;
static constexpr int MASK_DENSE = 6;
static constexpr int MASK_RANDOM = 7;

// Quantize values mapping (index 0-4 -> actual division)
static constexpr int QUANTIZE_VALUES[] = { 1, 2, 4, 8, 16 };

// UI layout constants
static constexpr int UI_LEFT_MARGIN = 2;
static constexpr int UI_VEL_BAR_TOP = 12;
static constexpr int UI_VEL_BAR_BOTTOM = 32;
static constexpr int UI_VEL_BAR_WIDTH = 7;
static constexpr int UI_VEL_BAR_HEIGHT = 20;
static constexpr int UI_STEP_Y_TOP = 26;
static constexpr int UI_STEP_Y_BOTTOM = 30;
static constexpr int UI_STEP_SPACING = 8;
static constexpr int UI_STEP_WIDTH = 4;
static constexpr int UI_INPUT_LABEL_X = 163;
static constexpr int UI_INPUT_BAR_X = 179;
static constexpr int UI_OUTPUT_LABEL_X = 195;
static constexpr int UI_OUTPUT_BAR_X = 211;
static constexpr int UI_OUTPUT_BAR_SPACE = 10;
static constexpr int UI_LABEL_Y = 20;
static constexpr int UI_TRACK_WIDTH = 65;
static constexpr int UI_TRACK_BOX_WIDTH = 56;
static constexpr int UI_TRACK_BOX_TOP = 40;
static constexpr int UI_TRACK_BOX_BOTTOM = 52;
static constexpr int UI_TRACK_TEXT_Y = 50;
static constexpr int UI_CHAR_WIDTH = 6;
static constexpr int UI_BRIGHTNESS_MAX = 15;
static constexpr int UI_BRIGHTNESS_DIM = 1;
static constexpr int UI_STOP_RIGHT = 10;
static constexpr int UI_STOP_BOTTOM = 20;
static constexpr int UI_REC_CENTER_X = 18;
static constexpr int UI_REC_CENTER_Y = 16;
static constexpr int UI_REC_RADIUS = 4;

// Gate detection thresholds (in volts, comparing against CV input)
static constexpr float GATE_THRESHOLD_HIGH = 2.0f;
static constexpr float GATE_THRESHOLD_LOW = 0.5f;

// ============================================================================
// PARAMETER ENUMS
// ============================================================================

// Global parameter indices (0-8)
enum {
    kParamRecord = 0,
    kParamRecTrack,
    kParamRecMode,
    kParamRecSnap,
    kParamMidiInCh,
    kParamMidiOutDest,
    kParamPanicOnWrap,
    kParamClearTrack,
    kParamClearAll,

    kGlobalParamCount  // = 9
};

// Per-track parameter offsets (0-18)
enum {
    kTrackEnabled = 0,
    kTrackLength,
    kTrackDivision,
    kTrackDirection,
    kTrackStrideSize,
    kTrackVelocity,
    kTrackHumanize,
    kTrackChannel,
    kTrackStability,
    kTrackMotion,
    kTrackRandomness,
    kTrackGravity,
    kTrackGravityAnchor,
    kTrackPedal,
    kTrackPedalStep,
    kTrackNoRepeat,
    kTrackStepMask,
    kTrackReadMode,
    kTrackReadWindow,

    kTrackParamCount  // = 19
};

// Validate parameter layout matches config constants
static_assert(PARAMS_PER_TRACK == kTrackParamCount,
              "PARAMS_PER_TRACK must match kTrackParamCount enum");
static_assert(GLOBAL_PARAMS == kGlobalParamCount,
              "GLOBAL_PARAMS must match kGlobalParamCount enum");

// Helper to get track parameter index
static inline int trackParam(int track, int param) {
    return kGlobalParamCount + (track * PARAMS_PER_TRACK) + param;
}

// ============================================================================
// TRACK PARAMETERS ACCESSOR
// ============================================================================

// Simple clamp for use in TrackParams (avoids circular dependency with utils)
static inline int clampParam(int val, int minVal, int maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// Encapsulates all parameters for a single track
// Provides clean access without scattered v[] index calculations
struct TrackParams {
    const int16_t* v;   // Pointer to parameter array
    int track;          // Track index (0-3)

    // Factory method
    static TrackParams fromAlgorithm(const int16_t* params, int trackIdx) {
        TrackParams p;
        p.v = params;
        p.track = trackIdx;
        return p;
    }

    // Raw parameter access (no clamping)
    int raw(int param) const {
        return v[trackParam(track, param)];
    }

    // Basic track settings
    bool enabled() const { return raw(kTrackEnabled) == 1; }
    int length() const { return clampParam(raw(kTrackLength), 1, MAX_STEPS); }
    int division() const { return clampParam(raw(kTrackDivision), 0, 4); }
    int direction() const { return raw(kTrackDirection); }
    int strideSize() const { return raw(kTrackStrideSize); }

    // Output settings
    int channel() const { return clampParam(raw(kTrackChannel), 1, 16); }
    int velocity() const { return raw(kTrackVelocity); }  // Offset, can be negative
    int humanize() const { return raw(kTrackHumanize); }

    // Continuous modifiers
    int stability() const { return raw(kTrackStability); }
    int motion() const { return raw(kTrackMotion); }
    int randomness() const { return raw(kTrackRandomness); }
    int gravity() const { return raw(kTrackGravity); }
    int gravityAnchor(int loopLen) const {
        return clampParam(raw(kTrackGravityAnchor), 1, loopLen);
    }
    int pedal() const { return raw(kTrackPedal); }
    int pedalStep(int loopLen) const {
        return clampParam(raw(kTrackPedalStep), 1, loopLen);
    }

    // Binary modifiers
    int noRepeat() const { return raw(kTrackNoRepeat); }
    int stepMask() const { return raw(kTrackStepMask); }

    // Read mode
    int readMode() const { return raw(kTrackReadMode); }
    int readWindow(int loopLen) const {
        return clampParam(raw(kTrackReadWindow), 1, loopLen);
    }
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================

// Note event (stored per step)
struct NoteEvent {
    uint8_t note;       // MIDI note number (0-127)
    uint8_t velocity;   // Velocity (0-127)
    uint16_t duration;  // Duration in clock ticks
};

// Events for a single step
struct StepEvents {
    NoteEvent events[MAX_EVENTS_PER_STEP];
    uint8_t count;
};

// Track data (step events only)
struct TrackData {
    StepEvents steps[MAX_STEPS];
};

// Held note during recording
struct HeldNote {
    uint8_t note;
    uint8_t velocity;
    uint8_t track;
    uint8_t quantizedStep;
    uint8_t effectiveStep;
    uint8_t quantize;
    uint8_t loopLen;
    uint8_t rawStep;
    bool active;
};

// Delayed note for humanization
struct DelayedNote {
    uint8_t note;
    uint8_t velocity;
    uint8_t track;
    uint8_t outCh;
    uint16_t duration;
    uint16_t delay;  // Remaining delay in step() calls
    uint32_t where;
    bool active;
};

// Playing note (tracking duration countdown)
struct PlayingNote {
    uint8_t note;
    uint16_t remaining;  // Remaining duration in clock ticks
    bool active;
};

// Cached derived values per track (computed from parameters)
// These are expensive to calculate and only change when parameters change
struct TrackCache {
    uint8_t effectiveQuantize;  // Cached quantize value
    uint8_t loopLen;            // Cached loop length
    bool dirty;                 // True if cache needs refresh

    // Initialize as dirty so first access calculates values
    void invalidate() { dirty = true; }
};

// Unified per-track state (allocated dynamically in DRAM)
// Combines track data with all per-track runtime state
struct TrackState {
    // Step event data
    TrackData data;

    // Playing notes (for duration tracking)
    PlayingNote playing[128];
    uint8_t activeNotes[128];  // Velocity of active notes

    // Shuffle order for shuffle direction mode
    uint8_t shuffleOrder[MAX_STEPS];

    // Playback state
    uint16_t clockCount;
    uint8_t step;           // Current step position
    uint8_t lastStep;       // Previous step (for no-repeat)
    uint8_t brownianPos;    // Brownian walk position
    uint8_t shufflePos;     // Position in shuffle order
    uint8_t readCyclePos;   // Position in read cycle
    uint8_t activeVel;      // Highest active velocity (for UI)

    // Parameter change detection
    int16_t lastEnabled;

    // Parameter cache
    TrackCache cache;
};

// DTC (Data Tightly Coupled) - Fast access global state for step()
// Per-track state is now in TrackState (DRAM)
struct MidiLooper_DTC {
    // Transport state (explicit state machine)
    TransportState transportState;

    // Gate/trigger edge detection
    bool prevGateHigh;
    bool prevClockHigh;

    // Timing
    float stepTime;
    float stepDuration;

    // Edge detection for parameter changes
    int16_t lastRecord;
    int16_t lastTrack;
    int16_t lastClearTrack;
    int16_t lastClearAll;

    // Input display
    uint8_t inputVel;
    uint8_t inputNotes[128];  // Bitmask of held input notes
};

// Main algorithm structure (SRAM)
struct MidiLooperAlgorithm : public _NT_algorithm {
    MidiLooper_DTC* dtc;
    TrackState* trackStates;  // Dynamically allocated per-track state

    // Dynamic track configuration (from specification)
    uint8_t numTracks;

    // Dynamic parameter pages (built in construct based on numTracks)
    uint8_t pageTrackIndices[MAX_TRACKS][PARAMS_PER_TRACK];
    _NT_parameterPage pageDefs[MAX_PAGES];
    _NT_parameterPages dynamicPages;

    // Held notes during recording
    HeldNote heldNotes[128];

    // Delayed notes for humanization
    DelayedNote delayedNotes[MAX_DELAYED_NOTES];

    // Simple PRNG state
    uint32_t randState;

    MidiLooperAlgorithm(MidiLooper_DTC* dtc_, TrackState* trackStates_, uint8_t numTracks_)
        : dtc(dtc_), trackStates(trackStates_), numTracks(numTracks_), randState(12345) {}
};
