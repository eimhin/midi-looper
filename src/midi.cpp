#include "midi.h"
#include "midi_utils.h"

// ============================================================================
// MIDI OUTPUT HELPERS
// ============================================================================

void sendAllNotesOff(MidiLooperAlgorithm* alg) {
    for (int t = 0; t < alg->numTracks; t++) {
        TrackParams tp = TrackParams::fromAlgorithm(alg->v, t);
        int ch = tp.channel();
        uint32_t where = destToWhere(tp.destination());
        NT_sendMidi3ByteMessage(where, withChannel(kMidiCC, ch), 123, 0);
    }
}

void sendTrackNotesOff(MidiLooperAlgorithm* alg, int track) {
    TrackState* ts = &alg->trackStates[track];

    for (int n = 0; n < 128; n++) {
        PlayingNote* pn = &ts->playing[n];
        if (pn->active) {
            if (!isNoteSharedByOtherTrack(alg, track, (uint8_t)n, pn->outCh, pn->where)) {
                NT_sendMidi3ByteMessage(pn->where, withChannel(kMidiNoteOff, pn->outCh), (uint8_t)n, 0);
            }
        }
        ts->activeNotes[n] = 0;
        pn->active = false;
    }
    ts->activeVel = 0;

    // Cancel any pending delayed notes for this track
    for (int i = 0; i < MAX_DELAYED_NOTES; i++) {
        if (alg->delayedNotes[i].active && alg->delayedNotes[i].track == (uint8_t)track) {
            alg->delayedNotes[i].active = false;
        }
    }
}

bool isNoteSharedByOtherTrack(MidiLooperAlgorithm* alg, int track, uint8_t note, uint8_t outCh, uint32_t where) {
    for (int t = 0; t < alg->numTracks; t++) {
        if (t == track) continue;
        PlayingNote* pn = &alg->trackStates[t].playing[note];
        if (pn->active && pn->outCh == outCh && pn->where == where) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// TRACK EVENT HELPERS
// ============================================================================

void clearTrackEvents(TrackData* track) {
    for (int s = 0; s < MAX_STEPS; s++) {
        track->steps[s].count = 0;
    }
}

bool hasNoteEvent(const StepEvents* evs, uint8_t noteNum) {
    for (int i = 0; i < evs->count; i++) {
        if (evs->events[i].note == noteNum) return true;
    }
    return false;
}

void addEvent(StepEvents* evs, uint8_t note, uint8_t velocity, uint16_t duration) {
    if (evs->count < MAX_EVENTS_PER_STEP && !hasNoteEvent(evs, note)) {
        evs->events[evs->count].note = note;
        evs->events[evs->count].velocity = velocity;
        evs->events[evs->count].duration = duration;
        evs->count++;
    }
}
