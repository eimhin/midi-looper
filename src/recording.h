/*
 * MIDI Looper - Recording Subsystem
 * Handles note recording logic separate from MIDI pass-through
 */

#pragma once

#include "types.h"
#include "quantize.h"
#include "math.h"

// ============================================================================
// RECORDING CONTEXT
// ============================================================================

// Encapsulates all parameters needed for recording operations
struct RecordingContext {
    int track;
    int loopLen;
    int quantize;
    float snapThreshold;
    int rawStep;
    float stepFraction;
};

// Create recording context from algorithm state
// Uses cached quantize values for efficiency in MIDI handling path
inline RecordingContext createRecordingContext(
    const int16_t* v,
    int track,
    int currentStep,
    float stepTime,
    float stepDuration,
    TrackCache* cache
) {
    RecordingContext ctx;
    ctx.track = track;
    ctx.quantize = getCachedQuantize(v, track, cache, ctx.loopLen);
    ctx.rawStep = clamp(currentStep, 1, ctx.loopLen);
    ctx.stepFraction = (stepDuration > 0.0f) ? clampf(stepTime / stepDuration, 0.0f, 1.0f) : 0.0f;
    ctx.snapThreshold = (float)v[kParamRecSnap] / 100.0f;
    return ctx;
}

// Recording operations
void recordNoteOn(MidiLooperAlgorithm* alg, const RecordingContext& ctx, uint8_t note, uint8_t velocity);
void recordNoteOff(MidiLooperAlgorithm* alg, const RecordingContext& ctx, uint8_t note);
void finalizeHeldNotes(MidiLooperAlgorithm* alg);
void clearHeldNotes(MidiLooperAlgorithm* alg);

// Step record operations
void stepRecordNoteOn(MidiLooperAlgorithm* alg, int track, uint8_t note, uint8_t velocity);
void stepRecordNoteOff(MidiLooperAlgorithm* alg, int track, uint8_t note);
