/*
 * MIDI Looper - Direction Calculations
 * Step position calculations for all 12 playback direction modes
 * Uses Strategy pattern for extensibility
 */

#pragma once

#include "types.h"

// Strategy type definitions
typedef int (*DirectionStrategy)(int clockCount, int loopLen, int strideSize, uint32_t& randState);
typedef bool (*WrapDetector)(int prevPos, int currPos, int loopLen, int clockCount);

// Direction dispatch
int getStepForClock(int clockCount, int loopLen, int dir, int strideSize, uint32_t& randState);

// Stateful direction helpers
int updateBrownianStep(int currentPos, int loopLen, uint32_t& randState);
void generateShuffleOrder(uint8_t* order, int loopLen, uint32_t& randState);

// Wrap detection
bool detectWrap(int prevPos, int currPos, int loopLen, int dir, int clockCount);
