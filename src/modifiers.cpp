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

int applyBinaryModifiers(MidiLooperAlgorithm* alg, int track, int step, int prevStep, int loopLen) {
    TrackParams tp = TrackParams::fromAlgorithm(alg->v, track);

    // No Repeat: skip if same as previous
    if (tp.noRepeat() == 1 && step == prevStep && loopLen > 1) {
        step = (step % loopLen) + 1;
    }

    return step;
}
