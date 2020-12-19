#include "cpu.h"
#include "ram.h"
#include <stdio.h>

int main() {
	// test program initializes SNES components individually
	SNES_RAM ram;
	SNES_CPU cpu(&ram);
	return 0;
}

SNES_CPU::SNES_CPU(SNES_RAM* r) {
	ops = ops16;
	ram = r;
}

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

void SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		//get opcode
		byte opcode = ram->read8(PC++);
		
		cyclesRemaining += (this->ops[opcode].cycleCount)();
		
		(this->*ops[opcode].mode)();
		(this->*ops[opcode].op)();
	}
	cyclesRemaining--;
}

void SNES_CPU::ADC() {
	status.bits.c = (C + (fetched + status.bits.c) > 0xFFFF);
	
	C += (fetched + status.bits.c);
	
	status.bits.n = getBit(C, 15);
	status.bits.z = (C == 0x0000);
}

void SNES_CPU::ADC8() {
	
}

void SNES_CPU::ABS() {
	
}

void SNES_CPU::IMM() {
	
}

void SNES_CPU::IMM8() {
	
}