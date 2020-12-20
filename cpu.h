#ifndef _CPU_H
#define _CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <utility>
#include <functional>

typedef uint16_t twobyte;
typedef uint8_t byte;

class SNES_RAM;
#include "ram.h"

#define getBit(value, k)	((value >> k) & 1)

class SNES_CPU {
public:
	SNES_CPU(SNES_RAM* r, SNES_RAM* r2);
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
	void IMP() {return;};
	void IMM(); void ABS();
	
	// 8-bit addressing modes
	void IMM8(); void ABS8();
	
	twobyte debugAccum() {return C;};
	byte getCycles() {return cyclesRemaining;};
	
private:
	// memory
	SNES_RAM* ram = nullptr;
	SNES_RAM* rom = nullptr;

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
	union {
		struct {
			char n : 1;
			char v : 1;
			char m : 1;
			char x : 1;
			char d : 1;
			char i : 1;
			char z : 1;
			char c : 1;
		} bits;
		char full;
	} status;
	
	byte cyclesRemaining;
	
	twobyte fetched;
	byte* fetched_hi = (byte*)&fetched;
	byte* fetched_lo = fetched_hi + 1;
	twobyte abs_addr;
	twobyte rel_addr;
	
	typedef struct {
		void (SNES_CPU::*op)(void) = nullptr;
		void (SNES_CPU::*mode)(void) = nullptr;
		std::function<byte()> cycleCount;
	} instruction;
	
	std::map<byte, instruction> ops;
	
	using cpu = SNES_CPU;
	std::map<byte, instruction> ops16 {
		{0x69, {&cpu::ADC, &cpu::IMM, [=]() -> byte {return (byte)(3 - status.bits.m);}}},
		{0x6D, {&cpu::ADC, &cpu::ABS, [=]() -> byte {return (byte)(4);}}} // wip
		
	};
};

#endif
