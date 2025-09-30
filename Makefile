# Makefile

.PHONY: all

# Detect OS
ifeq ($(OS),Windows_NT)
	PLATFORM = windows
else
	PLATFORM = $(shell uname)
endif

all:
	ifeq ($PLATFORM, windows)
		@echo "Invalid OS. Can't build for windows yet. (Make should exit with an error)"
		exit
	
	@g++ src/main.cpp -o run.elf -lOpenCL -lSDL2 -lSDL2_image -march=native -pthread -O3

installReqDeps:
	sudo apt update
	sudo apt install libsdl2-dev libsdl2-image-dev -y
	sudo apt install libopencl-clang15
