#include "cpu.h"
#include <stdio.h>

int main() {
	SNES_CPU cpu;
	return 0;
}

SNES_CPU::SNES_CPU() {
	ops = &ops8;
}

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

void SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		//get opcode
		byte opcode = ram.read8(PC++);
		
		instruction& i = this->ops[opcode];
		cyclesRemaining += i.cycleCount)();
		
		i.mode();
		i.op();
	}
	cyclesRemaining--;
}

bool SNES_CPU::ADC() {
	status.bits.c = (C + (fetched + status.bits.c) > 0xFFFF);
	
	C += (fetched + status.bits.c);
	
	status.bits.n = getFlag(C, 15);
	status.bits.z = (C == 0x0000);
}

bool SNES_CPU::ADC8() {
	
}

bool SNES_CPU::IMM8() {
	
}