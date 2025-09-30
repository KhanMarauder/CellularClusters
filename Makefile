# Makefile

.PHONY: all

all:
	@g++ src/main.cpp -o run.elf -lOpenCL -lSDL2 -lSDL2_image -march=native -pthread -O3

installReqDeps:
	sudo apt update
	sudo apt install libsdl2-dev libsdl2-image-dev -y
	sudo apt install libopencl-clang15
