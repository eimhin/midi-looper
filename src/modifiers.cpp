#include "modifiers.h"
#include "random.h"

// ============================================================================
// CONTINUOUS MODIFIERS
// ============================================================================

int applyModifiers(MidiLooperAlgorithm* alg, int track, int baseStep, int loopLen) {
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    int step = baseStep;

    // Stability: chance to hold current step
    int stability = tp.stability();
    if (stability > 0 && randFloat(alg->randState) * 100.0f < (float)stability) {
        step = (alg->trackStates[track].lastStep > 0) ? alg->trackStates[track].lastStep : step;
    }

    // Motion: jitter step position
    int motion = tp.motion();
    if (motion > 0) {
        int maxJitter = (loopLen * motion) / 100;
        if (maxJitter < 1) maxJitter = 1;
        int jitter = randRange(alg->randState, -maxJitter, maxJitter);
        step = ((step - 1 + jitter + loopLen * 100) % loopLen) + 1;
    }

    // Randomness: chance to override with random step
    int randomness = tp.randomness();
    if (randomness > 0 && randFloat(alg->randState) * 100.0f < (float)randomness) {
        step = randRange(alg->randState, 1, loopLen);
    }

    // Gravity: bias toward anchor step
    int gravity = tp.gravity();
    if (gravity > 0 && randFloat(alg->randState) * 100.0f < (float)gravity) {
        int gravityAnchor = tp.gravityAnchor(loopLen);
        int diff = gravityAnchor - step;
        if (diff != 0) {
            step = step + (diff > 0 ? 1 : -1);
            step = ((step - 1 + loopLen) % loopLen) + 1;
        }
    }

    // Pedal: chance to return to pedal step
    int pedal = tp.pedal();
    if (pedal > 0 && randFloat(alg->randState) * 100.0f < (float)pedal) {
        step = tp.pedalStep(loopLen);
    }

    return step;
}

// ============================================================================
// BINARY MODIFIERS
// ============================================================================

static bool isStepAllowed(int step, int loopLen, int maskPattern, uint32_t& randState) {
    switch (maskPattern) {
        case MASK_ALL: return true;
        case MASK_ODDS: return (step % 2) == 1;
        case MASK_EVENS: return (step % 2) == 0;
        case MASK_FIRST_HALF: return step <= (loopLen + 1) / 2;
        case MASK_SECOND_HALF: return step > loopLen / 2;
        case MASK_SPARSE: return (step % MASK_SPARSE_DIVISOR) == 1;
        case MASK_DENSE: return (step % MASK_DENSE_DIVISOR) != 0;
        case MASK_RANDOM: return randFloat(randState) > MASK_RANDOM_THRESHOLD;
        default: return true;
    }
}

int applyBinaryModifiers(MidiLooperAlgorithm* alg, int track, int step, int prevStep, int loopLen) {
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    // No Repeat: skip if same as previous
    if (tp.noRepeat() == 1 && step == prevStep && loopLen > 1) {
        step = (step % loopLen) + 1;
    }

    // Step Mask: find next allowed step if current is masked
    int maskPattern = tp.stepMask();
    if (!isStepAllowed(step, loopLen, maskPattern, alg->randState)) {
        for (int offset = 1; offset < loopLen; offset++) {
            int testStep = ((step - 1 + offset) % loopLen) + 1;
            if (isStepAllowed(testStep, loopLen, maskPattern, alg->randState)) {
                step = testStep;
                break;
            }
        }
    }

    return step;
}

// ============================================================================
// READ MODES
// ============================================================================

int getStepsToEmit(MidiLooperAlgorithm* alg, int track, int selectedStep, int loopLen, int* outSteps) {
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    int mode = tp.readMode();
    int count = 0;

    switch (mode) {
        case 0:  // Single
            outSteps[count++] = selectedStep;
            break;

        case 1: {  // Arp N
            int window = tp.readWindow(loopLen);
            for (int i = 0; i < window && count < MAX_STEPS; i++) {
                int s = ((selectedStep - 1 + i) % loopLen) + 1;
                outSteps[count++] = s;
            }
            break;
        }

        case 2: {  // Growing
            int size = (alg->trackStates[track].readCyclePos % loopLen) + 1;
            for (int i = 0; i < size && count < MAX_STEPS; i++) {
                int s = ((selectedStep - 1 + i) % loopLen) + 1;
                outSteps[count++] = s;
            }
            alg->trackStates[track].readCyclePos++;
            break;
        }

        case 3: {  // Shrinking
            int size = loopLen - (alg->trackStates[track].readCyclePos % loopLen);
            for (int i = 0; i < size && count < MAX_STEPS; i++) {
                int s = ((selectedStep - 1 + i) % loopLen) + 1;
                outSteps[count++] = s;
            }
            alg->trackStates[track].readCyclePos++;
            break;
        }

        case 4: {  // Pedal Read
            int pedalStep = tp.pedalStep(loopLen);
            outSteps[count++] = selectedStep;
            if (pedalStep != selectedStep && count < MAX_STEPS) {
                outSteps[count++] = pedalStep;
            }
            break;
        }

        default:
            outSteps[count++] = selectedStep;
            break;
    }

    return count;
}
