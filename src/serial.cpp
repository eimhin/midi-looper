#include "serial.h"
#include "midi.h"

// ============================================================================
// SERIALIZATION
// ============================================================================

// Version history:
// 1 - Initial format (implicit, no version field in file)
// 2 - Added explicit version field
static const int SERIAL_VERSION = 2;

void serialiseData(MidiLooperAlgorithm* alg, _NT_jsonStream& stream) {
    int numTracks = alg->numTracks;

    // Version for future migration support
    stream.addMemberName("version");
    stream.addNumber(SERIAL_VERSION);

    // Save numTracks for forward compatibility
    stream.addMemberName("numTracks");
    stream.addNumber(numTracks);

    // Tracks (only save active tracks)
    stream.addMemberName("tracks");
    stream.openArray();
    for (int t = 0; t < numTracks; t++) {
        stream.openObject();
        stream.addMemberName("events");
        stream.openArray();
        for (int s = 0; s < MAX_STEPS; s++) {
            stream.openArray();
            StepEvents* evs = &alg->trackStates[t].data.steps[s];
            for (int e = 0; e < evs->count && e < MAX_EVENTS_PER_STEP; e++) {
                stream.openArray();
                stream.addNumber((int)evs->events[e].note);
                stream.addNumber((int)evs->events[e].velocity);
                stream.addNumber((int)evs->events[e].duration);
                stream.closeArray();
            }
            stream.closeArray();
        }
        stream.closeArray();
        stream.closeObject();
    }
    stream.closeArray();

    // Shuffle order (save all tracks)
    stream.addMemberName("shuffleOrder");
    stream.openArray();
    for (int t = 0; t < numTracks; t++) {
        stream.openArray();
        for (int s = 0; s < MAX_STEPS; s++) {
            stream.addNumber((int)alg->trackStates[t].shuffleOrder[s]);
        }
        stream.closeArray();
    }
    stream.closeArray();

    // State arrays (save all tracks)
    stream.addMemberName("shufflePos");
    stream.openArray();
    for (int t = 0; t < numTracks; t++) {
        stream.addNumber((int)alg->trackStates[t].shufflePos);
    }
    stream.closeArray();

    stream.addMemberName("brownianPos");
    stream.openArray();
    for (int t = 0; t < numTracks; t++) {
        stream.addNumber((int)alg->trackStates[t].brownianPos);
    }
    stream.closeArray();
}

bool deserialiseData(MidiLooperAlgorithm* alg, _NT_jsonParse& parse) {
    int maxTracks = alg->numTracks; // Only load tracks we have allocated
    int version = 1;                // Default for legacy data without version field

    int numMembers;
    if (!parse.numberOfObjectMembers(numMembers)) return false;

    for (int m = 0; m < numMembers; m++) {
        if (parse.matchName("version")) {
            if (!parse.number(version)) return false;
            // Future: Add migration logic here when format changes
            // if (version < SERIAL_VERSION) { migrate... }
        } else if (parse.matchName("numTracks")) {
            int savedTracks;
            if (!parse.number(savedTracks)) return false;
            // Ignore the saved value, we use our current allocation
            (void)savedTracks;
        } else if (parse.matchName("tracks")) {
            int fileTracks;
            if (!parse.numberOfArrayElements(fileTracks)) return false;

            // Only load up to alg->numTracks (DRAM allocation limit)
            for (int t = 0; t < fileTracks && t < maxTracks; t++) {
                int numTrackMembers;
                if (!parse.numberOfObjectMembers(numTrackMembers)) return false;

                for (int tm = 0; tm < numTrackMembers; tm++) {
                    if (parse.matchName("events")) {
                        int numSteps;
                        if (!parse.numberOfArrayElements(numSteps)) return false;

                        for (int s = 0; s < numSteps; s++) {
                            int numEvents;
                            if (!parse.numberOfArrayElements(numEvents)) return false;

                            if (s < MAX_STEPS)
                                alg->trackStates[t].data.steps[s].count = 0;

                            for (int e = 0; e < numEvents; e++) {
                                int numFields;
                                if (!parse.numberOfArrayElements(numFields)) return false;

                                int note = 0, vel = 0, dur = 0;
                                for (int f = 0; f < numFields; f++) {
                                    int val;
                                    if (!parse.number(val)) return false;
                                    if (f == 0) note = val;
                                    else if (f == 1) vel = val;
                                    else if (f == 2) dur = val;
                                }

                                if (s < MAX_STEPS && e < MAX_EVENTS_PER_STEP &&
                                    note >= 0 && note <= 127 && vel >= 0 && vel <= 127 && dur >= 1) {
                                    addEvent(&alg->trackStates[t].data.steps[s], (uint8_t)note, (uint8_t)vel,
                                             (uint16_t)dur);
                                }
                            }
                        }
                    } else {
                        if (!parse.skipMember()) return false;
                    }
                }
            }
            // Skip any extra tracks in the file beyond our allocation
            for (int t = maxTracks; t < fileTracks; t++) {
                int numTrackMembers;
                if (!parse.numberOfObjectMembers(numTrackMembers)) return false;
                for (int tm = 0; tm < numTrackMembers; tm++) {
                    if (!parse.skipMember()) return false;
                }
            }
        } else if (parse.matchName("shuffleOrder")) {
            int fileTracks;
            if (!parse.numberOfArrayElements(fileTracks)) return false;

            // Only load up to maxTracks (allocated TrackState count)
            for (int t = 0; t < fileTracks && t < maxTracks; t++) {
                int numSteps;
                if (!parse.numberOfArrayElements(numSteps)) return false;

                for (int s = 0; s < numSteps; s++) {
                    int val;
                    if (!parse.number(val)) return false;
                    if (s < MAX_STEPS)
                        alg->trackStates[t].shuffleOrder[s] = (uint8_t)clampParam(val, 1, MAX_STEPS);
                }
            }
            // Skip any extra tracks in the file beyond our allocation
            for (int t = maxTracks; t < fileTracks; t++) {
                int numSteps;
                if (!parse.numberOfArrayElements(numSteps)) return false;
                for (int s = 0; s < numSteps; s++) {
                    int val;
                    if (!parse.number(val)) return false;
                }
            }
        } else if (parse.matchName("shufflePos")) {
            int num;
            if (!parse.numberOfArrayElements(num)) return false;
            for (int t = 0; t < num && t < maxTracks; t++) {
                int val;
                if (!parse.number(val)) return false;
                alg->trackStates[t].shufflePos = (uint8_t)clampParam(val, 1, MAX_STEPS);
            }
            for (int t = maxTracks; t < num; t++) {
                int val;
                if (!parse.number(val)) return false;
            }
        } else if (parse.matchName("brownianPos")) {
            int num;
            if (!parse.numberOfArrayElements(num)) return false;
            for (int t = 0; t < num && t < maxTracks; t++) {
                int val;
                if (!parse.number(val)) return false;
                alg->trackStates[t].brownianPos = (uint8_t)clampParam(val, 1, MAX_STEPS);
            }
            for (int t = maxTracks; t < num; t++) {
                int val;
                if (!parse.number(val)) return false;
            }
        } else {
            if (!parse.skipMember()) return false;
        }
    }

    return true;
}
