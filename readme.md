# CSC8508 Team Project

- [CSC8508 Team Project](#csc8508-team-project)
  - [Building](#building)
  - [PS5 Specific Workarounds](#ps5-specific-workarounds)
  - [Controls](#controls)

## Building

To prepare for building on Windows, run [`configure.bat`](./configure.bat).
This will install dependencies and run cmake.

When adding a new file to the project:
1. Add the file to the `set(SOURCES` of the relevant `CMakeLists.txt`.
2. Build `CMakePredefinedTargets/ZERO_CHECK`.
3. Reload the solution when prompted.

## PS5 Specific Workarounds
<!-- TODO: Automate these -->
- Copy `sce_module` (not included) and `Assets` to `build`
- Mirror push repo directory using workspace explorer

## Controls

- `F3`: Toggle Bullet debug draw
- `F4`: Toggle showing time usage
