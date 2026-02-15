/*
 * MIDI Looper - Utility Functions
 *
 * This header provides backward compatibility by including all utility modules.
 * For new code, prefer including specific headers directly:
 *   - midilooper/math.h       : clamp, safe index helpers
 *   - midilooper/random.h     : PRNG functions
 *   - midilooper/midi_utils.h : MIDI destination and channel helpers
 *   - midilooper/quantize.h   : Recording quantization functions
 */

#pragma once

#include "midilooper/math.h"
#include "midilooper/random.h"
#include "midilooper/midi_utils.h"
#include "midilooper/quantize.h"
