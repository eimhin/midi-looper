# MIDI Looper

A multi-track MIDI looper plugin for the Expert Sleepers disting NT eurorack module. Supports live recording, step recording, and sequence generation.

## CV Inputs

- **Run (In1)**: Gate input. Rising edge resets position and starts playback. Falling edge stops.
- **Clock (In2)**: Trigger input. Each rising edge advances the step position.

## Recording

- **Record**: Toggle recording on/off
- **Rec Track**: Select which track to record into
- **Rec Mode**: Replace (clears track first), Overdub (adds to existing events), or Step (transport-independent step entry)
- **Rec Division**: Global quantization grid for recording: 1 (off), 2, 4, 8, or 16
- **Rec Snap**: Quantization snap threshold (50-100%, default 75%). Controls how aggressively notes snap to the quantization grid.
- Records note on/off and velocity. Does not record pitch bend or CC.

Events are snapped to the grid as they are recorded. The grid adapts to the track length, and note duration is snapped to the nearest grid point (minimum one grid unit).

### Scale Quantization

- **Scale Root**: C, C#, D, Eb, E, F, F#, G, Ab, A, Bb, B
- **Scale**: Off, Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, Locrian, Harmonic Minor, Melodic Minor, Major Pentatonic, Minor Pentatonic

## Tracks

- 1-8 independently configurable tracks (set via specification)
- Up to 128 steps per track
- Up to 8 polyphonic note events per step
- Independent length, direction, clock division, channel, and modifiers per track
- **Clear Track**: Clear all events on the active recording track
- **Clear All**: Clear all events on all tracks
- **MIDI In Ch**: Input channel filter (0 = omni, 1-16 for a specific channel; default 1)

### Playback Division

Each track has an independent clock divider (1-16). A division of N means the track advances once every N incoming clock pulses, allowing polymetric patterns.

### Playback Directions

Each track has an independent direction setting:

| Direction | Description                                        |
| --------- | -------------------------------------------------- |
| Forward   | Linear 1, 2, 3, ...                                |
| Reverse   | Backward linear                                    |
| Pendulum  | Bounces at endpoints without repeating them        |
| Ping-Pong | Bounces at endpoints, repeating them               |
| Odd/Even  | Plays all odd-numbered steps, then all even        |
| Hopscotch | Alternating forward and backward single steps      |
| Converge  | Step pairs converge toward the center              |
| Diverge   | Step pairs diverge from the center                 |
| Brownian  | Wanders randomly from current position             |
| Random    | Fully random step selection                        |
| Shuffle   | Plays every step once in random order              |
| Stride 2-5 | Skips by a fixed stride size (2, 3, 4, or 5)    |

### Modifiers

Applied in order to the current step:

1. **Stability** (0-100%): Probability to hold the current step instead of advancing
2. **Motion** (0-100%): Random jitter applied to step position
3. **Randomness** (0-100%): Probability to jump to a completely random step
4. **Pedal** (0-100%): Probability to return to the **Pedal Step** (1-128)
- **No Repeat**: Skip if the resulting step is the same as the previous one

### Trig Conditions & Step Probability

- **Step Prob** (0-100%): Global probability that any step plays
- **Step Cond**: Trig condition applied to all steps (Always, 1:2, 2:2, ..., 1:8-8:8, inverted variants, First, !First, Fill, !Fill, Fixed)
- **Cond Stp A/B**: Step-specific conditions — assign a different trig condition and probability to up to two individual steps
- **Fill**: Global toggle that activates Fill-conditioned steps

### Octave Jump

Random octave shifts per note:

- **Oct Min** (-3 to 3): Minimum octave shift
- **Oct Max** (-3 to 3): Maximum octave shift
- **Oct Prob** (0-100%): Probability of applying an octave shift per note
- **Oct Bypass** (0-64): Every Nth note is unshifted (0 = off)

### Output

- **Channel**: Per-track MIDI output channel (1-16, defaults to track number + 1)
- **Velocity**: Offset applied to recorded velocity (-64 to +64)
- **Humanize**: Random delay per note (0-100ms)
- **Destination**: Breakout, SelectBus, USB, Internal, or All
- **Panic On Wrap**: Send all-notes-off when a track's loop wraps around
> **Note:** MIDI input is passed through so you can play live alongside the sequencer, unless the input and output channels match.

## Sequence Generation

Generate or transform patterns on the active recording track:

- **Generate**: Trigger generation
- **Gen Mode**:
  - **New**: Generate a fresh monophonic pattern from scratch
  - **Reorder**: Shuffle existing note positions, preserving rhythm
  - **Re-pitch**: Replace note values with new random pitches, keeping rhythm and velocity
  - **Invert**: Reverse the step sequence
- **Density** (1-100%): Probability of placing a note on each grid position
- **Bias** (MIDI note): Center pitch for generated notes
- **Range** (0-48 semitones): Pitch spread around bias
- **Note Rand** (0-100%): How much pitch varies within the range
- **Vel Var** (0-100%): Velocity variation around center (100)
- **Ties** (0-100%): Probability of extending a note's duration to reach the next note
- **Gate Rand** (0-100%): Random shortening of note durations

Generated notes follow the active scale quantization.

## Display

- Play/stop and record indicators
- Step recording position number
- Recording division metronome indicator
- Input velocity meter
- Per-track output velocity meters
- Track info boxes with position indicator and recording track highlight

## Presets

Track data and playback state are saved/loaded with presets.

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
```

## API Reference

- [distingNT API Documentation](https://github.com/expertsleepersltd/distingNT_API)
- [disting NT User Manual](https://expert-sleepers.co.uk/distingNT.html)
