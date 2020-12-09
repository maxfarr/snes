#ifndef _CPU_H
#define _CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <utility>

typedef uint16_t twobyte;
typedef uint8_t byte;

#include "ram.h"

#define getBit(value, k)	((value >> k) & 1)

class SNES_CPU {
public:
	SNES_CPU();
	~SNES_CPU();

	// signals
	void clock();
	void reset();
	void irq();
	void nmi();
	
	// operations
	bool ADC(); bool AND(); bool ASL(); bool BCC();
	
	// 8-bit addressing modes
	bool IMP8(); bool IMM8(); bool ABS8();
	
	// 16-bit addressing modes
	bool IMP16(); bool IMM16(); bool ABS16();
	
private:
	// ram
	SNES_RAM ram;

	// accumulator
	twobyte C;
	byte* B = (byte*)&C;
	byte* A = B + 1;
	
	// data bank
	byte DBR;
	
	// direct
	twobyte D;
	byte* DH = (byte*)&D;
	byte* DL = DH + 1;
	
	// program bank
	byte K;
	
	// program counter
	twobyte PC;
	byte* PCH = (byte*)&PC;
	byte* PCL = PCH + 1;
	
	// stack pointer
	twobyte S;
	byte* SH = (byte*)&S;
	byte* SL = SH + 1;
	
	// X
	twobyte X;
	byte* XH = (byte*)&X;
	byte* XL = XH + 1;
	
	// Y
	twobyte Y;
	byte* YH = (byte*)&Y;
	byte* YL = YH + 1;
	
	// flags
	union status {
		struct bits {
			char carry : 1;
			char zero : 1;
			char disable : 1;
			char decimal : 1;
			char index_size : 1;
			char accum_size : 1;
			char overflow : 1;
			char negative : 1;
		};
		char full;
	};
	
	byte cycles;
	
	twobyte fetched;
	twobyte abs_addr;
	twobyte rel_addr;
	
	struct instruction {
		bool (SNES_CPU::*op)(void) = nullptr;
		bool (SNES_CPU::*mode)(void) = nullptr;
		byte cycles = 0;
	};
	
	struct instruction i = {&SNES_CPU::ADC, &SNES_CPU::IMM8, 2};
	
	std::map<byte, struct instruction>* ops;
	
	using cpu = SNES_CPU;
	std::map<byte, struct instruction> ops8 {
		{0x69, {&cpu::ADC, &cpu::IMM8, 2}},
		{0x6D, {&cpu::ADC, &cpu::ABS8, 4}}
	};
};

#endif
