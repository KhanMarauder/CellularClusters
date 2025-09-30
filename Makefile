# Makefile

.PHONY: all

all:
	@echo "If you are compiling on windows, please run cmake. Install by running 'winget install Kitware.CMake'."
	@g++ src/main.cpp -o run.elf -lOpenCL -lSDL2 -lSDL2_image -march=native -pthread -O3

installReqDeps:
	sudo apt update
	sudo apt install libsdl2-dev libsdl2-image-dev -y
	sudo apt install libopencl-clang15
