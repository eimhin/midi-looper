/*
 * MIDI Looper - Modifiers
 * Step modification algorithms
 */

#pragma once

#include "types.h"

// Continuous modifiers
int applyModifiers(MidiLooperAlgorithm* alg, int track, int baseStep, int loopLen);

// Binary modifiers
int applyBinaryModifiers(MidiLooperAlgorithm* alg, int track, int step, int prevStep, int loopLen);
