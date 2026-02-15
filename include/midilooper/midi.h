/*
 * MIDI Looper - MIDI Output Helpers
 * Functions for sending MIDI messages and managing note state
 */

#pragma once

#include "midilooper/config.h"
#include "midilooper/types.h"
#include "midilooper/utils.h"

// ============================================================================
// MIDI OUTPUT HELPERS
// ============================================================================

static void sendAllNotesOff(MidiLooperAlgorithm* alg, uint32_t where) {
    bool sentChannels[16] = {false};

    for (int t = 0; t < alg->numTracks; t++) {
        TrackParams tp = TrackParams::fromAlgorithm(alg->v, t);
        int ch = tp.channel();
        if (!sentChannels[ch - 1]) {
            NT_sendMidi3ByteMessage(where, withChannel(kMidiCC, ch), 123, 0);
            sentChannels[ch - 1] = true;
        }
    }
}

static void sendTrackNotesOff(MidiLooperAlgorithm* alg, int track, uint32_t where, int outCh) {
    TrackState* ts = &alg->trackStates[track];
    // track is bounded by caller (for loop or clamped parameter)

    for (int n = 0; n < 128; n++) {
        if (ts->activeNotes[n] > 0) {
            NT_sendMidi3ByteMessage(where, withChannel(kMidiNoteOff, outCh), (uint8_t)n, 0);
        }
        ts->activeNotes[n] = 0;
        ts->playing[n].active = false;
    }
    ts->activeVel = 0;
}

// ============================================================================
// TRACK EVENT HELPERS
// ============================================================================

static void clearTrackEvents(TrackData* track) {
    for (int s = 0; s < MAX_STEPS; s++) {
        track->steps[s].count = 0;
    }
}

static bool hasNoteEvent(const StepEvents* evs, uint8_t noteNum) {
    for (int i = 0; i < evs->count; i++) {
        if (evs->events[i].note == noteNum) return true;
    }
    return false;
}

static void addEvent(StepEvents* evs, uint8_t note, uint8_t velocity, uint16_t duration) {
    if (evs->count < MAX_EVENTS_PER_STEP) {
        evs->events[evs->count].note = note;
        evs->events[evs->count].velocity = velocity;
        evs->events[evs->count].duration = duration;
        evs->count++;
    }
}
