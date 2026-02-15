/*
 * MIDI Looper v3 - distingNT Plugin
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
 * - Read modes (Single, Arp N, Growing, Shrinking, Pedal Read)
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
#include "midilooper/types.h"
#include "midilooper/params.h"
#include "midilooper/midi.h"
#include "midilooper/ui.h"
#include "midilooper/serial.h"
#include "midilooper/playback.h"

// ============================================================================
// SPECIFICATIONS
// ============================================================================

enum SpecIndex {
    SPEC_NUM_TRACKS = 0,
    NUM_SPECS
};

static const _NT_specification specifications[] = {
    { .name = "Tracks", .min = MIN_TRACKS, .max = MAX_TRACKS, .def = MAX_TRACKS, .type = kNT_typeGeneric }
};

// ============================================================================
// FACTORY FUNCTIONS
// ============================================================================

void calculateRequirements(_NT_algorithmRequirements& req, const int32_t* specs) {
    int numTracks = specs ? specs[SPEC_NUM_TRACKS] : MAX_TRACKS;
    req.numParameters = calcTotalParams(numTracks);
    req.sram = sizeof(MidiLooperAlgorithm);
    req.dram = sizeof(TrackState) * numTracks;  // Full per-track state
    req.dtc = sizeof(MidiLooper_DTC);
    req.itc = 0;
}

_NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs,
                         const _NT_algorithmRequirements& req,
                         const int32_t* specs) {
    MidiLooper_DTC* dtc = (MidiLooper_DTC*)ptrs.dtc;
    TrackState* trackStates = (TrackState*)ptrs.dram;
    int numTracks = specs ? specs[SPEC_NUM_TRACKS] : MAX_TRACKS;

    // Initialize DTC (global state only)
    memset(dtc, 0, sizeof(MidiLooper_DTC));
    dtc->transportState = TRANSPORT_STOPPED;
    dtc->prevGateHigh = false;
    dtc->prevClockHigh = false;
    dtc->stepTime = 0.0f;
    dtc->stepDuration = 0.1f;
    dtc->lastRecord = 0;
    dtc->lastTrack = 0;
    dtc->lastClearTrack = 0;
    dtc->lastClearAll = 0;

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
        ts->readCyclePos = 0;
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
    // Page 0: Global
    pThis->pageDefs[0] = { .name = "Global", .numParams = ARRAY_SIZE(pageGlobal), .group = 1, .unused = {0, 0}, .params = pageGlobal };
    // Page 1: MIDI
    pThis->pageDefs[1] = { .name = "MIDI", .numParams = ARRAY_SIZE(pageMidiConfig), .group = 2, .unused = {0, 0}, .params = pageMidiConfig };
    // Track pages (2 to 2+numTracks-1)
    for (int t = 0; t < numTracks; t++) {
        buildTrackPageIndices(pThis->pageTrackIndices[t], t);
        pThis->pageDefs[2 + t] = {
            .name = trackPageNames[t],
            .numParams = PARAMS_PER_TRACK,
            .group = 3,
            .unused = {0, 0},
            .params = pThis->pageTrackIndices[t]
        };
    }

    pThis->dynamicPages.numPages = 2 + numTracks;
    pThis->dynamicPages.pages = pThis->pageDefs;

    // Set up parameters and pages
    pThis->parameters = parameters;
    pThis->parameterPages = &pThis->dynamicPages;

    (void)req;  // Silence unused parameter warning

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

    // Read CV inputs
    float gateVal = busFrames[numFrames - 1];
    float clockVal = busFrames[numFrames + numFrames - 1];

    bool gateHigh = gateVal > GATE_THRESHOLD_HIGH;
    bool gateLow = gateVal < GATE_THRESHOLD_LOW;
    bool clockHigh = clockVal > GATE_THRESHOLD_HIGH;
    bool clockLow = clockVal < GATE_THRESHOLD_LOW;

    uint32_t where = destToWhere(v[kParamMidiOutDest]);

    // Gate edge detection (transport control)
    if (gateHigh && !dtc->prevGateHigh) {
        handleTransportStart(alg);
    } else if (gateLow && dtc->prevGateHigh) {
        handleTransportStop(alg, where);
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
                sendTrackNotesOff(alg, t, where, tp.channel());
                clearTrackEvents(&alg->trackStates[t].data);
            }
        }
        dtc->lastClearAll = clearAll;
    }

    // Timing and delayed notes
    dtc->stepTime += dt;
    processDelayedNotes(alg, dt);

    // Clock trigger processing
    if (clockRising && transportIsRunning(dtc->transportState)) {
        // Update step duration estimate
        if (dtc->stepTime > 0.001f) {
            dtc->stepDuration = dtc->stepTime;
        }
        dtc->stepTime = 0.0f;

        int recTrack = v[kParamRecTrack];
        bool panicOnWrap = (v[kParamPanicOnWrap] == 1);

        // Handle recording track change
        if (recTrack != dtc->lastTrack) {
            clearHeldNotes(alg);
            dtc->lastTrack = recTrack;
        }

        // Handle record state change
        int record = v[kParamRecord];
        if (record != dtc->lastRecord) {
            if (record == 1) {
                dtc->transportState = transportTransition_RecordBegin(dtc->transportState);
                if (v[kParamRecMode] == 0) {
                    clearTrackEvents(&alg->trackStates[recTrack].data);
                }
            } else {
                finalizeHeldNotes(alg);
                dtc->transportState = transportTransition_RecordEnd(dtc->transportState);
            }
            dtc->lastRecord = record;
        }

        // Process each track
        for (int t = 0; t < alg->numTracks; t++) {
            processTrack(alg, t, where, panicOnWrap);
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
    uint32_t where = destToWhere(v[kParamMidiOutDest]);

    bool isNoteOn = (status == kMidiNoteOn && byte2 > 0);
    bool isNoteOff = (status == kMidiNoteOff || (status == kMidiNoteOn && byte2 == 0));

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

    // Recording - only process if in recording state
    if (!transportIsRecording(dtc->transportState)) return;

    // Create recording context with current state (uses cached quantize)
    TrackState* ts = &alg->trackStates[track];
    RecordingContext ctx = createRecordingContext(
        v, track, ts->step, dtc->stepTime, dtc->stepDuration,
        &ts->cache
    );

    if (isNoteOn) {
        recordNoteOn(alg, ctx, byte1, byte2);
    }
    else if (isNoteOff) {
        recordNoteOff(alg, ctx, byte1);
    }
}

// ============================================================================
// UI AND SERIALIZATION WRAPPERS
// ============================================================================

bool draw(_NT_algorithm* self) {
    return drawUI((MidiLooperAlgorithm*)self);
}

void serialise(_NT_algorithm* self, _NT_jsonStream& stream) {
    serialiseData((MidiLooperAlgorithm*)self, stream);
}

bool deserialise(_NT_algorithm* self, _NT_jsonParse& parse) {
    return deserialiseData((MidiLooperAlgorithm*)self, parse);
}

// ============================================================================
// FACTORY DEFINITION
// ============================================================================

static const _NT_factory factory = {
    .guid = NT_MULTICHAR('M', 'i', 'L', '3'),  // MIDI Looper v3
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
    .serialise = serialise,
    .deserialise = deserialise,
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
            return kNT_apiVersion12;
        case kNT_selector_numFactories:
            return 1;
        case kNT_selector_factoryInfo:
            return (uintptr_t)((data == 0) ? &factory : NULL);
    }
    return 0;
}
