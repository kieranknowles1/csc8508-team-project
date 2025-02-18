# CSC8508 Team Project

- [CSC8508 Team Project](#csc8508-team-project)
  - [Building](#building)
  - [Debug Controls](#debug-controls)

## Building

To prepare for building on Windows, run
`git submodule update --init` then [`configure.bat`](./configure.bat).

This will install dependencies and run cmake.

When adding a new file to the project:
1. Add the file to the `set(SOURCES` of the relevant `CMakeLists.txt`.
2. Build `CMakePredefinedTargets/ZERO_CHECK`.
3. Reload the solution when prompted.

## Debug Controls

On PS5, `L1` must be held for debug inputs to register.

On PC/PS5:
- `F1`/`Dpad Right`: Reload world
- `F3`/`Dpad up`: Toggle Bullet debug draw
- `F4`/`Dpad right`: Toggle showing time usage
- `F`/`Dpad left`: Toggle free cam
