/*
 * MIDI Looper - Random Sequence Generator
 * Algorithmic pattern generation and transformation
 */

#include "generate.h"
#include "math.h"
#include "midi.h"
#include "midi_utils.h"
#include "quantize.h"
#include "random.h"
#include "scales.h"

// ============================================================================
// MODE: NEW - Generate fresh monophonic pattern
// ============================================================================

static void generateNew(MidiLooperAlgorithm* alg, int track) {
    const int16_t* v = alg->v;
    TrackState* ts = &alg->trackStates[track];

    int density = v[kParamGenDensity];
    int bias = v[kParamGenBias];
    int range = v[kParamGenRange];
    int noteRand = v[kParamGenNoteRand];
    int velVar = v[kParamGenVelVar];
    int ties = v[kParamGenTies];
    int gateRand = v[kParamGenGateRand];
    int scaleRoot = v[kParamScaleRoot];
    int scaleType = v[kParamScaleType];

    int loopLen;
    int quantize = getCachedQuantize(v, track, &ts->cache, loopLen);

    clearTrackEvents(&ts->data);

    for (int s = 1; s <= loopLen; s++) {
        // Only place notes on division boundaries
        if (quantize > 1 && ((s - 1) % quantize) != 0) continue;

        // Density roll
        if (randRange(ts->randState, 1, 100) > density) continue;

        // Generate note: bias +/- (range * noteRand / 100)
        int spread = (range * noteRand) / 100;
        int note;
        if (spread > 0) {
            note = bias + randRange(ts->randState, -spread, spread);
        } else {
            note = bias;
        }
        note = clamp(note, 0, 127);
        note = quantizeToScale((uint8_t)note, scaleRoot, scaleType);

        // Velocity: centered around 100, varied by velVar
        int velSpread = (100 * velVar) / 200; // half-range
        int vel = 100;
        if (velSpread > 0) {
            vel = 100 + randRange(ts->randState, -velSpread, velSpread);
        }
        vel = clamp(vel, 1, 127);

        // Duration: base is quantize unit, randomly shortened by gateRand %
        int maxDur = (quantize > 1) ? quantize : 1;
        int minDur = maxDur - (maxDur * gateRand) / 100;
        if (minDur < 1) minDur = 1;
        int durVal = (minDur < maxDur) ? randRange(ts->randState, minDur, maxDur) : maxDur;
        uint16_t dur = (uint16_t)durVal;

        int idx = safeStepIndex(s - 1);
        addEvent(&ts->data.steps[idx], (uint8_t)note, (uint8_t)vel, dur);
    }

    // Pass 2: Ties - extend note duration to reach the next note
    if (ties > 0) {
        for (int s = 0; s < loopLen; s++) {
            StepEvents* evs = &ts->data.steps[s];
            if (evs->count == 0) continue;
            if (randRange(ts->randState, 1, 100) > ties) continue;

            // Scan forward (wrapping) to find next occupied step
            int dist = 0;
            for (int d = 1; d <= loopLen - 1; d++) {
                int nextIdx = (s + d) % loopLen;
                if (ts->data.steps[nextIdx].count > 0) {
                    dist = d;
                    break;
                }
            }
            if (dist == 0) continue;  // Only note in loop, skip

            // Extend all events on this step to reach the next note
            for (int e = 0; e < evs->count; e++) {
                evs->events[e].duration = (uint16_t)dist;
            }
        }
    }
}

// ============================================================================
// MODE: REORDER - Shuffle note positions (Fisher-Yates)
// ============================================================================

