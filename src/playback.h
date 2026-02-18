/*
 * MIDI Looper - Playback Subsystem
 * Transport control, note scheduling, and step processing
 */

#pragma once

#include "types.h"

// Transport control
void handleTransportStart(MidiLooperAlgorithm* alg);
void handleTransportStop(MidiLooperAlgorithm* alg);

// Delayed note processing
void processDelayedNotes(MidiLooperAlgorithm* alg, float dt);

// Track processing
void processTrack(MidiLooperAlgorithm* alg, int track, bool panicOnWrap);
