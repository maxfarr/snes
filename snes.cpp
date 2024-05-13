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
	
	return 0;
}

SNES::SNES() {
	std::string filename;
	filename << std::cin;
	std::cout << "reading ROM file: " << filename << std::endl;
	(cpu.mem)->openROM(filename);
	std::cout << "finished reading file" << std::endl;

// todo: why is this here?
#ifdef FORCE_RESET_TO_8000
	(cpu.mem)->override_reset_vector(0x8000);
#endif
}

void SNES::run() {
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