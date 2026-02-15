#include "recording.h"
#include "midi.h"

// ============================================================================
// RECORDING OPERATIONS
// ============================================================================

void recordNoteOn(
    MidiLooperAlgorithm* alg,
    const RecordingContext& ctx,
    uint8_t note,
    uint8_t velocity
) {
    HeldNote* held = &alg->heldNotes[note];
    held->active = true;
    held->note = note;
    held->velocity = velocity;
    held->track = (uint8_t)ctx.track;
    held->quantizedStep = (uint8_t)snapToDivisionSubclock(
        ctx.rawStep, ctx.stepFraction, ctx.quantize, ctx.snapThreshold, ctx.loopLen
    );
    held->effectiveStep = (uint8_t)snapStepSubclock(
        ctx.rawStep, ctx.stepFraction, ctx.snapThreshold, ctx.loopLen
    );
    held->quantize = (uint8_t)ctx.quantize;
    held->loopLen = (uint8_t)ctx.loopLen;
    held->rawStep = (uint8_t)ctx.rawStep;
}

void recordNoteOff(
    MidiLooperAlgorithm* alg,
    const RecordingContext& ctx,
    uint8_t note
) {
    HeldNote* held = &alg->heldNotes[note];
    if (!held->active) return;

    int effectiveEndStep = snapStepSubclock(
        ctx.rawStep, ctx.stepFraction, ctx.snapThreshold, held->loopLen
    );

    int duration = effectiveEndStep - held->effectiveStep;
    if (duration < 0) duration += held->loopLen;
    if (duration < 1) duration = 1;
    duration = calcQuantizedDuration(duration, held->quantize);

    int maxDuration = held->loopLen - held->quantizedStep + 1;
    if (duration > maxDuration) duration = maxDuration;

    // Store the event
    int heldTrack = safeTrackIndex(held->track);
    int heldStepIdx = safeStepIndex(held->quantizedStep - 1);
    StepEvents* evs = &alg->trackStates[heldTrack].data.steps[heldStepIdx];

    if (!hasNoteEvent(evs, note)) {
        addEvent(evs, note, held->velocity, (uint16_t)duration);
    }

    held->active = false;
}

void finalizeHeldNotes(MidiLooperAlgorithm* alg) {
    for (int noteNum = 0; noteNum < 128; noteNum++) {
        HeldNote* held = &alg->heldNotes[noteNum];
        if (!held->active) continue;

        int track = safeTrackIndex(held->track);
        int currentStep = clamp(alg->trackStates[track].step, 1, held->loopLen);

        int duration = currentStep - held->effectiveStep;
        if (duration < 0) duration += held->loopLen;
        if (duration < 1) duration = 1;
        duration = calcQuantizedDuration(duration, held->quantize);

        int maxDuration = held->loopLen - held->quantizedStep + 1;
        if (duration > maxDuration) duration = maxDuration;

        int stepIdx = safeStepIndex(held->quantizedStep - 1);
        StepEvents* evs = &alg->trackStates[track].data.steps[stepIdx];

        if (!hasNoteEvent(evs, (uint8_t)noteNum)) {
            addEvent(evs, (uint8_t)noteNum, held->velocity, (uint16_t)duration);
        }

        held->active = false;
    }
}

void clearHeldNotes(MidiLooperAlgorithm* alg) {
    for (int i = 0; i < 128; i++) {
        alg->heldNotes[i].active = false;
    }
}
