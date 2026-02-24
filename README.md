# MIDI Looper

A multi-track MIDI step recorder/sequencer plugin for the Expert Sleepers disting NT eurorack module. Records MIDI note input and plays it back with 12 playback directions, probability-based modifiers, trig conditions, and algorithmic sequence generation.

## Features

### Tracks

- 1-4 independently configurable tracks (set via specification, default 4)
- Up to 128 steps per track
- Up to 8 polyphonic note events per step
- Independent length, direction, clock division, channel, and modifiers per track
- Track 1 enabled by default, tracks 2-4 disabled

### CV Inputs

- **Run (In1)**: Gate input. Rising edge resets position and starts playback. Falling edge stops.
- **Clock (In2)**: Trigger input. Each rising edge advances the step position.

### Recording

- **Record**: Toggle recording on/off
- **Rec Track**: Select which track to record into (1-4)
- **Rec Mode**: Replace (clears track first), Overdub (adds to existing events), or Step (transport-independent step entry)
- **Division**: Global quantization grid for recording: 1 (off), 2, 4, 8, or 16
- **Rec Snap**: Quantization snap threshold (50-100%, default 75%). Controls how aggressively notes snap to the quantization grid.
- Records note on/off and velocity. Does not record pitch bend or CC.
- Tracks up to 128 simultaneous held notes during recording
- Held notes are finalized when recording stops

### Quantization

Applied during recording — events are snapped to the grid as they are recorded.

- **Division**: Global quantization grid: 1 (off), 2, 4, 8, or 16
- Finds the largest valid divisor that evenly divides the track length
- Note duration is snapped to the nearest grid point (minimum one grid unit)

### Scale Quantization

Applied at input before pass-through and recording. All incoming MIDI notes are quantized to the selected scale.

- **Scale Root**: C, C#, D, Eb, E, F, F#, G, Ab, A, Bb, B
- **Scale**: Off, Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian, Harmonic Minor, Melodic Minor, Major Pentatonic, Minor Pentatonic

### Playback Directions

Each track has an independent direction setting:

| Direction | Description                                        |
| --------- | -------------------------------------------------- |
| Forward   | Linear 1, 2, 3, ...                                |
| Reverse   | Backward linear                                    |
| Pendulum  | Bounces at endpoints without repeating them        |
| Ping-Pong | Bounces at endpoints, repeating them               |
| Stride    | Skips by a configurable stride size (2-16)         |
| Odd/Even  | Plays all odd-numbered steps, then all even        |
| Hopscotch | Alternating forward and backward single steps      |
| Converge  | Step pairs converge toward the center              |
| Diverge   | Step pairs diverge from the center                 |
| Brownian  | Random walk from current position (delta -2 to +2) |
| Random    | Fully random step selection (stateless)            |
| Shuffle   | Randomized order without repetition (Fisher-Yates) |

Brownian and Shuffle are stateful and reset on transport start.

### Per-Track Clock Division

Each track has an independent clock divider (1-16). A division of N means the track advances once every N incoming clock pulses, allowing polymetric patterns.

### Continuous Modifiers

Probability-based transformations applied in this fixed order to the step chosen by the direction mode:

1. **Stability** (0-100%): Probability to hold the current step instead of advancing
2. **Motion** (0-100%): Random jitter applied to step position
3. **Randomness** (0-100%): Probability to jump to a completely random step
4. **Pedal** (0-100%): Probability to return to the **Pedal Step** (1-128)

### Binary Modifiers

Deterministic filters applied after continuous modifiers:

- **No Repeat**: Skip if the resulting step is the same as the previous one

### Trig Conditions & Step Probability

Per-track controls for conditional step playback:

- **Step Prob** (0-100%): Global probability that any step plays
- **Step Cond**: Trig condition applied to all steps (Always, 1:2, 2:2, ..., 1:8-8:8, inverted variants, First, !First, Fill, !Fill, Fixed)
- **Cond Stp A/B**: Step-specific conditions — assign a different trig condition and probability to up to two individual steps
- **Fill**: Global toggle that activates Fill-conditioned steps

### Octave Jump

Per-track probabilistic pitch transposition with rhythmic bypass:

- **Oct Up** (0-4): Maximum upward octave shift
- **Oct Down** (0-4): Maximum downward octave shift
- **Oct Prob** (0-100%): Probability of applying an octave shift per note
- **Oct Bypass** (0-64): Number of notes to play unshifted before applying octave jumps
- **Bypass Offset** (-24 to +24): Semitone offset applied during the bypass period

### Sequence Generation

Algorithmic pattern generation and transformation, applied to the active recording track:

- **Generate**: Trigger generation
- **Gen Mode**:
  - **New**: Generate a fresh monophonic pattern from scratch
  - **Reorder**: Shuffle existing note positions (Fisher-Yates), preserving rhythm
  - **Re-pitch**: Replace note values with new random pitches, keeping rhythm and velocity
  - **Invert**: Reverse the step sequence in-place
- **Density** (1-100%): Probability of placing a note on each grid position
- **Bias** (MIDI note): Center pitch for generated notes
- **Range** (0-48 semitones): Pitch spread around bias
- **Note Rand** (0-100%): How much pitch varies within the range
- **Vel Var** (0-100%): Velocity variation around center (100)
- **Ties** (0-100%): Probability of extending a note's duration to reach the next note
- **Gate Rand** (0-100%): Random shortening of note durations

Generated notes respect the active scale quantization settings.

### Output

- **Channel**: Per-track MIDI output channel (1-16, defaults to track number + 1)
- **Velocity**: Offset applied to recorded velocity (-64 to +64)
- **Humanize**: Random delay per note (0-100ms)
- **MIDI Out Dest**: Breakout, SelectBus, USB, Internal, or All
- **Panic On Wrap**: Send all-notes-off when a track's loop wraps around
- Notes from the input channel are passed through to the output

### Track Management

- **Clear Track**: Clear all events on the active recording track
- **Clear All**: Clear all events on all tracks
- **MIDI In Ch**: Input channel filter (0 = omni, 1-16 for a specific channel; default 1)

### Display

- Play/stop and record indicators
- Quantization grid visualization (up to 16 divisions)
- Input velocity meter
- Per-track output velocity meters
- Track info boxes showing current step, loop length, and recording status

### State Persistence

Track data and playback state are saved/loaded with presets via JSON serialization.

## Prerequisites

### ARM Toolchain (for hardware builds)

**macOS:**

```bash
brew install --cask gcc-arm-embedded
```

> **Note:** Use the cask version (`--cask gcc-arm-embedded`), not the formula (`arm-none-eabi-gcc`). The cask includes the required newlib headers.

**Linux (Ubuntu/Debian):**

```bash
sudo apt install gcc-arm-none-eabi
```

**Windows:**
Download from [ARM Developer](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

## Build Commands

```bash
make hardware    # Build for disting NT hardware
make push        # Build and push to disting NT via MIDI
make clean       # Clean build artifacts
make help        # Show all options
```

## Deploying to Hardware

### Option 1: Direct MIDI Push

```bash
make push
```

Requires ntpush utility and MIDI connection to disting NT.

### Option 2: SD Card

1. `make hardware`
2. Copy the `.o` file from `plugins/` to the disting NT's SD card `plugins/` folder
3. Restart or rescan plugins on the disting NT

## Creating Releases

Push a version tag to trigger the GitHub Action:

```bash
git tag v1.0.0
git push origin v1.0.0
```

## API Reference

- [distingNT API Documentation](https://github.com/expertsleepersltd/distingNT_API)
- [disting NT User Manual](https://expert-sleepers.co.uk/distingNT.html)
