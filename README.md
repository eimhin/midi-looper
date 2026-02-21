# MIDI Looper

A multi-track MIDI step recorder/sequencer plugin for the Expert Sleepers disting NT eurorack module. Records MIDI note input and plays it back with 12 playback directions and probability-based modifiers.

## Features

### Tracks

- 1-4 independently configurable tracks (set via specification)
- Up to 128 steps per track
- Up to 8 polyphonic note events per step
- Independent length, direction, channel, and modifiers per track
- Track 1 enabled by default, tracks 2-4 disabled

### CV Inputs

- **Run (In1)**: Gate input. Rising edge resets position and starts playback. Falling edge stops.
- **Clock (In2)**: Trigger input. Each rising edge advances the step position.

### Recording

- **Record**: Toggle recording on/off (only while transport is running)
- **Rec Track**: Select which track to record into (1-4)
- **Rec Mode**: Replace (clears track first) or Overdub (adds to existing events)
- **Rec Snap**: Quantization snap threshold (50-100%, default 75%). Controls how aggressively notes snap to the quantization grid.
- Records note on/off and velocity. Does not record pitch bend or CC.
- Tracks up to 128 simultaneous held notes during recording
- Held notes are finalized when recording stops

### Quantization

Applied during recording — events are snapped to the grid as they are recorded.

- **Division**: Per-track quantization grid: 1 (off), 2, 4, 8, or 16
- Finds the largest valid divisor that evenly divides the track length
- Note duration is snapped to the nearest grid point (minimum one grid unit)

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

### Continuous Modifiers

Probability-based transformations applied in this fixed order to the step chosen by the direction mode:

1. **Stability** (0-100%): Probability to hold the current step instead of advancing
2. **Motion** (0-100%): Random jitter applied to step position
3. **Randomness** (0-100%): Probability to jump to a completely random step
4. **Gravity** (0-100%): Bias toward the **Anchor** step (1-128)
5. **Pedal** (0-100%): Probability to return to the **Pedal Step** (1-128)

### Binary Modifiers

Deterministic filters applied after continuous modifiers:

- **No Repeat**: Skip if the resulting step is the same as the previous one

### Output

- **Channel**: Per-track MIDI output channel (1-16, defaults to track number)
- **Velocity**: Offset applied to recorded velocity (-64 to +64)
- **Humanize**: Random delay per note (0-100ms)
- **MIDI Out Dest**: Breakout, SelectBus, USB, Internal, or All
- **Panic On Wrap**: Send all-notes-off when a track's loop wraps around
- Notes from the input channel are passed through to the output

### Track Management

- **Clear Track**: Clear all events on the active recording track
- **Clear All**: Clear all events on all tracks
- **MIDI In Ch**: Input channel filter (0 = omni, 1-16 for a specific channel)

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

### nt_emu (for testing)

For testing without hardware, use the nt_emu VCV Rack module:
https://github.com/expertsleepersltd/nt_emu

## Build Commands

```bash
make hardware    # Build for disting NT hardware
make test        # Build for nt_emu testing
make both        # Build both targets
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
