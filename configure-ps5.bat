mkdir build
pushd build
cmake.exe -DCMAKE_TOOLCHAIN_FILE="%SCE_ROOT_DIR%\Prospero\Tools\CMake\PS5.cmake" -DPS5_BUILD=ON -A Prospero %*  -DDEBUG_ASSET_ROOT:STRING=/app0/Assets/ ..
popd
