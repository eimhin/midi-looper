#include "playback.h"
#include "math.h"
#include "midi.h"
#include "midi_utils.h"
#include "directions.h"
#include "modifiers.h"
#include "recording.h"
#include "random.h"
#include "scales.h"

// ============================================================================
// TRIG CONDITION EVALUATION
// ============================================================================

static bool evaluateTrigCondition(int cond, uint16_t loopCount, bool fillActive) {
    // Lookup tables for A:B ratio conditions (periods 2-8)
    // Maps ratio index (0-34) to period and position
    static const uint8_t ratioPeriod[] = {
        2, 2,
        3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 8
    };
    static const uint8_t ratioPos[] = {
        0, 1,
        0, 1, 2,
        0, 1, 2, 3,
        0, 1, 2, 3, 4,
        0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 5, 6,
        0, 1, 2, 3, 4, 5, 6, 7
    };
    static constexpr int NUM_RATIOS = 35;

    if (cond == 0) return true;  // Always

    // Positive A:B ratios (indices 1-35)
    if (cond <= NUM_RATIOS) {
        int idx = cond - 1;
        return (loopCount % ratioPeriod[idx]) == ratioPos[idx];
    }

    // NOT A:B ratios (indices 36-70)
    if (cond <= NUM_RATIOS * 2) {
        int idx = cond - NUM_RATIOS - 1;
        return (loopCount % ratioPeriod[idx]) != ratioPos[idx];
    }

    // Special conditions (71-75)
    switch (cond) {
    case 71: return loopCount == 0;   // First
    case 72: return loopCount != 0;   // !First
    case 73: return fillActive;       // Fill
    case 74: return !fillActive;      // !Fill
    case COND_FIXED: return true;     // Fixed (semantics handled in processTrack)
    default: return true;
    }
}

// ============================================================================
// TRANSPORT CONTROL
// ============================================================================

// Reset all tracks and start transport
void handleTransportStart(MidiLooperAlgorithm* alg) {
    MidiLooper_DTC* dtc = alg->dtc;

    for (int t = 0; t < alg->numTracks; t++) {
        TrackState* ts = &alg->trackStates[t];
        ts->step = 0;
        ts->clockCount = 0;
        ts->divCounter = 0;
        ts->loopCount = 0;
        ts->lastStep = 1;
        ts->brownianPos = 1;
        ts->shufflePos = 1;
        ts->octavePlayCount = 0;

        for (int s = 0; s < MAX_STEPS; s++) {
            ts->shuffleOrder[s] = (uint8_t)(s + 1);
        }
    }
    dtc->stepTime = 0.0f;
    dtc->transportState = transportTransition_Start(dtc->transportState);

    // Promote pending live recording now that transport is running
    if (dtc->recordState == REC_LIVE_PENDING) {
        int recTrack = clampParam(alg->v[kParamRecTrack], 0, alg->numTracks - 1);
        if (alg->v[kParamRecMode] == REC_MODE_REPLACE) {
            clearTrackEvents(&alg->trackStates[recTrack].data);
        }
        dtc->recordState = REC_LIVE;
    }
}

