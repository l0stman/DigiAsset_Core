cd jsoncpp
cmake -B build -DCMAKE_INSTALL_PREFIX=install -DCMAKE_INSTALL_INCLUDEDIR=jsoncpp
cd ..\libjson-rpc-cpp
cmake -B build -DCMAKE_INSTALL_PREFIX=install -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH=..\jsoncpp\install
cd ..
