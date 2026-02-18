/*
 * MIDI Looper - distingNT Plugin
 *
 * 4-track MIDI step recorder/sequencer with quantized recording and independent
 * track lengths, directions, and output channels.
 *
 * FEATURES:
 * - 4 independent MIDI tracks with separate lengths (1-128 steps), divisions, and output channels
 * - Quantized step recording with configurable snap threshold
 * - Replace or Overdub recording modes
 * - MIDI pass-through from input to active track's output channel
 * - Up to 8 polyphonic note events per step with duration tracking
 * - State persistence (track data survives preset save/load)
 * - 12 playback directions per track
 * - Continuous modifiers (Stability, Motion, Randomness, Gravity, Pedal)
 * - Binary modifiers (No Repeat, Step Mask)
 *
 * Inputs:
 * - In1: Run gate (rising edge resets and starts; falling edge stops)
 * - In2: Clock trigger (advances step position)
 *
 * Copyright (c) 2025 Eimhin Rooney
 */

#include <cstring>
#include <new>

// Module headers
#include "midi.h"
#include "midi_utils.h"
#include "params.h"
#include "playback.h"
#include "recording.h"
#include "scales.h"
#include "serial.h"
#include "types.h"
#include "ui.h"

// ============================================================================
// SPECIFICATIONS
// ============================================================================

enum SpecIndex { SPEC_NUM_TRACKS = 0, NUM_SPECS };

static const _NT_specification specifications[] = {
    {.name = "Tracks", .min = MIN_TRACKS, .max = MAX_TRACKS, .def = MAX_TRACKS, .type = kNT_typeGeneric}};

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

void calculateRequirements(_NT_algorithmRequirements& req, const int32_t* specs) {
    int numTracks = specs ? specs[SPEC_NUM_TRACKS] : MAX_TRACKS;
    req.numParameters = calcTotalParams(numTracks);
    req.sram = sizeof(MidiLooperAlgorithm);
    req.dram = sizeof(TrackState) * numTracks; // Full per-track state
    req.dtc = sizeof(MidiLooper_DTC);
    req.itc = 0;
}

_NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req,
                         const int32_t* specs) {
    MidiLooper_DTC* dtc = (MidiLooper_DTC*)ptrs.dtc;
    TrackState* trackStates = (TrackState*)ptrs.dram;
    int numTracks = specs ? specs[SPEC_NUM_TRACKS] : MAX_TRACKS;

    // Initialize DTC (global state only)
    memset(dtc, 0, sizeof(MidiLooper_DTC));
    for (int i = 0; i < 128; i++) {
        dtc->noteMap[i] = (uint8_t)i;
    }
    dtc->transportState = TRANSPORT_STOPPED;
    dtc->recordState = REC_IDLE;
    dtc->prevGateHigh = false;
    dtc->prevClockHigh = false;
    dtc->stepTime = 0.0f;
    dtc->stepDuration = 0.1f;
    dtc->lastRecord = 0;
    dtc->lastTrack = 0;
    dtc->lastClearTrack = 0;
    dtc->lastClearAll = 0;
    dtc->stepRecPos = 0;

    // Initialize per-track state in DRAM
    for (int t = 0; t < numTracks; t++) {
        TrackState* ts = &trackStates[t];

        // Clear all track data
        for (int s = 0; s < MAX_STEPS; s++) {
            ts->data.steps[s].count = 0;
            ts->shuffleOrder[s] = (uint8_t)(s + 1);
        }

        // Clear playing notes
        for (int n = 0; n < 128; n++) {
            ts->playing[n].active = false;
            ts->activeNotes[n] = 0;
        }

        // Initialize playback state
        ts->clockCount = 0;
        ts->step = 0;
        ts->lastStep = 1;
        ts->brownianPos = 1;
        ts->shufflePos = 1;
        ts->activeVel = 0;
        ts->lastEnabled = (t == 0) ? 1 : 0;

        // Initialize cache as dirty
        ts->cache.invalidate();
    }

    // Construct algorithm in SRAM
    MidiLooperAlgorithm* pThis = new (ptrs.sram) MidiLooperAlgorithm(dtc, trackStates, numTracks);

    // Seed PRNG with hardware entropy for unique randomness per instance
    pThis->randState = NT_getCpuCycleCount();

    // Initialize held notes
    for (int i = 0; i < 128; i++) {
        pThis->heldNotes[i].active = false;
    }

    // Initialize delayed notes
    for (int i = 0; i < MAX_DELAYED_NOTES; i++) {
        pThis->delayedNotes[i].active = false;
    }

    // Build dynamic parameter pages based on track count
    // Page 0: Routing
    pThis->pageDefs[0] = {
        .name = "Routing", .numParams = ARRAY_SIZE(pageRouting), .group = 0, .unused = {0, 0}, .params = pageRouting};
    // Page 1: Global
    pThis->pageDefs[1] = {
        .name = "Global", .numParams = ARRAY_SIZE(pageGlobal), .group = 1, .unused = {0, 0}, .params = pageGlobal};
    // Page 2: MIDI
    pThis->pageDefs[2] = {.name = "MIDI",
                          .numParams = ARRAY_SIZE(pageMidiConfig),
                          .group = 2,
                          .unused = {0, 0},
                          .params = pageMidiConfig};
    // Track pages (3 to 3+numTracks-1)
    for (int t = 0; t < numTracks; t++) {
        buildTrackPageIndices(pThis->pageTrackIndices[t], t);
        pThis->pageDefs[3 + t] = {.name = trackPageNames[t],
                                  .numParams = PARAMS_PER_TRACK,
                                  .group = 3,
                                  .unused = {0, 0},
                                  .params = pThis->pageTrackIndices[t]};
    }

    pThis->dynamicPages.numPages = 3 + numTracks;
    pThis->dynamicPages.pages = pThis->pageDefs;

    // Set up parameters and pages
    pThis->parameters = parameters;
    pThis->parameterPages = &pThis->dynamicPages;

    (void)req; // Silence unused parameter warning

    return pThis;
}