// Stop transport and clear all note state
void handleTransportStop(MidiLooperAlgorithm* alg) {
    MidiLooper_DTC* dtc = alg->dtc;

    // Finalize any held notes before stopping
    if (dtc->recordState == REC_LIVE) {
        finalizeHeldNotes(alg);
        dtc->recordState = REC_IDLE;
    }

    dtc->transportState = transportTransition_Stop(dtc->transportState);
    sendAllNotesOff(alg);

    for (int t = 0; t < alg->numTracks; t++) {
        TrackState* ts = &alg->trackStates[t];
        ts->step = 0;
        ts->clockCount = 0;
        ts->divCounter = 0;
        ts->loopCount = 0;

        for (int n = 0; n < 128; n++) {
            ts->activeNotes[n] = 0;
            ts->playing[n].active = false;
        }
        ts->activeVel = 0;

        ts->brownianPos = 1;
        ts->shufflePos = 1;
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
void processDelayedNotes(MidiLooperAlgorithm* alg, float dt) {
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
static int calculateTrackStep(MidiLooperAlgorithm* alg, int track, int loopLen, int dir) {
    TrackState* ts = &alg->trackStates[track];

    if (dir == DIR_BROWNIAN) {
        if (ts->clockCount == 1) {
            ts->brownianPos = 1;
        } else {
            ts->brownianPos = (uint8_t)updateBrownianStep(ts->brownianPos, loopLen, ts->randState);
        }
        return ts->brownianPos;
    }

    if (dir == DIR_SHUFFLE) {
        if (ts->shufflePos > loopLen) {
            generateShuffleOrder(ts->shuffleOrder, loopLen, ts->randState);
            ts->shufflePos = 1;
        }
        // shufflePos is validated above (1 to loopLen), loopLen <= MAX_STEPS
        int step = ts->shuffleOrder[ts->shufflePos - 1];
        ts->shufflePos++;
        return step;
    }

    return getStepForClock(ts->clockCount, loopLen, dir, ts->randState);
}

// ============================================================================
// PANIC / ALL NOTES OFF
// ============================================================================

// Clear all notes and state (panic)
static void handlePanicOnWrap(MidiLooperAlgorithm* alg) {
    sendAllNotesOff(alg);

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
// OCTAVE JUMP
// ============================================================================

// Calculate pitch shift (in semitones) for octave jump feature
// Called once per step trigger — all notes in the step get the same shift
static int calculateOctaveJump(MidiLooperAlgorithm* alg, int track, TrackParams& tp) {
    int octMin = tp.octMin();
    int octMax = tp.octMax();

    // Feature inactive when both ranges are 0
    if (octMin == 0 && octMax == 0) return 0;

    TrackState* ts = &alg->trackStates[track];
    ts->octavePlayCount++;

    // Bypass: every Nth note-play is unshifted
    int bypass = tp.octBypass();
    if (bypass > 0 && (ts->octavePlayCount % bypass) == 0)
        return 0;

    // Probability check
    int prob = tp.octProb();
    if (randFloat(ts->randState) * 100.0f < (float)prob) {
        int octave = randRange(ts->randState, octMin, octMax);
        return octave * 12;
    }

    return 0;
}

// ============================================================================
// NOTE EMISSION
// ============================================================================

// Play or schedule a single note
static void emitNote(MidiLooperAlgorithm* alg, int track, NoteEvent* ev,
                     int velOffset, int humanize, int outCh, uint32_t where,
                     int noteShift) {
    TrackState* ts = &alg->trackStates[track];
    int actualNote = clamp((int)ev->note + noteShift, 0, 127);
    int scaleRoot = alg->v[kParamScaleRoot];
    int scaleType = alg->v[kParamScaleType];
    actualNote = quantizeToScale((uint8_t)actualNote, scaleRoot, scaleType);
    int velocity = clamp((int)ev->velocity + velOffset, 0, 127);
    int delay = (humanize > 0) ? randRange(ts->randState, 0, humanize) : 0;

    if (delay == 0) {
        NT_sendMidi3ByteMessage(where, withChannel(kMidiNoteOn, outCh), (uint8_t)actualNote, (uint8_t)velocity);
        ts->playing[actualNote].active = true;
        ts->playing[actualNote].remaining = ev->duration;
        ts->activeNotes[actualNote] = (uint8_t)velocity;
        ts->activeVel = (uint8_t)velocity;
    } else {
        scheduleDelayedNote(alg, (uint8_t)actualNote, (uint8_t)velocity, (uint8_t)track,
                           (uint8_t)outCh, ev->duration, (uint16_t)delay, where);
    }
}

// Play all events for the selected step on a track
static void playTrackEvents(MidiLooperAlgorithm* alg, int track, int finalStep,
                            TrackParams& tp, int velOffset, int humanize,
                            int outCh, uint32_t where, bool fixed) {
    int stepIdx = finalStep - 1;
    if (stepIdx < 0 || stepIdx >= MAX_STEPS) return;

    StepEvents* evs = &alg->trackStates[track].data.steps[stepIdx];
    if (evs->count == 0) return;

    int noteShift = fixed ? 0 : calculateOctaveJump(alg, track, tp);

    for (int e = 0; e < evs->count; e++) {
        emitNote(alg, track, &evs->events[e], velOffset, humanize, outCh, where, noteShift);
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
//    - Order within: Stability -> Motion -> Randomness -> Pedal
//    - Each modifier may override or adjust the step independently
//    - Output: potentially modified step position
//
// 3. BINARY MODIFIERS (applyBinaryModifiers)
//    - Deterministic accept/reject filters
//    - No-Repeat: skips if same as previous cycle's final step
//    - Output: final step guaranteed to pass all filters
//
// IMPORTANT: This ordering ensures:
// - Direction intent is established before any modification
// - Chaos/probability modifiers transform that intent
// - Binary filters operate on the fully-modified result
// - lastStep comparison uses previous cycle's FINAL step, not base step
//

// Process a single track on clock trigger
void processTrack(MidiLooperAlgorithm* alg, int track, bool panicOnWrap) {
    TrackState* ts = &alg->trackStates[track];
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    int loopLen = tp.length();
    int outCh = tp.channel();
    uint32_t where = destToWhere(tp.destination());

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
    int baseStep = calculateTrackStep(alg, track, loopLen, tp.direction());
    // Stage 2: Continuous probability-based modifiers
    int modifiedStep = applyModifiers(alg, track, baseStep, loopLen);
    // Stage 3: Binary accept/reject filters (uses lastStep from previous cycle)
    int finalStep = applyBinaryModifiers(alg, track, modifiedStep, ts->lastStep, loopLen);

    // Update state with final calculated step
    ts->lastStep = (uint8_t)finalStep;
    ts->step = (uint8_t)finalStep;

    // Check for loop wrap and trigger panic if configured
    bool wrapped = detectWrap(prevPos, finalStep, loopLen, tp.direction(), ts->clockCount);
    if (wrapped && ts->clockCount > 1) {
        ts->loopCount++;
    }
    if (wrapped && panicOnWrap) {
        handlePanicOnWrap(alg);
    }

    // Emit notes for the calculated step(s), gated by trig conditions
    if (enabled) {
        bool fillActive = (alg->v[kParamFill] == 1);

        // Per-track condition gates the entire track
        if (evaluateTrigCondition(tp.stepCond(), ts->loopCount, fillActive)) {
            // Per-step conditions target specific steps
            bool stepCondMet = true;
            int condStepA = tp.condStepA();
            int condStepB = tp.condStepB();
            if (condStepA > 0 && finalStep == condStepA) {
                stepCondMet = evaluateTrigCondition(tp.condA(), ts->loopCount, fillActive);
            }
            if (condStepB > 0 && finalStep == condStepB) {
                stepCondMet = evaluateTrigCondition(tp.condB(), ts->loopCount, fillActive);
            }

            if (stepCondMet) {
                // Determine if Fixed condition applies to this step
                bool fixed = (tp.stepCond() == COND_FIXED);
                if (condStepA > 0 && finalStep == condStepA && tp.condA() == COND_FIXED) fixed = true;
                if (condStepB > 0 && finalStep == condStepB && tp.condB() == COND_FIXED) fixed = true;

                // Step probability gate (bypassed by Fixed)
                int prob = tp.stepProb();
                if (condStepA > 0 && finalStep == condStepA) prob = tp.probA();
                if (condStepB > 0 && finalStep == condStepB) prob = tp.probB();
                if (fixed) prob = 100;

                if (prob >= 100 || (int)(randFloat(ts->randState) * 100.0f) < prob) {
                    playTrackEvents(alg, track, finalStep, tp, tp.velocity(), tp.humanize(), outCh, where, fixed);
                }
            }
        }
    }
}
