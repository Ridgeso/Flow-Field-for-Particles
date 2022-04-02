CurrentDir=.
BuildSRC=build
BinarySRC=bin

ifeq ($(OS),Windows_NT)
CMFlags=-G "MinGW Makefiles"
RemoveFolder=rmdir /s /q
p=".\"
ext=.exe
else
CMFlags=
RemoveFolder=rm -rf
p=./
ext=
endif

.PHONY: debug build run

all: build main

main:
	cmake --build ${BuildSRC} --config Release

build:
	cmake -S $(CurrentDir) -B $(BuildSRC) $(CMFlags) -DCMAKE_BUILD_TYPE="Release"

debug:
	cmake -S $(CurrentDir) -B $(BuildSRC) $(CMFlags) -DCMAKE_BUILD_TYPE="Debug"
	cmake --build ${BuildSRC} --config Debug

run:
	cd ${BinarySRC} && ${p}Particle${ext}

clean:
	$(RemoveFolder) $(BuildSRC)
	$(RemoveFolder) $(BinarySRC)