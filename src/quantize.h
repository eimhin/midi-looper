/*
 * MIDI Looper - Quantization Utilities
 * Step quantization and snap functions for recording
 */

#pragma once

#include "types.h"

// Quantization calculations
int findValidQuantize(int loopLen, int targetQuantize);
int getEffectiveQuantize(const int16_t* v, int track, int& outLoopLen);
int getCachedQuantize(const int16_t* v, int track, TrackCache* cache, int& outLoopLen);

// Step snapping (for recording)
int snapStepSubclock(int rawStep, float stepFraction, float threshold, int loopLen);
int snapToDivisionSubclock(int rawStep, float stepFraction, int quantize, float threshold, int loopLen);

// Duration quantization
int calcQuantizedDuration(int duration, int quantize);
