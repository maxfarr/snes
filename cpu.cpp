#include "cpu.h"
#include "ram.h"
#include <stdio.h>
#include <iostream>

#define DEBUG

int main() {
	// test program initializes SNES components individually
	SNES_RAM ram;
	SNES_RAM rom;
	std::cout << "reading file" << std::endl;
	rom.readFile("testrom_3adc");
	std::cout << "read file" << std::endl;
	SNES_CPU cpu(&ram, &rom);
	
	for(int i = 0; i < 9; i++) {
		cpu.clock();
	}
	
	return 0;
}

SNES_CPU::SNES_CPU(SNES_RAM* r, SNES_RAM* r2) {
	ops = ops16;
	ram = r;
	rom = r2;
}

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

void SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		//get opcode
		byte opcode = rom->read8(PC++);
		
		cyclesRemaining += (this->ops[opcode].cycleCount)();
		
		(this->*ops[opcode].mode)();
		(this->*ops[opcode].op)();
		
#ifdef DEBUG
		std::cout << "-- executed opcode 0x" << std::hex << (unsigned int)opcode << std::dec << std::endl;
		std::cout << "status flags: " << std::endl;
		std::cout << "n v m x d i z c" << std::endl;
		for(int i = 7; i >= 0; i--)
			std::cout << getBit(status.full, i) << " ";
		std::cout << std::endl;
		std::cout << "accumulator: " << std::endl;
		for(int i = 15; i >= 0; i--)
			std::cout << getBit(C, i);
		std::cout << std::endl << std::endl;
#endif
	}
	cyclesRemaining--;
}

void SNES_CPU::ADC() {
	char final_c = ((unsigned int)C + (fetched + status.bits.c) > (unsigned int)0xFFFF);
	
	C += (fetched + status.bits.c);
	
	status.bits.c = final_c;
	status.bits.v = ((int)C < -32768 || (int)C > 32767);
	status.bits.n = getBit(C, 15);
	status.bits.z = (C == 0x0000);
}

void SNES_CPU::ADC8() {
	status.bits.c = (*A + (fetched + status.bits.c) > 0xFF);
	
	*A += (fetched + status.bits.c);
	
	status.bits.v = ((int)*A < -128 || (int)*A > 127);
	status.bits.n = getBit(*A, 7);
	status.bits.z = (*A == 0x00);
}

void SNES_CPU::ABS() {
	
}

void SNES_CPU::IMM() {
	fetched = (rom->read8(PC++) << 8);
	fetched |= rom->read8(PC++);
}

void SNES_CPU::IMM8() {
	fetched = rom->read8(PC++);
}