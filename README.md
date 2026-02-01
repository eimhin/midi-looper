# MIDI Looper

A MIDI looper plugin for the Expert Sleepers disting NT eurorack module.

## Features

- **Dual-target build system**: Build for hardware or nt_emu testing
- **GitHub Actions CI/CD**: Automatic builds and releases on version tags

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

## Getting Started

### Clone with Submodules

```bash
git clone --recursive https://github.com/yourusername/your-plugin.git
cd your-plugin
```

Or if already cloned:
```bash
git submodule update --init --recursive
```

### Build Commands

```bash
# Build for distingNT hardware
make hardware

# Build for nt_emu testing
make test

# Build both
make both

# Build and push to distingNT via MIDI
make push

# Clean build artifacts
make clean

# Show all options
make help
```

## Testing with nt_emu

1. Build the test target:
   ```bash
   make test
   ```

2. Copy the plugin to nt_emu's plugin directory:
   ```bash
   cp plugins/myplugin.dylib ~/Documents/nt_emu/plugins/
   ```

3. Load VCV Rack and add the nt_emu module

4. Your plugin should appear in the algorithm list

## Deploying to Hardware

### Option 1: Direct MIDI Push
```bash
make push
```
Requires ntpush utility and MIDI connection to disting NT.

### Option 2: SD Card
1. Build the hardware target:
   ```bash
   make hardware
   ```

2. Copy `plugins/myplugin.o` to the disting NT's SD card `plugins/` folder

3. Restart or rescan plugins on the disting NT

## Project Structure

```
.
├── .github/workflows/
│   └── release.yaml      # CI/CD: build on tag, create release
├── distingNT_API/        # API submodule (git submodule)
├── .gitmodules           # Submodule configuration
├── .gitignore            # Build artifacts exclusions
├── Makefile              # Dual-target build system
├── myplugin.cpp          # Your plugin source
└── README.md             # This file
```

## Customizing the Template

1. **Rename your plugin**: Update `PLUGIN_NAME` in `Makefile`

2. **Change the GUID**: In your `.cpp` file, change the `NT_MULTICHAR()` to a unique 4-character identifier

3. **Update CI/CD**: In `.github/workflows/release.yaml`, update the output filename

4. **Add more source files**: Update the `SOURCES` variable in `Makefile`

## Creating Releases

Push a version tag to trigger the GitHub Action:
```bash
git tag v1.0.0
git push origin v1.0.0
```

This will:
1. Build the hardware plugin
2. Create a GitHub Release
3. Attach the `.o` file to the release

## API Reference

- [distingNT API Documentation](https://github.com/expertsleepersltd/distingNT_API)
- [disting NT User Manual](https://expert-sleepers.co.uk/distingNT.html)

## Example Plugin Overview

The included `myplugin.cpp` demonstrates:

### MIDI Input
```cpp
void midiMessage(_NT_algorithm* self, uint8_t byte0, uint8_t byte1, uint8_t byte2) {
    uint8_t status = byte0 & 0xF0;  // 0x90 = note on, 0x80 = note off
    uint8_t channel = byte0 & 0x0F;
    uint8_t note = byte1;
    uint8_t velocity = byte2;
    // Process MIDI data...
}
```

### Parameter Pages with Groups
```cpp
static const _NT_parameterPage pages[] = {
    { .name = "Main", .numParams = ..., .group = 1, .params = pageMain },
    { .name = "Settings", .numParams = ..., .group = 2, .params = pageSettings },
};
```

### Custom UI
```cpp
bool draw(_NT_algorithm* self) {
    NT_drawText(10, 10, "Title", 15, kNT_textLeft, kNT_textNormal);
    NT_drawShapeI(kNT_rectangle, x1, y1, x2, y2, brightness);
    return true;  // Hide standard parameter display
}
```

## License

This template is provided as-is for educational purposes. See the distingNT API repository for API licensing terms.
