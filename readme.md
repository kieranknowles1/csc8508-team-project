# CSC8508 Team Project

- [CSC8508 Team Project](#csc8508-team-project)
  - [FMod](#fmod)
  - [Building](#building)
    - [Windows](#windows)
    - [Linux](#linux)
    - [PS5](#ps5)
  - [Debug Controls](#debug-controls)
  - [Unity Importer](#Unity-Exporter)

For further information, see [docs/readme.md](docs/readme.md)

## FMod

Create an account on fmod.com and download FMod Engine files (Version 2.02.26).
Assuming you install to the default directory, headers, libraries, and DLLs will
be found automatically. Otherwise set `FMOD_SDK_PATH` to your FMod install directory.

`Fmod-cmake` is an interface provided by github user `tadashibashi` at
`https://github.com/tadashibashi/fmod-cmake`

On Linux, the `FMOD_SDK_PATH` must be manually set to wherever you've
extracted FMOD to. NOTE: currently fmod loads but can't find an audio
server. This may be an issue with just my system, but is untested.

## Building

This project uses submodules for dependencies. These are not cloned by default
and must be manually added using `git submodule update --init`

### Windows

Run `configure.bat`

### Linux

Run `cmake` in a build directory. The following declarations must be added
manually through the CLI:
- `USE_SDL2=YES`
- `FMOD_SDK_PATH=/path/to/fmod/install/api`
- `CMAKE_C_COMPILER=clang` - only Clang is supported, GCC is known not to work
- `CMAKE_CXX_COMPILER=clang++`

### PS5

Run `configure-ps5.bat`. This requires the PS5 SDK to be installed which cannot
be included for licensing reasons.

## Debug Controls

On controllers, **R3** must be held for debug inputs to register.

On Keyboard/Controller:
- **F1/Dpad Right**: Reload world
- **F3/Dpad up**: Toggle Bullet debug draw
- **F4/Dpad right**: Toggle showing time usage
- **H/Dpad left**: Toggle free cam

  ## Unity Exporter
  [Repository for the Unity expoerter and map](https://github.com/A-Barnett/Unity-Exporter)
 
