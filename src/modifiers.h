/*
 * MIDI Looper - Modifiers and Read Modes
 * Step modification algorithms and multi-step read modes
 */

#pragma once

#include "types.h"

// Continuous modifiers
int applyModifiers(MidiLooperAlgorithm* alg, int track, int baseStep, int loopLen);

// Binary modifiers
int applyBinaryModifiers(MidiLooperAlgorithm* alg, int track, int step, int prevStep, int loopLen);

// Read modes
int getStepsToEmit(MidiLooperAlgorithm* alg, int track, int selectedStep, int loopLen, int* outSteps);
