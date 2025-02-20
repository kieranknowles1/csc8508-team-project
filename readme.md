# CSC8508 Team Project

- [CSC8508 Team Project](#csc8508-team-project)
  - [FMod](#fmod)
  - [Building](#building)
  - [Controls](#controls)

## FMod

Create an account on fmod.com and download FMod Engine files (Version 2.02.26).
Assuming you install to the default directory, headers, libraries, and DLLs will
be found automatically. Otherwise set `FMOD_SDK_PATH` to your FMod install directory.

`Fmod-cmake` is an interface provided by github user `tadashibashi` at
`https://github.com/tadashibashi/fmod-cmake`

## Building

To prepare for building on Windows, run
`git submodule update --init` then [`configure.bat`](./configure.bat).

This will install dependencies and run cmake.

When adding a new file to the project:
1. Add the file to the `set(SOURCES` of the relevant `CMakeLists.txt`.
2. Build `CMakePredefinedTargets/ZERO_CHECK`.
3. Reload the solution when prompted.

## Controls

- `F3`: Toggle Bullet debug draw
- `F4`: Toggle showing time usage
