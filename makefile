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

.PHONY: mainDebug build run

all: build main

main:
	cmake --build ${BuildSRC} --config Release

mainDebug:
	cmake --build ${BuildSRC} --config Debug

build:
	cmake -S $(CurrentDir) -B $(BuildSRC) $(CMFlags)

run:
	cd ${p}bin && ${p}Particle${ext}

clean:
	$(RemoveFolder) $(BuildSRC)
	$(RemoveFolder) $(BinarySRC)