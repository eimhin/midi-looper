/*
 * MIDI Looper - MIDI Output Helpers
 * Functions for sending MIDI messages and managing note state
 */

#pragma once

#include "types.h"

// MIDI output helpers
void sendAllNotesOff(MidiLooperAlgorithm* alg);
void sendTrackNotesOff(MidiLooperAlgorithm* alg, int track);
bool isNoteSharedByOtherTrack(MidiLooperAlgorithm* alg, int track, uint8_t note, uint8_t outCh, uint32_t where);

// Track event helpers
void clearTrackEvents(TrackData* track);
bool hasNoteEvent(const StepEvents* evs, uint8_t noteNum);
void addEvent(StepEvents* evs, uint8_t note, uint8_t velocity, uint16_t duration);
