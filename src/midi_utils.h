/*
 * MIDI Looper - MIDI Utilities
 * Helper functions for MIDI message handling
 */

#pragma once

#include <cstdint>
#include <distingnt/api.h>
#include "math.h"

// ============================================================================
// MIDI DESTINATION HELPERS
// ============================================================================

static inline uint32_t destToWhere(int dest) {
    switch (dest) {
        case 0: return kNT_destinationBreakout;
        case 1: return kNT_destinationSelectBus;
        case 2: return kNT_destinationUSB;
        case 3: return kNT_destinationInternal;
        default: return kNT_destinationBreakout | kNT_destinationSelectBus |
                        kNT_destinationUSB | kNT_destinationInternal;
    }
}

// ============================================================================
// MIDI CHANNEL HELPERS
// ============================================================================

static inline uint8_t withChannel(uint8_t status, int ch) {
    return (status & 0xF0) | (uint8_t)(clamp(ch, 1, 16) - 1);
}
