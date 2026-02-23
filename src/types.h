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
    TRANSPORT_RUNNING = 1
};

// Recording state machine
enum RecordState {
    REC_IDLE = 0,         // Not recording
    REC_LIVE,             // Live recording (transport must be RUNNING)
    REC_STEP,             // Step recording (transport-independent)
    REC_LIVE_PENDING      // Record ON + live mode, waiting for transport
};

// ============================================================================
// TRANSPORT STATE MACHINE TRANSITIONS
// ============================================================================
//
//   STOPPED ──Start──► RUNNING ──Stop──► STOPPED
//
// ============================================================================
// RECORDING STATE MACHINE TRANSITIONS
// ============================================================================
//
//   REC_IDLE ──Record ON + Step──────────► REC_STEP
//   REC_IDLE ──Record ON + Live + running─► REC_LIVE
//   REC_IDLE ──Record ON + Live + stopped─► REC_LIVE_PENDING
//
//   REC_LIVE ──Record OFF / Transport stop─► REC_IDLE
//   REC_LIVE ──Mode changed to Step───────► REC_STEP
//
//   REC_STEP ──Record OFF─────────────────► REC_IDLE
//   REC_STEP ──Mode to Live + running─────► REC_LIVE
//   REC_STEP ──Mode to Live + stopped─────► REC_LIVE_PENDING
//
//   REC_LIVE_PENDING ──Record OFF─────────► REC_IDLE
//   REC_LIVE_PENDING ──Mode to Step───────► REC_STEP
//   REC_LIVE_PENDING ──Transport starts───► REC_LIVE
//

// Transport state query helper
static inline bool transportIsRunning(TransportState state) {
    return state != TRANSPORT_STOPPED;
}