void parameterChanged(_NT_algorithm* self, int p) {
    MidiLooperAlgorithm* alg = (MidiLooperAlgorithm*)self;

    // Check if this is a track parameter that affects cached values
    if (p >= kGlobalParamCount) {
        int track = (p - kGlobalParamCount) / PARAMS_PER_TRACK;
        int trackParam = (p - kGlobalParamCount) % PARAMS_PER_TRACK;

        // Invalidate cache when length or division changes
        // These are the only parameters that affect effectiveQuantize
        if (trackParam == kTrackLength || trackParam == kTrackDivision) {
            if (track >= 0 && track < alg->numTracks) {
                alg->trackStates[track].cache.invalidate();

                // Reset step record cursor if the active recording track's grid changed
                if (track == alg->v[kParamRecTrack] && alg->dtc->recordState == REC_STEP) {
                    alg->dtc->stepRecPos = 1;
                }
            }
        }
    }
}

// ============================================================================
// STEP FUNCTION (Audio rate processing)
// ============================================================================

void step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    MidiLooperAlgorithm* alg = (MidiLooperAlgorithm*)self;
    MidiLooper_DTC* dtc = alg->dtc;
    const int16_t* v = alg->v;

    int numFrames = numFramesBy4 * 4;
    float dt = (float)numFrames / (float)NT_globals.sampleRate;

    // Read CV inputs from user-selected buses
    int runBus = v[kParamRunInput];
    int clkBus = v[kParamClockInput];
    float gateVal = (runBus > 0) ? busFrames[(runBus - 1) * numFrames + numFrames - 1] : 0.0f;
    float clockVal = (clkBus > 0) ? busFrames[(clkBus - 1) * numFrames + numFrames - 1] : 0.0f;

    bool gateHigh = gateVal > GATE_THRESHOLD_HIGH;
    bool gateLow = gateVal < GATE_THRESHOLD_LOW;
    bool clockHigh = clockVal > GATE_THRESHOLD_HIGH;
    bool clockLow = clockVal < GATE_THRESHOLD_LOW;

    // Gate edge detection (transport control)
    if (gateHigh && !dtc->prevGateHigh) {
        handleTransportStart(alg);
    } else if (gateLow && dtc->prevGateHigh) {
        handleTransportStop(alg);
    }
    dtc->prevGateHigh = gateHigh && !gateLow;

    // Clock edge detection
    bool clockRising = clockHigh && !dtc->prevClockHigh;
    dtc->prevClockHigh = clockHigh && !clockLow;

    // Parameter change detection: Clear Track
    int clearTrack = v[kParamClearTrack];
    if (clearTrack != dtc->lastClearTrack) {
        if (clearTrack == 1) {
            int track = v[kParamRecTrack];
            TrackParams tp = TrackParams::fromAlgorithm(v, track);
            uint32_t where = destToWhere(tp.destination());
            sendTrackNotesOff(alg, track, where, tp.channel());
            clearTrackEvents(&alg->trackStates[track].data);
        }
        dtc->lastClearTrack = clearTrack;
    }

    // Parameter change detection: Clear All
    int clearAll = v[kParamClearAll];
    if (clearAll != dtc->lastClearAll) {
        if (clearAll == 1) {
            for (int t = 0; t < alg->numTracks; t++) {
                TrackParams tp = TrackParams::fromAlgorithm(v, t);
                uint32_t where = destToWhere(tp.destination());
                sendTrackNotesOff(alg, t, where, tp.channel());
                clearTrackEvents(&alg->trackStates[t].data);
            }
        }
        dtc->lastClearAll = clearAll;
    }

    // Timing and delayed notes
    dtc->stepTime += dt;
    processDelayedNotes(alg, dt);

    // Recording state machine evaluation
    {
        int record = v[kParamRecord];
        int recMode = v[kParamRecMode];
        int recTrack = v[kParamRecTrack];
        bool isStepMode = (recMode == REC_MODE_STEP);

        // Handle recording track change
        if (recTrack != dtc->lastTrack) {
            clearHeldNotes(alg);
            if (dtc->recordState == REC_STEP) {
                dtc->stepRecPos = 1;
            }
            dtc->lastTrack = recTrack;
        }

        bool recordChanged = (record != dtc->lastRecord);

        switch (dtc->recordState) {
        case REC_IDLE:
            if (recordChanged && record == 1) {
                if (isStepMode) {
                    dtc->stepRecPos = 1;
                    dtc->recordState = REC_STEP;
                } else if (transportIsRunning(dtc->transportState)) {
                    if (recMode == REC_MODE_REPLACE) {
                        clearTrackEvents(&alg->trackStates[recTrack].data);
                    }
                    dtc->recordState = REC_LIVE;
                } else {
                    dtc->recordState = REC_LIVE_PENDING;
                }
            }
            break;

        case REC_LIVE:
            if (recordChanged && record == 0) {
                finalizeHeldNotes(alg);
                dtc->recordState = REC_IDLE;
            } else if (isStepMode) {
                // Mode changed to Step while live recording
                finalizeHeldNotes(alg);
                dtc->stepRecPos = 1;
                dtc->recordState = REC_STEP;
            }
            break;

        case REC_STEP:
            if (recordChanged && record == 0) {
                dtc->stepRecPos = 0;
                dtc->recordState = REC_IDLE;
            } else if (!isStepMode) {
                // Mode changed to Live while step recording
                dtc->stepRecPos = 0;
                if (transportIsRunning(dtc->transportState)) {
                    if (recMode == REC_MODE_REPLACE) {
                        clearTrackEvents(&alg->trackStates[recTrack].data);
                    }
                    dtc->recordState = REC_LIVE;
                } else {
                    dtc->recordState = REC_LIVE_PENDING;
                }
            }
            break;

        case REC_LIVE_PENDING:
            if (recordChanged && record == 0) {
                dtc->recordState = REC_IDLE;
            } else if (isStepMode) {
                // Mode changed to Step while pending
                dtc->stepRecPos = 1;
                dtc->recordState = REC_STEP;
            } else if (transportIsRunning(dtc->transportState)) {
                // Normally handled by handleTransportStart(), but kept as
                // a safety net in case execution order within step() changes.
                if (recMode == REC_MODE_REPLACE) {
                    clearTrackEvents(&alg->trackStates[recTrack].data);
                }
                dtc->recordState = REC_LIVE;
            }
            break;
        }

        dtc->lastRecord = record;
    }

    // Clock trigger processing
    if (clockRising && transportIsRunning(dtc->transportState)) {
        // Update step duration estimate
        if (dtc->stepTime > 0.001f) {
            dtc->stepDuration = dtc->stepTime;
        }
        dtc->stepTime = 0.0f;

        bool panicOnWrap = (v[kParamPanicOnWrap] == 1);

        // Process each track
        for (int t = 0; t < alg->numTracks; t++) {
            processTrack(alg, t, panicOnWrap);
        }
    }
}

