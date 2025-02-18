mkdir build-ps5
pushd build-ps5
cmake.exe -DCMAKE_TOOLCHAIN_FILE="%SCE_ROOT_DIR%\Prospero\Tools\CMake\PS5.cmake" -DPS5_BUILD=ON -A Prospero %*  -DDEBUG_ASSET_ROOT:STRING=/app0/Assets/ ..
popd

@REM# FIXME: Manual steps required:
@REM Copy assets to output
@REM Copy sce_module to output
