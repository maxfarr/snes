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
	void setM(); void clearM();
	void setI(); void clearI();

	//
	// operations
	//
	
	void ADC16(); void ADC8();
	std::function<void()> ADC = bind_fn(ADC16);
	
	void AND();
	
	void ASL(); void BCC();
	
	//
	// addressing modes
	//
	
	// implied
	void IMP_FN() {return;};
	std::function<void()> IMP = bind_fn(IMP_FN);
	
	// immediate
	void IMM16(); void IMM8();
	std::function<void()> IMM = bind_fn(IMM16);
	
	// direct page
	void DP16(); void DP8();
	std::function<void()> DP = bind_fn(DP16);
	
	void DPX16(); void DPX8();
	std::function<void()> DPX = bind_fn(DPX16);
	
	void DPY16(); void DPY8();
	std::function<void()> DPY = bind_fn(DPY16);
	
	// indirect
	void DPI16(); void DPI8();
	std::function<void()> DPI = bind_fn(DPI16);
	
	void DPIL16(); void DPIL8();
	std::function<void()> DPIL = bind_fn(DPIL16);
	
	void DPIX16(); void DPIX8();
	std::function<void()> DPIX = bind_fn(DPIX16);
	
	void DPIY16(); void DPIY8();
	std::function<void()> DPIY = bind_fn(DPIY16);
	
	void DPILX16(); void DPILX8();
	std::function<void()> DPILX = bind_fn(DPILX16);
	
	void DPILY16(); void DPILY8();
	std::function<void()> DPILY = bind_fn(DPILY16);
	
	// absolute
	void ABS16(); void ABS8();
	std::function<void()> ABS = bind_fn(ABS16);
	
	void ABSL16();  void ABSL8();
	std::function<void()> ABSL = bind_fn(ABSL16);
	
	void ABSX16(); void ABSX8();
	std::function<void()> ABSX = bind_fn(ABS16);
	
	void ABSY16(); void ABSY8();
	std::function<void()> ABSY = bind_fn(ABS16);
	
	void ABSLX16();  void ABSLX8();
	std::function<void()> ABSLX = bind_fn(ABSL16);
	
	void ABSLY16();  void ABSLY8();
	std::function<void()> ABSLY = bind_fn(ABSL16);
	
	// stack relative
	void SR16(); void SR8();
	std::function<void()> SR = bind_fn(SR16);
	
	void SRIX16(); void SRIX8();
	std::function<void()> SRIX = bind_fn(SRIX16);
	
	void SRIY16(); void SRIY8();
	std::function<void()> SRIY = bind_fn(SRIY16);
	
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
		std::function<void()>* op;
		std::function<void()>* mode;
		std::function<byte()> cycleCount;
	} instruction;
	
	std::map<byte, instruction> ops {
		{0x61, {&ADC, &DPIX, [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x63, {&ADC, &SR, [=]() -> byte {return 5 - status.bits.m;}}},
		{0x65, {&ADC, &DP, [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0x67, {&ADC, &DPIL, [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x69, {&ADC, &IMM, [=]() -> byte {return 3 - status.bits.m;}}},
		{0x6D, {&ADC, &ABS, [=]() -> byte {return 5 - status.bits.m;}}},
		{0x6F, {&ADC, &ABSL, [=]() -> byte {return 6 - status.bits.m;}}},
		{0x71, {&ADC, &DPIY, [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x72, {&ADC, &DPI, [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x73, {&ADC, &SRIY, [=]() -> byte {return 8 - status.bits.m;}}},
		{0x75, {&ADC, &DPX, [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0x77, {&ADC, &DPILY, [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x79, {&ADC, &ABSY, [=]() -> byte {return 5 - status.bits.m;}}},
		{0x7D, {&ADC, &ABSX, [=]() -> byte {return 5 - status.bits.m;}}},
		{0x7F, {&ADC, &ABSLX, [=]() -> byte {return 6 - status.bits.m;}}}
	};
};

#endif