// Recording state query helpers
static inline bool isRecording(RecordState state) {
    return state != REC_IDLE;
}
static inline bool isLiveRecording(RecordState state) {
    return state == REC_LIVE;
}
static inline bool isStepRecording(RecordState state) {
    return state == REC_STEP;
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

// Trig condition constants
static constexpr int COND_FIXED = 75;

// Direction constants (0-indexed to match parameter values)
static constexpr int DIR_FORWARD = 0;
static constexpr int DIR_REVERSE = 1;
static constexpr int DIR_PENDULUM = 2;
static constexpr int DIR_PINGPONG = 3;
static constexpr int DIR_ODD_EVEN = 4;
static constexpr int DIR_HOPSCOTCH = 5;
static constexpr int DIR_CONVERGE = 6;
static constexpr int DIR_DIVERGE = 7;
static constexpr int DIR_BROWNIAN = 8;
static constexpr int DIR_RANDOM = 9;
static constexpr int DIR_SHUFFLE = 10;
static constexpr int DIR_STRIDE2 = 11;
static constexpr int DIR_STRIDE3 = 12;
static constexpr int DIR_STRIDE4 = 13;
static constexpr int DIR_STRIDE5 = 14;

// Generate mode constants
static constexpr int GEN_MODE_NEW = 0;
static constexpr int GEN_MODE_REORDER = 1;
static constexpr int GEN_MODE_REPITCH = 2;
static constexpr int GEN_MODE_INVERT = 3;

// Recording mode constants
static constexpr int REC_MODE_REPLACE = 0;
static constexpr int REC_MODE_OVERDUB = 1;
static constexpr int REC_MODE_STEP    = 2;

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
static constexpr int UI_INPUT_LABEL_X = 124;
static constexpr int UI_INPUT_BAR_X = 138;
static constexpr int UI_OUTPUT_LABEL_X = 162;
static constexpr int UI_OUTPUT_BAR_X = 176;
static constexpr int UI_OUTPUT_BAR_SPACE = 10;
static constexpr int UI_LABEL_Y = 20;
static constexpr int UI_TRACK_WIDTH = 65;
static constexpr int UI_TRACK_BOX_WIDTH = 56;

// Track boxes - Row 1 (tracks 1-4)
static constexpr int UI_TRACK_ROW1_TOP = 35;
static constexpr int UI_TRACK_ROW1_BOTTOM = 45;
static constexpr int UI_TRACK_ROW1_TEXT_Y = 43;

// Track boxes - Row 2 (tracks 5-8)
static constexpr int UI_TRACK_ROW2_TOP = 47;
static constexpr int UI_TRACK_ROW2_BOTTOM = 57;
static constexpr int UI_TRACK_ROW2_TEXT_Y = 55;
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

// Global parameter indices (0-10)
enum {
    kParamRunInput = 0,    // CV input bus selector for run/gate
    kParamClockInput,      // CV input bus selector for clock/trigger
    kParamRecord,
    kParamRecTrack,
    kParamRecDivision,
    kParamRecMode,
    kParamRecSnap,
    kParamMidiInCh,
    kParamPanicOnWrap,
    kParamScaleRoot,
    kParamScaleType,
    kParamClearTrack,
    kParamClearAll,
    kParamGenerate,
    kParamGenMode,
    kParamGenDensity,
    kParamGenBias,
    kParamGenRange,
    kParamGenNoteRand,
    kParamGenVelVar,
    kParamGenTies,
    kParamGenGateRand,
    kParamFill,

    kGlobalParamCount  // = 23
};

// Per-track parameter offsets (0-25)
enum {
    kTrackEnabled = 0,
    kTrackLength,
    kTrackClockDiv,
    kTrackDirection,
    kTrackVelocity,
    kTrackHumanize,
    kTrackChannel,
    kTrackDestination,
    kTrackStability,
    kTrackMotion,
    kTrackRandomness,
    kTrackPedal,
    kTrackPedalStep,
    kTrackNoRepeat,
    kTrackOctMin,
    kTrackOctMax,
    kTrackOctProb,
    kTrackOctBypass,
    kTrackStepProb,
    kTrackStepCond,
    kTrackCondStepA,
    kTrackCondA,
    kTrackProbA,
    kTrackCondStepB,
    kTrackCondB,
    kTrackProbB,

    kTrackParamCount  // = 26
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
    int track;          // Track index (0-7)

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
    int direction() const { return raw(kTrackDirection); }

    // Output settings
    int channel() const { return clampParam(raw(kTrackChannel), 1, 16); }
    int destination() const { return raw(kTrackDestination); }
    int velocity() const { return raw(kTrackVelocity); }  // Offset, can be negative
    int humanize() const { return raw(kTrackHumanize); }

    // Continuous modifiers
    int stability() const { return raw(kTrackStability); }
    int motion() const { return raw(kTrackMotion); }
    int randomness() const { return raw(kTrackRandomness); }
    int pedal() const { return raw(kTrackPedal); }
    int pedalStep(int loopLen) const {
        return clampParam(raw(kTrackPedalStep), 1, loopLen);
    }

    // Binary modifiers
    int noRepeat() const { return raw(kTrackNoRepeat); }

    // Octave jump
    int octMin() const { return raw(kTrackOctMin); }
    int octMax() const { return raw(kTrackOctMax); }
    int octProb() const { return raw(kTrackOctProb); }
    int octBypass() const { return raw(kTrackOctBypass); }

    // Step conditions
    int stepProb() const { return raw(kTrackStepProb); }
    int stepCond() const { return raw(kTrackStepCond); }
    int condStepA() const { return raw(kTrackCondStepA); }
    int condA() const { return raw(kTrackCondA); }
    int probA() const { return raw(kTrackProbA); }
    int condStepB() const { return raw(kTrackCondStepB); }
    int condB() const { return raw(kTrackCondB); }
    int probB() const { return raw(kTrackProbB); }

    int clockDiv() const {
        return clampParam(raw(kTrackClockDiv), 1, 16);
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
    uint16_t divCounter;    // Clock division counter
    uint16_t loopCount;     // Loop iteration counter (for trig conditions)
    uint8_t step;           // Current step position
    uint8_t lastStep;       // Previous step (for no-repeat)
    uint8_t brownianPos;    // Brownian walk position
    uint8_t shufflePos;     // Position in shuffle order
    uint8_t activeVel;      // Highest active velocity (for UI)
    uint16_t octavePlayCount; // Octave jump note-play counter

    // Parameter change detection
    int16_t lastEnabled;

    // Parameter cache
    TrackCache cache;

    // Per-track PRNG state
    uint32_t randState;
};

// DTC (Data Tightly Coupled) - Fast access global state for step()
// Per-track state is now in TrackState (DRAM)
struct MidiLooper_DTC {
    // Transport state (explicit state machine)
    TransportState transportState;

    // Recording state (explicit state machine)
    RecordState recordState;

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
    int16_t lastGenerate;

    // Step record state
    uint8_t stepRecPos;  // Step record cursor: 1-based division-step index, 0 = inactive

    // Input display
    uint8_t inputVel;
    uint8_t inputNotes[128];  // Bitmask of held input notes

    // Scale quantization note tracking
    // Maps original MIDI note → quantized note sent, so Note Off releases the correct note
    uint8_t noteMap[128];
};

// Main algorithm structure (SRAM)
struct MidiLooperAlgorithm : public _NT_algorithm {
    MidiLooper_DTC* dtc;
    TrackState* trackStates;  // Dynamically allocated per-track state

    // Dynamic track configuration (from specification)
    uint8_t numTracks;

    // Mutable copy of parameter definitions (for runtime max adjustments)
    _NT_parameter paramDefs[MAX_TOTAL_PARAMS];

    // Dynamic parameter pages (built in construct based on numTracks)
    uint8_t pageTrackIndices[MAX_TRACKS][PARAMS_PER_TRACK];
    _NT_parameterPage pageDefs[MAX_PAGES];
    _NT_parameterPages dynamicPages;

    // Held notes during recording
    HeldNote heldNotes[128];

    // Delayed notes for humanization
    DelayedNote delayedNotes[MAX_DELAYED_NOTES];

    MidiLooperAlgorithm(MidiLooper_DTC* dtc_, TrackState* trackStates_, uint8_t numTracks_)
        : dtc(dtc_), trackStates(trackStates_), numTracks(numTracks_) {}
};
