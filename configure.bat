ECHO OFF

IF NOT EXIST "../vcpkg" (
    echo Cloning vcpkg
    pushd ..
    git clone https://github.com/microsoft/vcpkg.git
    popd
    CALL ../vcpkg/bootstrap-vcpkg.bat
) ELSE (
    echo vcpkg is already installed
)

..\vcpkg\vcpkg.exe install bullet3:x64-windows
..\vcpkg\vcpkg.exe install nlohmann-json:x64-windows
..\vcpkg\vcpkg.exe integrate install

mkdir build
pushd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../vcpkg/scripts/buildsystems/vcpkg.cmake
popd
