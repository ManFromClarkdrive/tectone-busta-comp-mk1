# TECTONE BUSTER COMP MK1

An original JUCE VST3 compressor prototype with a hardware-inspired interface and an optical/tube-coloured compression direction.

## Important
This is **not** an exact CL 1B circuit emulation and does not contain Tube-Tech code, samples, or assets. It is an original DSP implementation intended as a starting point for tuning by ear and measurement. The current detector/curve is a simplified feed-forward design; it is not yet a measured model of the Legacy hardware.

## Build on Windows
1. Install Visual Studio 2022 with the **Desktop development with C++** workload.
2. Install CMake 3.22+ and Git.
3. Clone this repository.
4. Open a Developer PowerShell in the project folder and run:

```powershell
git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git
cmake -S . -B build -DJUCE_DIR="$PWD/JUCE" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target TectoneBusterCompMK1_VST3
```

The VST3 bundle will be in `build/TectoneBusterCompMK1_artefacts/Release/VST3/`.

## GitHub Actions
Push the project to a GitHub repository. The workflow in `.github/workflows/build-windows.yml` builds a Windows VST3 and uploads a downloadable artifact under Actions.

## Controls
Threshold, Ratio, Attack, Release, Makeup, Mix, Sidechain HPF, Warmth.

## DSP notes
The prototype uses a stereo RMS detector, sidechain high-pass filtering, soft-knee gain computer, attack/release envelope smoothing, wet/dry blend and gentle tanh-based colour. It requires listening tests, parameter calibration, oversampling/aliasing review, denormal/performance testing and comparison against licensed reference material before being considered release-ready.
