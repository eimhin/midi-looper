#include "directions.h"
#include "random.h"

// ============================================================================
// DIRECTION STRATEGY IMPLEMENTATIONS
// ============================================================================

static int dirForward(int clockCount, int loopLen, int, uint32_t&) {
    return ((clockCount - 1) % loopLen) + 1;
}

static int dirReverse(int clockCount, int loopLen, int, uint32_t&) {
    return loopLen - ((clockCount - 1) % loopLen);
}

static int dirPendulum(int clockCount, int loopLen, int, uint32_t&) {
    int cycle = 2 * (loopLen - 1);
    int posInCycle = (clockCount - 1) % cycle;
    if (posInCycle < loopLen) {
        return posInCycle + 1;
    } else {
        return 2 * loopLen - 1 - posInCycle;
    }
}

static int dirPingPong(int clockCount, int loopLen, int, uint32_t&) {
    int cycle = 2 * loopLen;
    int posInCycle = (clockCount - 1) % cycle;
    if (posInCycle < loopLen) {
        return posInCycle + 1;
    } else {
        return 2 * loopLen - posInCycle;
    }
}

static int dirStride(int clockCount, int loopLen, int strideSize, uint32_t&) {
    return (((clockCount - 1) * strideSize) % loopLen) + 1;
}

static int dirOddEven(int clockCount, int loopLen, int, uint32_t&) {
    int pos = ((clockCount - 1) % loopLen) + 1;
    int numOdds = (loopLen + 1) / 2;
    if (pos <= numOdds) {
        return (pos - 1) * 2 + 1;
    } else {
        return (pos - numOdds) * 2;
    }
}

static int dirHopscotch(int clockCount, int loopLen, int, uint32_t&) {
    int pos = ((clockCount - 1) % (loopLen * 2)) + 1;
    int stepIndex = (pos + 1) / 2;
    if (pos % 2 == 1) {
        return ((stepIndex - 1) % loopLen) + 1;
    } else {
        int nextForward = (stepIndex % loopLen) + 1;
        return ((nextForward - 2 + loopLen) % loopLen) + 1;
    }
}

static int dirConverge(int clockCount, int loopLen, int, uint32_t&) {
    int pos = ((clockCount - 1) % loopLen) + 1;
    int pairIndex = (pos + 1) / 2;
    if (pos % 2 == 1) {
        return pairIndex;
    } else {
        return loopLen - pairIndex + 1;
    }
}

static int dirDiverge(int clockCount, int loopLen, int, uint32_t&) {
    int pos = ((clockCount - 1) % loopLen) + 1;
    int mid = (loopLen + 1) / 2;
    int pairIndex = (pos + 1) / 2;
    if (pos % 2 == 1) {
        return mid - pairIndex + 1;
    } else {
        return mid + pairIndex;
    }
}

// Note: DIR_BROWNIAN and DIR_SHUFFLE are handled specially in calculateTrackStep()
// They maintain state across calls, so they don't fit the stateless strategy pattern
static int dirBrownianPlaceholder(int clockCount, int loopLen, int, uint32_t&) {
    return ((clockCount - 1) % loopLen) + 1;
}

static int dirRandom(int, int loopLen, int, uint32_t& randState) {
    return randRange(randState, 1, loopLen);
}

static int dirShufflePlaceholder(int clockCount, int loopLen, int, uint32_t&) {
    return ((clockCount - 1) % loopLen) + 1;
}

// ============================================================================
// STRATEGY TABLE
// ============================================================================

static const DirectionStrategy directionStrategies[] = {
    dirForward,             // DIR_FORWARD = 0
    dirReverse,             // DIR_REVERSE = 1
    dirPendulum,            // DIR_PENDULUM = 2
    dirPingPong,            // DIR_PINGPONG = 3
    dirStride,              // DIR_STRIDE = 4
    dirOddEven,             // DIR_ODD_EVEN = 5
    dirHopscotch,           // DIR_HOPSCOTCH = 6
    dirConverge,            // DIR_CONVERGE = 7
    dirDiverge,             // DIR_DIVERGE = 8
    dirBrownianPlaceholder, // DIR_BROWNIAN = 9 (handled specially)
    dirRandom,              // DIR_RANDOM = 10
    dirShufflePlaceholder,  // DIR_SHUFFLE = 11 (handled specially)
};

