/*
 * MIDI Looper - Serialization
 *
 * Format: v1 (object-based, extensible)
 *
 * {
 *   "version": 1,
 *   "numTracks": 4,
 *   "tracks": [
 *     {
 *       "events": [
 *         [{"n": 60, "v": 100, "d": 48}, ...],  // step 0
 *         [],                                      // step 1 (empty)
 *         ...
 *       ],
 *       "shuffleOrder": [1, 2, 3, ...],
 *       "shufflePos": 1,
 *       "brownianPos": 1
 *     },
 *     ...
 *   ]
 * }
 *
 * EXTENDING THE FORMAT
 * --------------------
 * Unknown fields are skipped at every level (top-level, track, event), so
 * additive changes are always backward compatible without a version bump.
 *
 * Add a new event field (e.g., probability):
 *   Serialise:   stream.addMemberName("p"); stream.addNumber(ev.prob);
 *   Deserialise: else if (parse.matchName("p")) { parse.number(prob); }
 *   Old presets without "p" default to 0 — no migration needed.
 *
 * Add new per-track state (e.g., strideOffset):
 *   Serialise:   stream.addMemberName("strideOffset"); stream.addNumber(...);
 *   Deserialise: else if (parse.matchName("strideOffset")) { ... }
 *
 * Add new top-level state:
 *   Same pattern — add to serialiser, add matchName in main deserialise loop.
 *
 * Bump "version" only for structural changes that break the object layout
 * (e.g., renaming "tracks" or changing array nesting). Additive fields never
 * need a version bump.
 */

#include "serial.h"
#include "midi.h"

static const int SERIAL_VERSION = 1;

// ============================================================================
// SERIALIZATION
// ============================================================================

void serialiseData(MidiLooperAlgorithm* alg, _NT_jsonStream& stream) {
    int numTracks = alg->numTracks;

    stream.addMemberName("version");
    stream.addNumber(SERIAL_VERSION);

    stream.addMemberName("numTracks");
    stream.addNumber(numTracks);

    stream.addMemberName("tracks");
    stream.openArray();
    for (int t = 0; t < numTracks; t++) {
        TrackState& ts = alg->trackStates[t];

        stream.openObject();

        // Events: array of steps, each step is array of event objects
        stream.addMemberName("events");
        stream.openArray();
        for (int s = 0; s < MAX_STEPS; s++) {
            stream.openArray();
            StepEvents* evs = &ts.data.steps[s];
            for (int e = 0; e < evs->count && e < MAX_EVENTS_PER_STEP; e++) {
                stream.openObject();
                stream.addMemberName("n");
                stream.addNumber((int)evs->events[e].note);
                stream.addMemberName("v");
                stream.addNumber((int)evs->events[e].velocity);
                stream.addMemberName("d");
                stream.addNumber((int)evs->events[e].duration);
                stream.closeObject();
            }
            stream.closeArray();
        }
        stream.closeArray();

        // Shuffle order
        stream.addMemberName("shuffleOrder");
        stream.openArray();
        for (int s = 0; s < MAX_STEPS; s++) {
            stream.addNumber((int)ts.shuffleOrder[s]);
        }
        stream.closeArray();

        // Per-track playback state
        stream.addMemberName("shufflePos");
        stream.addNumber((int)ts.shufflePos);

        stream.addMemberName("brownianPos");
        stream.addNumber((int)ts.brownianPos);

        stream.closeObject();
    }
    stream.closeArray();
}

// ============================================================================
// DESERIALIZATION HELPERS
// ============================================================================

// Parse a single event object {"n":60,"v":100,"d":48}, skipping unknown fields.
// Returns false on parse error.
static bool parseEventObject(_NT_jsonParse& parse,
                             int& note, int& vel, int& dur) {
    note = 0;
    vel = 0;
    dur = 0;

    int numMembers;
    if (!parse.numberOfObjectMembers(numMembers)) return false;

    for (int i = 0; i < numMembers; i++) {
        if (parse.matchName("n")) {
            if (!parse.number(note)) return false;
        } else if (parse.matchName("v")) {
            if (!parse.number(vel)) return false;
        } else if (parse.matchName("d")) {
            if (!parse.number(dur)) return false;
        } else {
            if (!parse.skipMember()) return false;
        }
    }
    return true;
}

