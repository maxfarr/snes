#include "cpu.h"

int main() {
	SNES_CPU cpu;
	return 0;
}

SNES_CPU::SNES_CPU() {
	ops = &ops8;
}

void SNES_CPU::clock() {
	if(cycles == 0) {
		//get opcode
		byte opcode = ram.read8(PC++);
		
		byte additional_cycle_1 = ((this->*ops[opcode]).mode)();
		byte additional_cycle_2 = ((this->*ops[opcode]).op)();
	}
	cycles--;
}
