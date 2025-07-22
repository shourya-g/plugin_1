# Audio Pro Plugin

A versatile multi-effect audio plugin built with JUCE framework, featuring five high-quality audio effects with an intuitive drag-and-drop interface.

## Features

### Audio Effects
- **Phaser** - Classic swooshing modulation effect
- **Chorus** - Lush, dimensional sound that makes single sources sound like multiple
- **Overdrive** - Warm, analog-style saturation and distortion
- **Ladder Filter** - Vintage Moog-style filter with multiple modes (LPF, HPF, BPF in 12dB and 24dB variants)
- **General Filter** - Precise EQ-style filtering (Peak, Bandpass, Notch, Allpass)

### Interface Features
- **Drag & Drop Effect Ordering** - Reorder effects in real-time by dragging tabs
- **Real-time Audio Metering** - Monitor input and output levels with RMS meters
- **Spectrum Analyzer** - Visual frequency analysis of your audio
- **Individual Bypass Controls** - Toggle any effect on/off instantly
- **Smooth Parameter Changes** - All controls use parameter smoothing to prevent audio artifacts

### Effect Parameters

#### Phaser
- Rate: Modulation speed (0.01-2 Hz)
- Depth: Effect intensity (0-100%)
- Center Freq: Base frequency for modulation (20-20000 Hz)
- Feedback: Amount of feedback (-100% to +100%)
- Mix: Dry/wet balance (0-100%)

#### Chorus
- Rate: Modulation speed (0.01-100 Hz)
- Depth: Pitch variation amount (0-100%)
- Center Delay: Base delay time (0.01-100 ms)
- Feedback: Amount of feedback (-100% to +100%)
- Mix: Dry/wet balance (0-100%)

#### Overdrive
- Saturation: Amount of distortion (1-100%)

#### Ladder Filter
- Mode: Filter type (LPF12/24, HPF12/24, BPF12/24)
- Cutoff: Filter frequency (20-20000 Hz)
- Resonance: Filter resonance (0-100%)
- Drive: Filter drive amount (1-100x)

#### General Filter
- Mode: Filter type (Peak, Bandpass, Notch, Allpass)
- Frequency: Target frequency (20-20000 Hz)
- Quality: Filter Q factor (0.1-10)
- Gain: Boost/cut amount (-24dB to +24dB)

## Technical Specifications

- **Audio Processing**: 32-bit floating point
- **Sample Rates**: Supports all standard sample rates
- **Channels**: Stereo processing
- **Latency**: Near-zero latency processing
- **Plugin Formats**: VST3, AU, Standalone (depending on build configuration)



### Prerequisites
- JUCE Framework (version 6.0+)
- C++17 compatible compiler
- CMake (optional) or IDE with JUCE support