// Parse the events array for one track: array of steps, each step is array of
// event objects.
static bool parseTrackEvents(_NT_jsonParse& parse, TrackState& ts) {
    int numSteps;
    if (!parse.numberOfArrayElements(numSteps)) return false;

    for (int s = 0; s < numSteps; s++) {
        int numEvents;
        if (!parse.numberOfArrayElements(numEvents)) return false;

        if (s < MAX_STEPS)
            ts.data.steps[s].count = 0;

        for (int e = 0; e < numEvents; e++) {
            int note, vel, dur;
            if (!parseEventObject(parse, note, vel, dur)) return false;

            if (s < MAX_STEPS && e < MAX_EVENTS_PER_STEP &&
                note >= 0 && note <= 127 && vel >= 0 && vel <= 127 &&
                dur >= 1 && dur <= 65535) {
                addEvent(&ts.data.steps[s], (uint8_t)note, (uint8_t)vel,
                         (uint16_t)dur);
            }
        }
    }
    return true;
}

// Parse a shuffle order array for one track.
static bool parseShuffleOrderArray(_NT_jsonParse& parse, TrackState& ts) {
    int numSteps;
    if (!parse.numberOfArrayElements(numSteps)) return false;

    for (int s = 0; s < numSteps; s++) {
        int val;
        if (!parse.number(val)) return false;
        if (s < MAX_STEPS)
            ts.shuffleOrder[s] = (uint8_t)clampParam(val, 1, MAX_STEPS);
    }
    return true;
}

// Parse one track object: events, shuffleOrder, shufflePos, brownianPos,
// plus skip any unknown members.
static bool parseTrackObject(_NT_jsonParse& parse, TrackState& ts) {
    int numMembers;
    if (!parse.numberOfObjectMembers(numMembers)) return false;

    for (int i = 0; i < numMembers; i++) {
        if (parse.matchName("events")) {
            if (!parseTrackEvents(parse, ts)) return false;
        } else if (parse.matchName("shuffleOrder")) {
            if (!parseShuffleOrderArray(parse, ts)) return false;
        } else if (parse.matchName("shufflePos")) {
            int val;
            if (!parse.number(val)) return false;
            ts.shufflePos = (uint8_t)clampParam(val, 1, MAX_STEPS);
        } else if (parse.matchName("brownianPos")) {
            int val;
            if (!parse.number(val)) return false;
            ts.brownianPos = (uint8_t)clampParam(val, 1, MAX_STEPS);
        } else {
            if (!parse.skipMember()) return false;
        }
    }
    return true;
}

// Skip a track object we can't store (excess tracks beyond allocation).
static bool skipTrackObject(_NT_jsonParse& parse) {
    int numMembers;
    if (!parse.numberOfObjectMembers(numMembers)) return false;

    for (int i = 0; i < numMembers; i++) {
        if (!parse.skipMember()) return false;
    }
    return true;
}

// ============================================================================
// DESERIALIZATION
// ============================================================================

bool deserialiseData(MidiLooperAlgorithm* alg, _NT_jsonParse& parse) {
    int maxTracks = alg->numTracks;

    int numMembers;
    if (!parse.numberOfObjectMembers(numMembers)) return false;

    for (int m = 0; m < numMembers; m++) {
        if (parse.matchName("version")) {
            int version;
            if (!parse.number(version)) return false;
            (void)version;
        } else if (parse.matchName("numTracks")) {
            int savedTracks;
            if (!parse.number(savedTracks)) return false;
            (void)savedTracks;
        } else if (parse.matchName("tracks")) {
            int fileTracks;
            if (!parse.numberOfArrayElements(fileTracks)) return false;

            for (int t = 0; t < fileTracks; t++) {
                if (t < maxTracks) {
                    if (!parseTrackObject(parse, alg->trackStates[t]))
                        return false;
                } else {
                    if (!skipTrackObject(parse)) return false;
                }
            }
        } else {
            if (!parse.skipMember()) return false;
        }
    }

    return true;
}
