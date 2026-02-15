#include "quantize.h"

// ============================================================================
// QUANTIZATION CALCULATIONS
// ============================================================================

int findValidQuantize(int loopLen, int targetQuantize) {
    int maxQ = (targetQuantize < loopLen) ? targetQuantize : loopLen;
    for (int q = maxQ; q >= 1; q--) {
        if (loopLen % q == 0) return q;
    }
    return 1;
}

int getEffectiveQuantize(const int16_t* v, int track, int& outLoopLen) {
    TrackParams tp = TrackParams::fromAlgorithm(v, track);
    int loopLen = tp.length();
    int targetQuantize = QUANTIZE_VALUES[tp.division()];
    outLoopLen = loopLen;
    return findValidQuantize(loopLen, targetQuantize);
}

int getCachedQuantize(const int16_t* v, int track, TrackCache* cache, int& outLoopLen) {
    if (cache->dirty) {
        int loopLen;
        cache->effectiveQuantize = (uint8_t)getEffectiveQuantize(v, track, loopLen);
        cache->loopLen = (uint8_t)loopLen;
        cache->dirty = false;
    }
    outLoopLen = cache->loopLen;
    return cache->effectiveQuantize;
}

// ============================================================================
// STEP SNAPPING (for recording)
// ============================================================================

int snapStepSubclock(int rawStep, float stepFraction, float threshold, int loopLen) {
    if (stepFraction < threshold) return rawStep;
    int snapped = rawStep + 1;
    if (snapped > loopLen) return 1;
    return snapped;
}

int snapToDivisionSubclock(int rawStep, float stepFraction, int quantize, float threshold, int loopLen) {
    int stepInDivision = (rawStep - 1) % quantize;
    float divisionPosition = ((float)stepInDivision + stepFraction) / (float)quantize;
    int currentDivision = (rawStep - 1) / quantize;
    int quantizedStep = currentDivision * quantize + 1;

    if (divisionPosition >= threshold) {
        quantizedStep = (currentDivision + 1) * quantize + 1;
        if (quantizedStep > loopLen) quantizedStep = 1;
    }
    return quantizedStep;
}

// ============================================================================
// DURATION QUANTIZATION
// ============================================================================

int calcQuantizedDuration(int duration, int quantize) {
    if (quantize <= 1) return duration;
    int quantizedDur = ((duration + quantize / 2) / quantize) * quantize;
    return (quantizedDur < quantize) ? quantize : quantizedDur;
}
