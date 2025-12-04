# 🎛️ Mono/Stereo Ears - Audio Plugin

<div align="center">

![JUCE](https://img.shields.io/badge/Framework-JUCE-blue)
![macOS](https://img.shields.io/badge/Platform-macOS-lightgrey)
![Audio Plugin](https://img.shields.io/badge/Plugin-Mono/Stereo_Converter-4a90e2)
![License](https://img.shields.io/badge/License-MIT-green)

*A professional mono/stereo converter with pan control and intuitive switching interface*

</div>

## 🎚️ Features

- **Dual Mode Operation**: Instant switching between Mono and Stereo processing
- **Smart Controls**: Volume (always active) + Pan (stereo mode only)
- **Visual Feedback**: Color-coded toggle switch with clear mode indication
- **Professional Quality**: Sample-accurate audio processing with zero latency
- **Formats**: VST3, AU (Audio Unit)

## 📸 Screenshots

<div align="center">

### Mono Mode (Turquoise)
![Mono Mode](screenshots/Mono.png)

### Stereo Mode (Blue)
![Stereo Mode](screenshots/Stereo.png)

### Plugin in Action
![Plugin in DAW](screenshots/Monovector.png)

![Plugin in DAW](screenshots/Stereovector.png)
</div>

## 🎛️ Controls

| Parameter | Range | Description | Availability |
|-----------|-------|-------------|--------------|
| **Mode** | Mono / Stereo | Toggle switch for processing mode | Always active |
| **Volume** | 0.0 - 1.0 | Output level control | Both modes |
| **Pan** | -1.0 to +1.0 | Stereo panorama control | Stereo mode only |

## 🛠️ Development

### Technologies
- **JUCE Framework** 8.0+
- **C++17**
- **Xcode** (main IDE)
- **AudioProcessorValueTreeState** for parameter management

### Signal Processing
```cpp
// Mono Mode: (L + R) * 0.5 → Both Channels
// Stereo Mode: L * (1 - pan), R * (1 + pan) for pan range -1 to 1