static void generateReorder(MidiLooperAlgorithm* alg, int track) {
    TrackState* ts = &alg->trackStates[track];

    int loopLen;
    getCachedQuantize(alg->v, track, &ts->cache, loopLen);

    // Collect all notes into a temporary buffer
    struct CollectedNote {
        uint8_t note;
        uint8_t velocity;
        uint16_t duration;
    };
    CollectedNote collected[128];
    int count = 0;

    for (int s = 0; s < loopLen && count < 128; s++) {
        StepEvents* evs = &ts->data.steps[s];
        for (int e = 0; e < evs->count && count < 128; e++) {
            collected[count].note = evs->events[e].note;
            collected[count].velocity = evs->events[e].velocity;
            collected[count].duration = evs->events[e].duration;
            count++;
        }
    }

    if (count == 0) return;

    // Collect the occupied step positions (preserving rhythm pattern)
    int positions[128];
    int posCount = 0;
    for (int s = 0; s < loopLen && posCount < 128; s++) {
        if (ts->data.steps[s].count > 0) {
            positions[posCount++] = s;
        }
    }

    // Fisher-Yates shuffle the notes
    for (int i = count - 1; i > 0; i--) {
        int j = randRange(ts->randState, 0, i);
        CollectedNote tmp = collected[i];
        collected[i] = collected[j];
        collected[j] = tmp;
    }

    // Clear and redistribute
    clearTrackEvents(&ts->data);
    int noteIdx = 0;
    for (int p = 0; p < posCount && noteIdx < count; p++) {
        int s = positions[p];
        addEvent(&ts->data.steps[s], collected[noteIdx].note, collected[noteIdx].velocity, collected[noteIdx].duration);
        noteIdx++;
    }
}

// ============================================================================
// MODE: RE-PITCH - Replace note values, keep rhythm
// ============================================================================

static void generateRepitch(MidiLooperAlgorithm* alg, int track) {
    const int16_t* v = alg->v;
    TrackState* ts = &alg->trackStates[track];

    int bias = v[kParamGenBias];
    int range = v[kParamGenRange];
    int noteRand = v[kParamGenNoteRand];
    int scaleRoot = v[kParamScaleRoot];
    int scaleType = v[kParamScaleType];

    int loopLen;
    getCachedQuantize(v, track, &ts->cache, loopLen);

    int spread = (range * noteRand) / 100;

    for (int s = 0; s < loopLen; s++) {
        StepEvents* evs = &ts->data.steps[s];
        for (int e = 0; e < evs->count; e++) {
            int note;
            if (spread > 0) {
                note = bias + randRange(ts->randState, -spread, spread);
            } else {
                note = bias;
            }
            note = clamp(note, 0, 127);
            note = quantizeToScale((uint8_t)note, scaleRoot, scaleType);
            evs->events[e].note = (uint8_t)note;
        }
    }
}

// ============================================================================
// MODE: INVERT - Reverse step sequence in-place
// ============================================================================

static void generateInvert(MidiLooperAlgorithm* alg, int track) {
    TrackState* ts = &alg->trackStates[track];

    int loopLen;
    getCachedQuantize(alg->v, track, &ts->cache, loopLen);

    int left = 0;
    int right = loopLen - 1;
    while (left < right) {
        // Swap steps[left] and steps[right]
        StepEvents tmp = ts->data.steps[left];
        ts->data.steps[left] = ts->data.steps[right];
        ts->data.steps[right] = tmp;

        // Clamp durations to remaining loop space from new position
        for (int e = 0; e < ts->data.steps[left].count; e++) {
            uint16_t maxDur = (uint16_t)(loopLen - left);
            if (ts->data.steps[left].events[e].duration > maxDur) {
                ts->data.steps[left].events[e].duration = maxDur;
            }
        }
        for (int e = 0; e < ts->data.steps[right].count; e++) {
            uint16_t maxDur = (uint16_t)(loopLen - right);
            if (ts->data.steps[right].events[e].duration > maxDur) {
                ts->data.steps[right].events[e].duration = maxDur;
            }
        }

        left++;
        right--;
    }
}

// ============================================================================
// ENTRY POINT
// ============================================================================

void executeGenerate(MidiLooperAlgorithm* alg, int track) {
    if (track < 0 || track >= alg->numTracks) return;

    sendTrackNotesOff(alg, track);

    int mode = alg->v[kParamGenMode];
    switch (mode) {
    case GEN_MODE_NEW:
        generateNew(alg, track);
        break;
    case GEN_MODE_REORDER:
        generateReorder(alg, track);
        break;
    case GEN_MODE_REPITCH:
        generateRepitch(alg, track);
        break;
    case GEN_MODE_INVERT:
        generateInvert(alg, track);
        break;
    }
}
