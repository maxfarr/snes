#include "common.h"

class SNES_CPU;
#include "cpu.hpp"
class SNES_MEMORY;
#include "ram.hpp"
#include "snes.hpp"

#include <stdio.h>
#include <iostream>
#include <iomanip>

int main() {
    SNES s;
    s.run();
	std::cout << "completed execution!" << std::endl;
	
	return 0;
}

SNES::SNES() : cpu(&cpu_apu_io), apu(&cpu_apu_io) {
	ready = false;
	std::cout << "running it!" << std::endl;
	std::string filename;
	std::cin >> filename;
	std::cout << "reading ROM file: " << filename << std::endl;
	if ((cpu.mem)->openROM(filename)) {
		ready = true;
	}
	std::cout << "finished reading file" << std::endl;

// todo: why is this here?
#ifdef FORCE_RESET_TO_8000
	(cpu.mem)->override_reset_vector(0x8000);
#endif
}

void SNES::run() {
	if(!ready) return;
    cpu.init();

    for(int i = 0; i < 10000; i++) {
		if (!cpu.clock()) {
			break;
		}
	}
}

SNES::~SNES() {
	//nothing yet
}
