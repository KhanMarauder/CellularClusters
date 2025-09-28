# Makefile

.PHONY: all

all:
	@g++ src/main.cpp -o run.elf -lOpenCL -lSDL2 -lSDL2_image -O3

install_req_dependancies:
	sudo apt update
	sudo apt install libsdl2-dev libsdl2-dev -y
