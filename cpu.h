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

#define DLNONZERO			(*DL != 0x00)

class SNES_CPU {
public:
	SNES_CPU(SNES_MEMORY* m);
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
	//
	// operations
	//
	
	void ADC();
	
	void AND();
	
	void ASL(); void BCC();
	
	//
	// addressing modes
	//
	
	// implied
	void IMP() {return;};
	
	// immediate
	void IMM();
	
	// direct page
	void DP();
	
	void DPX(); void DPY();
	
	// indirect
	void DPI();
	
	void DPIL();
	
	void DPIX(); void DPIY();
	
	void DPILX(); void DPILY();
	
	// absolute
	void ABS();
	
	void ABSL();
	
	void ABSX(); void ABSY();
	
	void ABSLX(); void ABSLY();
	
	// stack relative
	void SR();
	
	void SRIX(); void SRIY();
	
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
	
	bool e;
	
	byte cyclesRemaining = 0;
	
	twobyte fetched = 0x0000;
	byte* fetched_hi = (byte*)&fetched;
	byte* fetched_lo = fetched_hi + 1;
	
	typedef struct {
		std::function<void()> op;
		std::function<void()> mode;
		std::function<byte()> cycleCount;
	} instruction;
	
	std::map<byte, instruction> ops {
		{0x61, {bind_fn(ADC), bind_fn(DPIX), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x63, {bind_fn(ADC), bind_fn(SR), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x65, {bind_fn(ADC), bind_fn(DP), [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0x67, {bind_fn(ADC), bind_fn(DPIL), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x69, {bind_fn(ADC), bind_fn(IMM), [=]() -> byte {return 3 - status.bits.m;}}},
		{0x6D, {bind_fn(ADC), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x6F, {bind_fn(ADC), bind_fn(ABSL), [=]() -> byte {return 6 - status.bits.m;}}},
		{0x71, {bind_fn(ADC), bind_fn(DPIY), [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x72, {bind_fn(ADC), bind_fn(DPI), [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x73, {bind_fn(ADC), bind_fn(SRIY), [=]() -> byte {return 8 - status.bits.m;}}},
		{0x75, {bind_fn(ADC), bind_fn(DPX), [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0x77, {bind_fn(ADC), bind_fn(DPILY), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x79, {bind_fn(ADC), bind_fn(ABSY), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x7D, {bind_fn(ADC), bind_fn(ABSX), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x7F, {bind_fn(ADC), bind_fn(ABSLX), [=]() -> byte {return 6 - status.bits.m;}}}
	};
};

#endif
