#ifndef _CPU_H
#define _CPU_H

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <utility>
#include <functional>

class SNES_MEMORY;
#include "ram.h"

#define getBit(value, k)	((value >> k) & 1)
#define DLNONZERO			(*DL != 0x00)

class SNES_CPU {
public:
	SNES_CPU(SNES_MEMORY* m) : mem(m), PC(0x8000), ops(ops16) {status.full = 0x00;}
	~SNES_CPU();

	// signals
	void clock();
	void reset();
	void irq();
	void nmi();
	
	twobyte debugAccum() {return C;};
	void debugPrint();
	byte getCycles() {return cyclesRemaining;};
	
private:
	// operations
	void ADC(); void ADC8();
	void AND(); void ASL(); void BCC();
	
	void IMP() {return;};
	
	// 16-bit addressing modes
	void IMM();
	void ABS(); void ABSL();
	void DP(); void DPI(); void DPIL();
	
	// 8-bit addressing modes
	void IMM8();
	void ABS8(); void ABSL8();
	void DP8(); void DPI8(); void DPIL8();

	// memory
	SNES_MEMORY* mem;

	// accumulator
	twobyte C = 0x0000;
	byte* B = (byte*)&C;
	byte* A = B + 1;
	
	// data bank
	byte DBR = 0x00;
	
	// direct page
	twobyte D = 0x0000;
	byte* DH = (byte*)&D;
	byte* DL = DH + 1;
	
	// program bank
	byte K = 0x00;
	
	// program counter
	twobyte PC = 0x0000;
	byte* PCH = (byte*)&PC;
	byte* PCL = PCH + 1;
	
	// stack pointer
	twobyte S = 0x0000;
	byte* SH = (byte*)&S;
	byte* SL = SH + 1;
	
	// X
	twobyte X = 0x0000;
	byte* XH = (byte*)&X;
	byte* XL = XH + 1;
	
	// Y
	twobyte Y = 0x0000;
	byte* YH = (byte*)&Y;
	byte* YL = YH + 1;
	
	// flags
	union {
		struct {
			char c : 1;
			char z : 1;
			char i : 1;
			char d : 1;
			char x : 1;
			char m : 1;
			char v : 1;
			char n : 1;
		} bits;
		char full;
	} status;
	
	byte cyclesRemaining = 0;
	
	twobyte fetched = 0x0000;
	byte* fetched_hi = (byte*)&fetched;
	byte* fetched_lo = fetched_hi + 1;
	
	typedef struct {
		void (SNES_CPU::*op)(void) = nullptr;
		void (SNES_CPU::*mode)(void) = nullptr;
		std::function<byte()> cycleCount;
	} instruction;
	
	using cpu = SNES_CPU;
	std::map<byte, instruction> ops16 {
		{0x65, {&cpu::ADC, &cpu::DP, [=]() -> byte {return 4 + DLNONZERO;}}},
		{0x69, {&cpu::ADC, &cpu::IMM, [=]() -> byte {return 3;}}},
		{0x6D, {&cpu::ADC, &cpu::ABS, [=]() -> byte {return 5;}}},
		{0x6F, {&cpu::ADC, &cpu::ABSL, [=]() -> byte {return 6;}}}
	};
	
	std::map<byte, instruction> ops;
};

#endif
