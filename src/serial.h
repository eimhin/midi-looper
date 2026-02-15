/*
 * MIDI Looper - Serialization
 * Save and load track data and state
 */

#pragma once

#include "types.h"
#include <distingnt/serialisation.h>

void serialiseData(MidiLooperAlgorithm* alg, _NT_jsonStream& stream);
bool deserialiseData(MidiLooperAlgorithm* alg, _NT_jsonParse& parse);
