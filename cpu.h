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
	void ADC(); void ADC8();
	void AND(); void ASL(); void BCC();
	
	// 16-bit addressing modes
	void IMP(); void IMM(); void ABS();
	
	// 8-bit addressing modes
	void IMP8(); void IMM8(); void ABS8();
	
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
			char c : 1;
			char z : 1;
			char i : 1;
			char d : 1;
			char x : 1;
			char m : 1;
			char v : 1;
			char n : 1;
		};
		char full;
	};
	
	byte cyclesRemaining;
	
	twobyte fetched;
	twobyte abs_addr;
	twobyte rel_addr;
	
	typedef struct {
		bool (SNES_CPU::*op)(void) = nullptr;
		bool (SNES_CPU::*mode)(void) = nullptr;
		byte cycleCount();
	} instruction;
	
	std::map<byte, instruction> ops;
	
	using cpu = SNES_CPU;
	std::map<byte, instruction> ops16 {
		{0x69, {&cpu::ADC, &cpu::IMM, [=](){return 3 - status.bits.m;}}},
		{0x6D, {&cpu::ADC, &cpu::ABS, 4}}
	};
};

#endif
