/*
 * MIDI Looper - Scale Quantization
 * White-key-to-scale mapping for musical scale quantization
 */

#pragma once

#include <cstdint>

// ============================================================================
// SCALE TYPE ENUM
// ============================================================================

enum ScaleType {
    SCALE_OFF = 0,
    SCALE_IONIAN,
    SCALE_DORIAN,
    SCALE_PHRYGIAN,
    SCALE_LYDIAN,
    SCALE_MIXOLYDIAN,
    SCALE_AEOLIAN,
    SCALE_LOCRIAN,
    SCALE_HARMONIC_MIN,
    SCALE_MELODIC_MIN,
    SCALE_MAJ_PENTATONIC,
    SCALE_MIN_PENTATONIC,

    SCALE_COUNT  // = 12
};

// ============================================================================
// SCALE INTERVAL TABLES (0-based semitone offsets from root)
// ============================================================================

static const int scaleIntervals[][7] = {
    { 0, 2, 4, 5, 7, 9, 11 },  // Ionian (Major)
    { 0, 2, 3, 5, 7, 9, 10 },  // Dorian
    { 0, 1, 3, 5, 7, 8, 10 },  // Phrygian
    { 0, 2, 4, 6, 7, 9, 11 },  // Lydian
    { 0, 2, 4, 5, 7, 9, 10 },  // Mixolydian
    { 0, 2, 3, 5, 7, 8, 10 },  // Aeolian (Natural Minor)
    { 0, 1, 3, 5, 6, 8, 10 },  // Locrian
    { 0, 2, 3, 5, 7, 8, 11 },  // Harmonic Minor
    { 0, 2, 3, 5, 7, 9, 11 },  // Melodic Minor
    { 0, 2, 4, 7, 9, 0, 0 },   // Major Pentatonic (5 notes, last 2 unused)
    { 0, 3, 5, 7, 10, 0, 0 },  // Minor Pentatonic (5 notes, last 2 unused)
};

// Number of notes per scale
static const int scaleSizes[] = {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 5, 5
};

// ============================================================================
// WHITE KEY LOOKUP TABLE
// ============================================================================

// Maps pitch class (0-11) to white key index (0-6)
// Black keys map down to the white key below them
// C=0, C#→0, D=1, D#→1, E=2, F=3, F#→3, G=4, G#→4, A=5, A#→5, B=6
static const int PC_TO_WHITE_KEY[12] = {
    0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6
};

// ============================================================================
// QUANTIZATION FUNCTION
// ============================================================================

// Quantize a MIDI note to a given root + scale combination.
// Maps white key positions to scale degrees, with octave wrapping for
// pentatonic scales (7 white keys > 5 scale degrees).
// Returns the note unchanged when scaleType == SCALE_OFF.
static inline uint8_t quantizeToScale(uint8_t note, int root, int scaleType) {
    if (scaleType == SCALE_OFF) return note;

    int scaleIdx = scaleType - 1;  // Interval table is 0-indexed (no OFF entry)
    int scaleSize = scaleSizes[scaleIdx];

    int pc = note % 12;
    int octave = note / 12;
    int whiteKeyIdx = PC_TO_WHITE_KEY[pc];

    int extraOctave = whiteKeyIdx / scaleSize;
    int scaleDegree = whiteKeyIdx % scaleSize;

    int outNote = (octave + extraOctave) * 12 + root + scaleIntervals[scaleIdx][scaleDegree];

    // Clamp to MIDI range
    if (outNote < 0) outNote = 0;
    if (outNote > 127) outNote = 127;

    return (uint8_t)outNote;
}