// ============================================================================
// MIDI HANDLING
// ============================================================================

void midiMessage(_NT_algorithm* self, uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    MidiLooperAlgorithm* alg = (MidiLooperAlgorithm*)self;
    MidiLooper_DTC* dtc = alg->dtc;
    const int16_t* v = alg->v;

    uint8_t status = byte0 & 0xF0;
    uint8_t channel = byte0 & 0x0F;

    // Channel filter
    int channelFilter = v[kParamMidiInCh];
    if (channelFilter > 0 && channel != (channelFilter - 1)) {
        return;
    }

    int track = v[kParamRecTrack];
    TrackParams tp = TrackParams::fromAlgorithm(v, track);
    int outCh = tp.channel();
    uint32_t where = destToWhere(tp.destination());

    bool isNoteOn = (status == kMidiNoteOn && byte2 > 0);
    bool isNoteOff = (status == kMidiNoteOff || (status == kMidiNoteOn && byte2 == 0));

    // Scale quantization (applied at input, before pass-through and recording)
    if (isNoteOn) {
        uint8_t quantized = quantizeToScale(byte1, v[kParamScaleRoot], v[kParamScaleType]);
        dtc->noteMap[byte1] = quantized;
        byte1 = quantized;
    } else if (isNoteOff) {
        byte1 = dtc->noteMap[byte1];
    }

    // Pass-through (if input channel differs from output)
    if (isNoteOn || isNoteOff) {
        int inCh = channel + 1;
        if (inCh != outCh) {
            NT_sendMidi3ByteMessage(where, withChannel(status, outCh), byte1, byte2);
        }
    }

    // Update input display state
    if (isNoteOn) {
        dtc->inputNotes[byte1] = 1;
        dtc->inputVel = byte2;
    } else if (isNoteOff) {
        dtc->inputNotes[byte1] = 0;
        // Check if any notes still held
        bool anyHeld = false;
        for (int n = 0; n < 128; n++) {
            if (dtc->inputNotes[n]) {
                anyHeld = true;
                break;
            }
        }
        if (!anyHeld) {
            dtc->inputVel = 0;
        }
    }

    // Step recording - process independently of transport
    if (dtc->recordState == REC_STEP) {
        if (isNoteOn) {
            stepRecordNoteOn(alg, track, byte1, byte2);
        } else if (isNoteOff) {
            stepRecordNoteOff(alg, track, byte1);
        }
        return;
    }

    // Live recording - only process if in recording state
    if (dtc->recordState != REC_LIVE) return;

    // Create recording context with current state (uses cached quantize)
    TrackState* ts = &alg->trackStates[track];
    RecordingContext ctx = createRecordingContext(v, track, ts->step, dtc->stepTime, dtc->stepDuration, &ts->cache);

    if (isNoteOn) {
        recordNoteOn(alg, ctx, byte1, byte2);
    } else if (isNoteOff) {
        recordNoteOff(alg, ctx, byte1);
    }
}

