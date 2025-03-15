set "ORIG=%CD%"
set "SCRIPT_DIR=%~dp0"
cd "%SCRIPT_DIR%jsoncpp"
cmake -B build -DCMAKE_INSTALL_PREFIX=install -DCMAKE_INSTALL_INCLUDEDIR=include\jsoncpp
cd "%ORIG%"
