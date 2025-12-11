# Makefile

.PHONY: all

all:
	@echo "If you are compiling on windows, please run cmake. Install by running 'winget install Kitware.CMake'."
	@g++ src/main.cpp -o run.elf -lOpenCL -lSDL2 -lSDL2_image -march=native -pthread -O3

install:
	@sudo rm -f /usr/share/applications/cellularClusters.desktop \
	/usr/bin/cellularClusters.sh
	@echo "[Desktop Entry]" | sudo tee /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Name=CellularClusters" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "GenericName=Text Editor" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Comment=A simulation that follows basic rules to form organic shapes" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Icon=$(HOME)/CellularClusters.png" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Type=Application" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Exec=cellularClusters.sh %F" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "StartupNotify=false" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "Terminal=false" | sudo tee -a /usr/share/applications/cellularClusters.desktop > /dev/null
	@echo "#!/bin/bash" | sudo tee /usr/bin/cellularClusters.sh > /dev/null
	@echo "cd $(HOME)/CellularClusters/" | sudo tee -a /usr/bin/cellularClusters.sh > /dev/null
	@echo "./run.elf" | sudo tee -a /usr/bin/cellularClusters.sh > /dev/null
	@sudo chmod +x /usr/bin/cellularClusters.sh

installReqDeps:
	@sudo apt update
	@sudo apt install libsdl2-dev libsdl2-image-dev -y
	@sudo apt install libopencl-clang15