static constexpr int NUM_DIRECTIONS = sizeof(directionStrategies) / sizeof(directionStrategies[0]);

static_assert(NUM_DIRECTIONS == 12, "Direction strategy table size mismatch - update table when adding directions");

// ============================================================================
// MAIN DIRECTION DISPATCH
// ============================================================================

int getStepForClock(int clockCount, int loopLen, int dir, int strideSize, uint32_t& randState) {
    if (loopLen == 1) return 1;
    if (clockCount < 1) return 0;

    if (dir < 0 || dir >= NUM_DIRECTIONS) {
        return dirForward(clockCount, loopLen, strideSize, randState);
    }

    return directionStrategies[dir](clockCount, loopLen, strideSize, randState);
}

// ============================================================================
// STATEFUL DIRECTION HELPERS
// ============================================================================

int updateBrownianStep(int currentPos, int loopLen, uint32_t& randState) {
    int delta = randRange(randState, BROWNIAN_DELTA_MIN, BROWNIAN_DELTA_MAX);
    if (delta == 0) delta = 1;  // Ensure we always move
    int newPos = currentPos + delta;
    return ((newPos - 1 + loopLen * 100) % loopLen) + 1;
}

void generateShuffleOrder(uint8_t* order, int loopLen, uint32_t& randState) {
    for (int i = 0; i < loopLen; i++) {
        order[i] = (uint8_t)(i + 1);
    }
    // Fisher-Yates shuffle
    for (int i = loopLen - 1; i >= 1; i--) {
        int j = randRange(randState, 0, i);
        uint8_t tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }
}

// ============================================================================
// WRAP DETECTION
// ============================================================================

static bool wrapForward(int prevPos, int currPos, int loopLen, int) {
    return currPos == 1 && prevPos == loopLen;
}

static bool wrapReverse(int prevPos, int currPos, int loopLen, int) {
    return currPos == loopLen && prevPos == 1;
}

static bool wrapPendulum(int prevPos, int currPos, int loopLen, int) {
    return (currPos == 1 && prevPos == 2) || (currPos == loopLen && prevPos == loopLen - 1);
}

static bool wrapPingPong(int, int, int loopLen, int clockCount) {
    int cycle = 2 * loopLen;
    int posInCycle = (clockCount - 1) % cycle;
    return posInCycle == 0;
}

static bool wrapStride(int, int currPos, int, int clockCount) {
    return clockCount > 1 && currPos == 1;
}

static bool wrapCyclic(int, int, int loopLen, int clockCount) {
    return clockCount > 1 && ((clockCount - 1) % loopLen) == 0;
}

static bool wrapHopscotch(int, int, int loopLen, int clockCount) {
    return clockCount > 1 && ((clockCount - 1) % (loopLen * 2)) == 0;
}

static const WrapDetector wrapDetectors[] = {
    wrapForward,    // DIR_FORWARD = 0
    wrapReverse,    // DIR_REVERSE = 1
    wrapPendulum,   // DIR_PENDULUM = 2
    wrapPingPong,   // DIR_PINGPONG = 3
    wrapStride,     // DIR_STRIDE = 4
    wrapCyclic,     // DIR_ODD_EVEN = 5
    wrapHopscotch,  // DIR_HOPSCOTCH = 6
    wrapCyclic,     // DIR_CONVERGE = 7
    wrapCyclic,     // DIR_DIVERGE = 8
    wrapCyclic,     // DIR_BROWNIAN = 9
    wrapCyclic,     // DIR_RANDOM = 10
    wrapCyclic,     // DIR_SHUFFLE = 11
};

bool detectWrap(int prevPos, int currPos, int loopLen, int dir, int clockCount) {
    if (prevPos < 1) return false;
    if (loopLen <= 1) return currPos == 1;

    if (dir < 0 || dir >= NUM_DIRECTIONS) {
        return wrapForward(prevPos, currPos, loopLen, clockCount);
    }

    return wrapDetectors[dir](prevPos, currPos, loopLen, clockCount);
}
