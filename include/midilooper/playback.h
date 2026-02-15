/*
 * MIDI Looper - Playback Subsystem
 * Transport control, note scheduling, and step processing
 */

#pragma once

#include "midilooper/config.h"
#include "midilooper/types.h"
#include "midilooper/math.h"
#include "midilooper/random.h"
#include "midilooper/midi.h"
#include "midilooper/directions.h"
#include "midilooper/modifiers.h"
#include "midilooper/recording.h"

// ============================================================================
// TRANSPORT CONTROL
// ============================================================================

// Reset all tracks and start transport
static void handleTransportStart(MidiLooperAlgorithm* alg) {
    MidiLooper_DTC* dtc = alg->dtc;

    for (int t = 0; t < alg->numTracks; t++) {
        TrackState* ts = &alg->trackStates[t];
        ts->step = 0;
        ts->clockCount = 0;
        ts->lastStep = 1;
        ts->brownianPos = 1;
        ts->shufflePos = 1;
        ts->readCyclePos = 0;

        for (int s = 0; s < MAX_STEPS; s++) {
            ts->shuffleOrder[s] = (uint8_t)(s + 1);
        }
    }
    dtc->stepTime = 0.0f;
    dtc->transportState = transportTransition_Start(dtc->transportState);
}

// Stop transport and clear all note state
static void handleTransportStop(MidiLooperAlgorithm* alg, uint32_t where) {
    MidiLooper_DTC* dtc = alg->dtc;

    // Finalize any held notes before stopping
    if (transportIsRecording(dtc->transportState)) {
        finalizeHeldNotes(alg);
    }

    dtc->transportState = transportTransition_Stop(dtc->transportState);
    sendAllNotesOff(alg, where);

    for (int t = 0; t < alg->numTracks; t++) {
        TrackState* ts = &alg->trackStates[t];
        ts->step = 0;
        ts->clockCount = 0;

        for (int n = 0; n < 128; n++) {
            ts->activeNotes[n] = 0;
            ts->playing[n].active = false;
        }
        ts->activeVel = 0;

        ts->brownianPos = 1;
        ts->shufflePos = 1;
        ts->readCyclePos = 0;
    }

    for (int i = 0; i < MAX_DELAYED_NOTES; i++) {
        alg->delayedNotes[i].active = false;
    }

    dtc->stepTime = 0.0f;
}

// ============================================================================
// DELAYED NOTE PROCESSING (Humanization)
// ============================================================================

// Process delayed notes - decrement delays and send notes when ready
static void processDelayedNotes(MidiLooperAlgorithm* alg, float dt) {
    int delayDecrement = (int)(dt * 1000.0f);
    if (delayDecrement < 1) delayDecrement = 1;

    for (int i = 0; i < MAX_DELAYED_NOTES; i++) {
        DelayedNote* dn = &alg->delayedNotes[i];
        if (!dn->active) continue;

        if (dn->delay <= (uint16_t)delayDecrement) {
            NT_sendMidi3ByteMessage(dn->where, withChannel(kMidiNoteOn, dn->outCh), dn->note, dn->velocity);

            // Safe access - dn->track/note are stored values that could be invalid
            int track = safeTrackIndex(dn->track);
            int note = safeNoteIndex(dn->note);
            TrackState* ts = &alg->trackStates[track];
            ts->playing[note].active = true;
            ts->playing[note].remaining = dn->duration;
            ts->activeNotes[note] = dn->velocity;
            ts->activeVel = dn->velocity;

            dn->active = false;
        } else {
            dn->delay -= (uint16_t)delayDecrement;
        }
    }
}

// Schedule a note for delayed playback
// Returns true if note was scheduled, false if pool was full (note dropped)
static bool scheduleDelayedNote(MidiLooperAlgorithm* alg, uint8_t note, uint8_t velocity,
                                 uint8_t track, uint8_t outCh, uint16_t duration,
                                 uint16_t delay, uint32_t where) {
    for (int di = 0; di < MAX_DELAYED_NOTES; di++) {
        if (!alg->delayedNotes[di].active) {
            alg->delayedNotes[di].active = true;
            alg->delayedNotes[di].note = note;
            alg->delayedNotes[di].velocity = velocity;
            alg->delayedNotes[di].track = track;
            alg->delayedNotes[di].outCh = outCh;
            alg->delayedNotes[di].duration = duration;
            alg->delayedNotes[di].delay = delay;
            alg->delayedNotes[di].where = where;
            return true;  // Successfully scheduled
        }
    }
    // Pool exhausted - note will be dropped
    DEBUG_POOL_OVERFLOW("delayedNotes");
    return false;
}

