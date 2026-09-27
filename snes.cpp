#include "common.h"

#include "cpu.hpp"
#include "ram.hpp"
#include "snes.hpp"

#include <memory>
#include <stdio.h>
#include <iostream>
#include <iomanip>

int main() {
    SNES s;
    s.run();
	std::cout << "completed execution!" << std::endl;
	
	return 0;
}

SNES::SNES() {
	apu = std::make_unique<SNES_APU>(&cpu_apu_io);
	mem = std::make_unique<SNES_MEMORY>(&cpu_apu_io);
	cpu = std::make_unique<SNES_CPU>(mem.get());
	ready = false;
	std::cout << "running it!" << std::endl;
	std::string filename;
	std::cin >> filename;
	std::cout << "reading ROM file: " << filename << std::endl;
	if (mem->openROM(filename)) {
		ready = true;
	}
	std::cout << "finished reading file" << std::endl;
}

void SNES::run() {
	if(!ready) return;
    cpu->init();

    for(int i = 0; i < 10000; i++) {
		if (!cpu->clock()) {
			break;
		}
	}
}

SNES::~SNES() {
	//nothing yet
}