// ============================================================================
// UI AND SERIALIZATION WRAPPERS
// ============================================================================

bool draw(_NT_algorithm* self) { return drawUI((MidiLooperAlgorithm*)self); }

#ifdef DISTING_HARDWARE
void serialise(_NT_algorithm* self, _NT_jsonStream& stream) { serialiseData((MidiLooperAlgorithm*)self, stream); }

bool deserialise(_NT_algorithm* self, _NT_jsonParse& parse) {
    return deserialiseData((MidiLooperAlgorithm*)self, parse);
}
#endif

// ============================================================================
// FACTORY DEFINITION
// ============================================================================

static const _NT_factory factory = {
    .guid = NT_MULTICHAR('M', 'i', 'L', '3'), // MIDI Looper v3
    .name = "MIDI Looper",
    .description = "1-4 track MIDI step recorder/sequencer",
    .numSpecifications = NUM_SPECS,
    .specifications = specifications,
    .calculateStaticRequirements = NULL,
    .initialise = NULL,
    .calculateRequirements = calculateRequirements,
    .construct = construct,
    .parameterChanged = parameterChanged,
    .step = step,
    .draw = draw,
    .midiRealtime = NULL,
    .midiMessage = midiMessage,
    .tags = kNT_tagUtility,
    .hasCustomUi = NULL,
    .customUi = NULL,
    .setupUi = NULL,
#ifdef DISTING_HARDWARE
    .serialise = serialise,
    .deserialise = deserialise,
#else
    .serialise = NULL,
    .deserialise = NULL,
#endif
    .midiSysEx = NULL,
    .parameterUiPrefix = NULL,
    .parameterString = NULL,
};

// ============================================================================
// PLUGIN ENTRY POINT
// ============================================================================

uintptr_t pluginEntry(_NT_selector selector, uint32_t data) {
    switch (selector) {
    case kNT_selector_version:
        return kNT_apiVersion13;
    case kNT_selector_numFactories:
        return 1;
    case kNT_selector_factoryInfo:
        return (uintptr_t)((data == 0) ? &factory : NULL);
    }
    return 0;
}