// ============================================================================
// NOTE DURATION PROCESSING
// ============================================================================

// Process note duration countdowns for a track
static void processNoteDurations(MidiLooperAlgorithm* alg, int track, uint32_t where, int outCh) {
    TrackState* ts = &alg->trackStates[track];
    // track is bounded by caller's for loop (0 to numTracks-1)

    for (int n = 0; n < 128; n++) {
        PlayingNote* pn = &ts->playing[n];
        if (!pn->active) continue;

        if (pn->remaining <= 1) {
            NT_sendMidi3ByteMessage(where, withChannel(kMidiNoteOff, outCh), (uint8_t)n, 0);
            pn->active = false;
            ts->activeNotes[n] = 0;

            // Update activeVel if no more notes
            bool hasActive = false;
            for (int m = 0; m < 128; m++) {
                if (ts->activeNotes[m] > 0) {
                    hasActive = true;
                    break;
                }
            }
            if (!hasActive) {
                ts->activeVel = 0;
            }
        } else {
            pn->remaining--;
        }
    }
}

// ============================================================================
// STEP CALCULATION
// ============================================================================

// Calculate step position based on direction mode
static int calculateTrackStep(MidiLooperAlgorithm* alg, int track, int loopLen, int dir, int strideSize) {
    TrackState* ts = &alg->trackStates[track];

    if (dir == DIR_BROWNIAN) {
        if (ts->clockCount == 1) {
            ts->brownianPos = 1;
        } else {
            ts->brownianPos = (uint8_t)updateBrownianStep(ts->brownianPos, loopLen, alg->randState);
        }
        return ts->brownianPos;
    }

    if (dir == DIR_SHUFFLE) {
        if (ts->shufflePos > loopLen) {
            generateShuffleOrder(ts->shuffleOrder, loopLen, alg->randState);
            ts->shufflePos = 1;
        }
        // shufflePos is validated above (1 to loopLen), loopLen <= MAX_STEPS
        int step = ts->shuffleOrder[ts->shufflePos - 1];
        ts->shufflePos++;
        return step;
    }

    return getStepForClock(ts->clockCount, loopLen, dir, strideSize, alg->randState);
}

// ============================================================================
// PANIC / ALL NOTES OFF
// ============================================================================

// Clear all notes and state (panic)
static void handlePanicOnWrap(MidiLooperAlgorithm* alg, uint32_t where) {
    sendAllNotesOff(alg, where);

    for (int t = 0; t < alg->numTracks; t++) {
        TrackState* ts = &alg->trackStates[t];
        for (int n = 0; n < 128; n++) {
            ts->playing[n].active = false;
            ts->activeNotes[n] = 0;
        }
        ts->activeVel = 0;
    }

    for (int i = 0; i < MAX_DELAYED_NOTES; i++) {
        alg->delayedNotes[i].active = false;
    }
}

// ============================================================================
// NOTE EMISSION
// ============================================================================

// Play or schedule a single note
static void emitNote(MidiLooperAlgorithm* alg, int track, NoteEvent* ev,
                     int velOffset, int humanize, int outCh, uint32_t where) {
    TrackState* ts = &alg->trackStates[track];
    // track is bounded by caller, ev->note is 0-127 by MIDI spec
    int velocity = clamp((int)ev->velocity + velOffset, 0, 127);
    int delay = (humanize > 0) ? randRange(alg->randState, 0, humanize) : 0;

    if (delay == 0) {
        NT_sendMidi3ByteMessage(where, withChannel(kMidiNoteOn, outCh), ev->note, (uint8_t)velocity);
        ts->playing[ev->note].active = true;
        ts->playing[ev->note].remaining = ev->duration;
        ts->activeNotes[ev->note] = (uint8_t)velocity;
        ts->activeVel = (uint8_t)velocity;
    } else {
        scheduleDelayedNote(alg, ev->note, (uint8_t)velocity, (uint8_t)track,
                           (uint8_t)outCh, ev->duration, (uint16_t)delay, where);
    }
}

