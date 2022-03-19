if not exist build mkdir build
cd build
cmake -S ../ -B . -G "MinGW Makefiles"
cmake --build . --config Release
cd ..