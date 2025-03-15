set "ORIG=%CD%"
cd "%~dp0"
cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH=jsoncpp\install;libjson-rpc-cpp\install
cd "%ORIG%"