// Play all events for selected steps on a track
static void playTrackEvents(MidiLooperAlgorithm* alg, int track, int finalStep, int loopLen,
                            int velOffset, int humanize, int outCh, uint32_t where) {
    int stepsToPlay[MAX_STEPS];
    int numSteps = getStepsToEmit(alg, track, finalStep, loopLen, stepsToPlay);
    // track is bounded by caller's for loop, steps are 1-loopLen from getStepsToEmit

    for (int si = 0; si < numSteps; si++) {
        int stepIdx = stepsToPlay[si] - 1;
        if (stepIdx < 0 || stepIdx >= MAX_STEPS) continue;

        StepEvents* evs = &alg->trackStates[track].data.steps[stepIdx];

        for (int e = 0; e < evs->count; e++) {
            emitNote(alg, track, &evs->events[e], velOffset, humanize, outCh, where);
        }
    }
}

// ============================================================================
// TRACK PROCESSING
// ============================================================================

// ============================================================================
// STEP CALCULATION PIPELINE
// ============================================================================
//
// Step calculation proceeds through three stages in strict order:
//
// 1. BASE STEP (calculateTrackStep)
//    - Determines position from direction mode and clock count
//    - Stateful for Brownian/Shuffle modes (maintains walk position)
//    - Output: deterministic step based on playback direction
//
// 2. CONTINUOUS MODIFIERS (applyModifiers)
//    - Probability-based transformations applied to base step
//    - Order within: Stability -> Motion -> Randomness -> Gravity -> Pedal
//    - Each modifier may override or adjust the step independently
//    - Output: potentially modified step position
//
// 3. BINARY MODIFIERS (applyBinaryModifiers)
//    - Deterministic accept/reject filters
//    - No-Repeat: skips if same as previous cycle's final step
//    - Step Mask: finds next allowed step if current is masked
//    - Output: final step guaranteed to pass all filters
//
// IMPORTANT: This ordering ensures:
// - Direction intent is established before any modification
// - Chaos/probability modifiers transform that intent
// - Binary filters operate on the fully-modified result
// - lastStep comparison uses previous cycle's FINAL step, not base step
//

// Process a single track on clock trigger
static void processTrack(MidiLooperAlgorithm* alg, int track, uint32_t where, bool panicOnWrap) {
    TrackState* ts = &alg->trackStates[track];
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    int loopLen = tp.length();
    int outCh = tp.channel();

    // Process note durations first (independent of step calculation)
    processNoteDurations(alg, track, where, outCh);

    // Handle track enable/disable transitions
    bool enabled = tp.enabled();
    if (!enabled && ts->lastEnabled == 1) {
        sendTrackNotesOff(alg, track, where, outCh);
    }
    ts->lastEnabled = enabled ? 1 : 0;

    // Advance clock and save previous position for wrap detection
    ts->clockCount++;
    int prevPos = ts->step;

    // === STEP CALCULATION PIPELINE (see documentation above) ===
    // Stage 1: Base step from direction mode
    int baseStep = calculateTrackStep(alg, track, loopLen, tp.direction(), tp.strideSize());
    // Stage 2: Continuous probability-based modifiers
    int modifiedStep = applyModifiers(alg, track, baseStep, loopLen);
    // Stage 3: Binary accept/reject filters (uses lastStep from previous cycle)
    int finalStep = applyBinaryModifiers(alg, track, modifiedStep, ts->lastStep, loopLen);

    // Update state with final calculated step
    ts->lastStep = (uint8_t)finalStep;
    ts->step = (uint8_t)finalStep;

    // Check for loop wrap and trigger panic if configured
    bool wrapped = detectWrap(prevPos, finalStep, loopLen, tp.direction(), ts->clockCount);
    if (wrapped && panicOnWrap) {
        handlePanicOnWrap(alg, where);
    }

    // Emit notes for the calculated step(s)
    if (enabled) {
        playTrackEvents(alg, track, finalStep, loopLen, tp.velocity(), tp.humanize(), outCh, where);
    }
}
