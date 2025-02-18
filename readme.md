# CSC8508 Team Project

- [CSC8508 Team Project](#csc8508-team-project)
  - [FMod SDK](#FMod)
  - [Building](#building)
  - [Controls](#controls)

## FMod

Create an account on fmod.com and download FMod Engine files (Version 2.02.26).

Once downloaded, Create a folder `2.02.26` inside `this-repo/fmod-cmake/fmod/` directory.

Make two folders inside `this-repo/fmod-cmake/fmod/2.02.26/` named `include` and `lib` respectively.

Run the FMod installer and download the files to a directory of your choosing.

Once installed, copy all `.h` and `.hpp` files from `Your FMod install dir/core/inc` to `this-repo/fmod-cmake/fmod/2.02.26/include`.

Also copy all `.h` and `.hpp` files from `Your FMod install dir/studio/inc` to `this-repo/fmod-cmake/fmod/2.02.26/include`.

Once that's done, create a folder `windows-amd64` inside `this-repo/fmod-cmake/fmod/2.02.26/lib/`.

Copy all files from `Your FMod install dir/core/lib/x64` and all files from `Your FMod install dir/studio/lib/x64` and paste them in `this-repo/fmod-cmake/fmod/2.02.26/lib/windows-amd64/`

`Fmod-cmake` is an interface provided by github user `tadashibashi` at `https://github.com/tadashibashi/fmod-cmake`

I recommend copying the fmod-cmake folder you have just made in this repository and pasting it to another directory on your PC, just so you don't have to repeat these steps everytime. Do not upload the fmod sdk files to any public repositories.

*Note:* When trying to run the project the first time, the program will crash with an error. You need to copy all `.dll` files from `this-repo/fmod-cmake/fmod/2.02.26/lib/windows-amd64/` to `this-repo/build/TeamProject/Debug/` (Or wherever your CSC8508.exe is located). This error should be fixable by modifying CMakeLists.txt, just not sure how.

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
