/*
 * MIDI Looper - Configuration
 * Tunable constants for customizing plugin behavior
 */

#pragma once

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

// Uncomment to enable debug logging (disable for release builds)
// #define MIDILOOPER_DEBUG

#ifdef MIDILOOPER_DEBUG
// Debug logging macros - requires NT_logFormat or similar platform function
#define DEBUG_LOG(fmt, ...) NT_logFormat(fmt, ##__VA_ARGS__)
#define DEBUG_POOL_OVERFLOW(pool_name) DEBUG_LOG("Pool overflow: %s", pool_name)
#define DEBUG_ASSERT(cond, msg)                                                                                        \
    do {                                                                                                               \
        if (!(cond)) DEBUG_LOG("Assert failed: %s", msg);                                                              \
    } while (0)
#else
// No-op in release builds - zero overhead
#define DEBUG_LOG(fmt, ...) ((void)0)
#define DEBUG_POOL_OVERFLOW(pool_name) ((void)0)
#define DEBUG_ASSERT(cond, msg) ((void)0)
#endif

// ============================================================================
// TRACK CONFIGURATION
// ============================================================================

static constexpr int MAX_TRACKS = 4; // Maximum number of tracks
static constexpr int MIN_TRACKS = 1; // Minimum number of tracks

// ============================================================================
// SEQUENCE CONFIGURATION
// ============================================================================

static constexpr int MAX_STEPS = 128;         // Maximum steps per track
static constexpr int MAX_EVENTS_PER_STEP = 8; // Maximum polyphony per step

// ============================================================================
// PERFORMANCE TUNING
// ============================================================================

static constexpr int MAX_DELAYED_NOTES = 64; // Humanization delay buffer size

// ============================================================================
// PARAMETER LAYOUT
// ============================================================================

static constexpr int PARAMS_PER_TRACK = 18; // Parameters per track
static constexpr int GLOBAL_PARAMS = 21;    // Global parameters (Run Input, Clock Input, Record, Generate, etc.)

// Derived constants (do not modify directly)
static constexpr int MAX_TOTAL_PARAMS = GLOBAL_PARAMS + (PARAMS_PER_TRACK * MAX_TRACKS);
static constexpr int MAX_PAGES = 4 + MAX_TRACKS; // Routing + Global + MIDI + Generate + track pages

// ============================================================================
// ALGORITHM TUNING
// ============================================================================

// Brownian motion parameters (step delta range)
static constexpr int BROWNIAN_DELTA_MIN = -2;
static constexpr int BROWNIAN_DELTA_MAX = 2;

// Step mask divisors
static constexpr int MASK_SPARSE_DIVISOR = 3; // Every 3rd step (step % 3 == 1)
static constexpr int MASK_DENSE_DIVISOR = 4;  // Skip every 4th step (step % 4 != 0)

// Random mask probability threshold (0.0 to 1.0)
static constexpr float MASK_RANDOM_THRESHOLD = 0.5f;

// ============================================================================
// COMPILE-TIME VALIDATION
// ============================================================================

// Ensure data types can hold configuration values
static_assert(MAX_STEPS <= 255, "MAX_STEPS must fit in uint8_t (TrackCache, shuffleOrder)");
static_assert(MAX_EVENTS_PER_STEP <= 255, "MAX_EVENTS_PER_STEP must fit in uint8_t");
static_assert(MAX_TRACKS <= 255, "MAX_TRACKS must fit in uint8_t");
static_assert(MAX_DELAYED_NOTES <= 65535, "MAX_DELAYED_NOTES must fit in uint16_t");

// Ensure Brownian delta range is sensible
static_assert(BROWNIAN_DELTA_MIN < BROWNIAN_DELTA_MAX, "Brownian delta range is invalid");
static_assert(BROWNIAN_DELTA_MIN >= -MAX_STEPS && BROWNIAN_DELTA_MAX <= MAX_STEPS,
              "Brownian delta range exceeds step bounds");

// Ensure mask divisors are valid (> 0 to avoid division by zero)
static_assert(MASK_SPARSE_DIVISOR > 0, "MASK_SPARSE_DIVISOR must be positive");
static_assert(MASK_DENSE_DIVISOR > 0, "MASK_DENSE_DIVISOR must be positive");
