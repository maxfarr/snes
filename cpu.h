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
#define MZERO				(status.bits.m ? 0 : 2)

class SNES_CPU {
public:
	SNES_CPU(SNES_MEMORY* m);
	~SNES_CPU();

	// signals
	bool clock();
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
	
	void ASL(); void ASLA();

	void LSR(); void LSRA();
	
	void BCC(); void BCS(); void BEQ(); void BMI();
	void BNE(); void BPL(); void BRA(); void BRL();
	void BVC(); void BVS();
	
	void BIT(); void BITIMM();
	
	void CLC(); void CLD();
	void CLI(); void CLV();

	void CMP();
	void CPX();
	void CPY();

	void DEC();
	void DECA();
	void DEX();
	void DEY();

	void INC();
	void INCA();
	void INX();
	void INY();

	void SEC();
	void SED();
	void SEI();

	void STA();
	void STX();
	void STY();
	void STZ();
	
	//
	// addressing modes
	//
	
	// implied
	void IMP() {return;};
	
	// immediate
	void IMM_M(); void IMM_X();
	
	void IMM8(); void IMM16();
	
	// direct page
	void DP();
	
	void DPX(); void DPY();
	
	// indirect
	void DPI();
	
	void DPIL();

	void DPIX();
	
	void DPINY(); void DPILNY();
	
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

	#define GET_STATUS_C (getBit(status.full, 0))
	#define GET_STATUS_Z (getBit(status.full, 1))
	#define GET_STATUS_I (getBit(status.full, 2))
	#define GET_STATUS_D (getBit(status.full, 3))
	#define GET_STATUS_X (getBit(status.full, 4))
	#define GET_STATUS_M (getBit(status.full, 5))
	#define GET_STATUS_V (getBit(status.full, 6))
	#define GET_STATUS_N (getBit(status.full, 7))
	
	bool e;
	
	byte cyclesRemaining = 0;
	
	twobyte fetched = 0x0000;
	byte* fetched_hi = (byte*)&fetched;
	byte* fetched_lo = fetched_hi + 1;
	
	threebyte fetched_addr;
	byte* fetched_addr_bank = (byte*)&fetched_addr;
	byte* fetched_addr_page = fetched_addr_bank + 1;
	twobyte* fetched_addr_abs = (twobyte*)fetched_addr_page;
	byte* fetched_addr_lo = fetched_addr_page + 1;
	
	bool iBoundary = false;
	bool branchTaken = false;
	bool branchBoundary = false;
	bool wrap_writes = false;
	
	typedef struct {
		std::function<void()> op;
		std::function<void()> mode;
		std::function<byte()> cycleCount;
	} instruction;
	
	std::map<byte, instruction> ops {
		// adc
		{0x61, {bind_fn(ADC), bind_fn(DPIX), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x63, {bind_fn(ADC), bind_fn(SR), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x65, {bind_fn(ADC), bind_fn(DP), [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0x67, {bind_fn(ADC), bind_fn(DPIL), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x69, {bind_fn(ADC), bind_fn(IMM_M), [=]() -> byte {return 3 - status.bits.m;}}},
		{0x6D, {bind_fn(ADC), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x6F, {bind_fn(ADC), bind_fn(ABSL), [=]() -> byte {return 6 - status.bits.m;}}},
		{0x71, {bind_fn(ADC), bind_fn(DPINY), [=]() -> byte {return 6 - status.bits.m + DLNONZERO + iBoundary;}}},
		{0x72, {bind_fn(ADC), bind_fn(DPI), [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x73, {bind_fn(ADC), bind_fn(SRIY), [=]() -> byte {return 8 - status.bits.m;}}},
		{0x75, {bind_fn(ADC), bind_fn(DPX), [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0x77, {bind_fn(ADC), bind_fn(DPILNY), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x79, {bind_fn(ADC), bind_fn(ABSY), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0x7D, {bind_fn(ADC), bind_fn(ABSX), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0x7F, {bind_fn(ADC), bind_fn(ABSLX), [=]() -> byte {return 6 - status.bits.m;}}},
		// and
		{0x21, {bind_fn(AND), bind_fn(DPIX), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x23, {bind_fn(AND), bind_fn(SR), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x25, {bind_fn(AND), bind_fn(DP), [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0x27, {bind_fn(AND), bind_fn(DPIL), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x29, {bind_fn(AND), bind_fn(IMM_M), [=]() -> byte {return 3 - status.bits.m;}}},
		{0x2D, {bind_fn(AND), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x2F, {bind_fn(AND), bind_fn(ABSL), [=]() -> byte {return 6 - status.bits.m;}}},
		{0x31, {bind_fn(AND), bind_fn(DPINY), [=]() -> byte {return 6 - status.bits.m + DLNONZERO + iBoundary;}}},
		{0x32, {bind_fn(AND), bind_fn(DPI), [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0x33, {bind_fn(AND), bind_fn(SRIY), [=]() -> byte {return 8 - status.bits.m;}}},
		{0x35, {bind_fn(AND), bind_fn(DPX), [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0x37, {bind_fn(AND), bind_fn(DPILNY), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0x39, {bind_fn(AND), bind_fn(ABSY), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0x3D, {bind_fn(AND), bind_fn(ABSX), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0x3F, {bind_fn(AND), bind_fn(ABSLX), [=]() -> byte {return 6 - status.bits.m;}}},
		// asl
		{0x06, {bind_fn(ASL), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + MZERO;}}},
		{0x0A, {bind_fn(ASLA), bind_fn(IMP), [=]() -> byte {return 2;}}},
		{0x0E, {bind_fn(ASL), bind_fn(ABS), [=]() -> byte {return 6 + MZERO;}}},
		{0x16, {bind_fn(ASL), bind_fn(DPX), [=]() -> byte {return 5 + DLNONZERO + MZERO;}}},
		{0x1E, {bind_fn(ASL), bind_fn(ABSX), [=]() -> byte {return 7 + MZERO;}}},
		// lsr
		{0x46, {bind_fn(LSR), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + MZERO;}}},
		{0x4A, {bind_fn(LSRA), bind_fn(IMP), [=]() -> byte {return 2;}}},
		{0x4E, {bind_fn(LSR), bind_fn(ABS), [=]() -> byte {return 6 + MZERO;}}},
		{0x56, {bind_fn(LSR), bind_fn(DPX), [=]() -> byte {return 5 + DLNONZERO + MZERO;}}},
		{0x5E, {bind_fn(LSR), bind_fn(ABSX), [=]() -> byte {return 7 + MZERO;}}},
		// branching
		{0x90, {bind_fn(BCC), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0xB0, {bind_fn(BCS), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0xF0, {bind_fn(BEQ), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0x30, {bind_fn(BMI), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0xD0, {bind_fn(BNE), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0x10, {bind_fn(BPL), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0x80, {bind_fn(BRA), bind_fn(IMM8), [=]() -> byte {return 3;}}},
		{0x82, {bind_fn(BRL), bind_fn(IMM16), [=]() -> byte {return 4;}}},
		{0x50, {bind_fn(BVC), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		{0x70, {bind_fn(BVS), bind_fn(IMM8), [=]() -> byte {return 2 + branchTaken;}}},
		// bit
		{0x24, {bind_fn(BIT), bind_fn(DP), [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0x2C, {bind_fn(BIT), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.m;}}},
		{0x34, {bind_fn(BIT), bind_fn(DPX), [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0x3C, {bind_fn(BIT), bind_fn(ABSX), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0x89, {bind_fn(BITIMM), bind_fn(IMM_M), [=]() -> byte {return 3 - status.bits.m;}}},
		// clear flags
		{0x18, {bind_fn(CLC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xD8, {bind_fn(CLD), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x58, {bind_fn(CLI), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xB8, {bind_fn(CLV), bind_fn(IMP), []() -> byte {return 2;}}},
		// cmp
		{0xC1, {bind_fn(CMP), bind_fn(DPIX), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0xC3, {bind_fn(CMP), bind_fn(SR), [=]() -> byte {return 5 - status.bits.m;}}},
		{0xC5, {bind_fn(CMP), bind_fn(DP), [=]() -> byte {return 4 - status.bits.m + DLNONZERO;}}},
		{0xC7, {bind_fn(CMP), bind_fn(DPIL), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0xC9, {bind_fn(CMP), bind_fn(IMM_M), [=]() -> byte {return 3 - status.bits.m;}}},
		{0xCD, {bind_fn(CMP), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.m;}}},
		{0xCF, {bind_fn(CMP), bind_fn(ABSL), [=]() -> byte {return 6 - status.bits.m;}}},
		{0xD1, {bind_fn(CMP), bind_fn(DPINY), [=]() -> byte {return 6 - status.bits.m + DLNONZERO + iBoundary;}}},
		{0xD2, {bind_fn(CMP), bind_fn(DPI), [=]() -> byte {return 6 - status.bits.m + DLNONZERO;}}},
		{0xD3, {bind_fn(CMP), bind_fn(SRIY), [=]() -> byte {return 8 - status.bits.m;}}},
		{0xD5, {bind_fn(CMP), bind_fn(DPX), [=]() -> byte {return 5 - status.bits.m + DLNONZERO;}}},
		{0xD7, {bind_fn(CMP), bind_fn(DPILNY), [=]() -> byte {return 7 - status.bits.m + DLNONZERO;}}},
		{0xD9, {bind_fn(CMP), bind_fn(ABSY), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0xDD, {bind_fn(CMP), bind_fn(ABSX), [=]() -> byte {return 5 - status.bits.m + iBoundary;}}},
		{0xDF, {bind_fn(CMP), bind_fn(ABSLX), [=]() -> byte {return 6 - status.bits.m;}}},
		// cpx, cpy
		{0xE0, {bind_fn(CPX), bind_fn(IMM_X), [=]() -> byte {return 3 - status.bits.x;}}},
		{0xE4, {bind_fn(CPX), bind_fn(DP), [=]() -> byte {return 4 - status.bits.x + DLNONZERO;}}},
		{0xEC, {bind_fn(CPX), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.x;}}},
		{0xC0, {bind_fn(CPY), bind_fn(IMM_X), [=]() -> byte {return 3 - status.bits.x;}}},
		{0xC4, {bind_fn(CPY), bind_fn(DP), [=]() -> byte {return 4 - status.bits.x + DLNONZERO;}}},
		{0xCC, {bind_fn(CPY), bind_fn(ABS), [=]() -> byte {return 5 - status.bits.x;}}},
		// dec, dex, dey
		{0x3A, {bind_fn(DECA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xC6, {bind_fn(DEC), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + MZERO;}}},
		{0xCE, {bind_fn(DEC), bind_fn(ABS), [=]() -> byte {return 6 + MZERO;}}},
		{0xD6, {bind_fn(DEC), bind_fn(DPX), [=]() -> byte {return 6 + DLNONZERO + MZERO;}}},
		{0xDE, {bind_fn(DEC), bind_fn(ABSX), [=]() -> byte {return 7 + MZERO;}}},
		{0xCA, {bind_fn(DEX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x88, {bind_fn(DEY), bind_fn(IMP), []() -> byte {return 2;}}},
		// inc, inx, iny
		{0x1A, {bind_fn(INCA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xEE, {bind_fn(INC), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + (2 * MZERO);}}},
		{0xE6, {bind_fn(INC), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0xFE, {bind_fn(INC), bind_fn(ABSX), [=]() -> byte {return 7 + (2 * MZERO);}}},
		{0xF6, {bind_fn(INC), bind_fn(DPX), [=]() -> byte {return 6 + DLNONZERO + (2 * MZERO);}}},
		{0xE8, {bind_fn(INX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xC8, {bind_fn(INY), bind_fn(IMP), []() -> byte {return 2;}}},
		// sec, sed, sei
		{0x38, {bind_fn(SEC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x78, {bind_fn(SEI), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xF8, {bind_fn(SED), bind_fn(IMP), []() -> byte {return 2;}}},
		// sta
		{0x8D, {bind_fn(STA), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x8F, {bind_fn(STA), bind_fn(ABSL), [=]() -> byte {return 5 + MZERO;}}},
		{0x85, {bind_fn(STA), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x92, {bind_fn(STA), bind_fn(DPI), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0x87, {bind_fn(STA), bind_fn(DPIL), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x9D, {bind_fn(STA), bind_fn(ABSX), [=]() -> byte {return 5 + MZERO;}}},
		{0x9F, {bind_fn(STA), bind_fn(ABSLX), [=]() -> byte {return 5 + MZERO;}}},
		{0x99, {bind_fn(STA), bind_fn(ABSY), [=]() -> byte {return 5 + MZERO;}}},
		{0x95, {bind_fn(STA), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x81, {bind_fn(STA), bind_fn(DPIX), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x91, {bind_fn(STA), bind_fn(DPINY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x97, {bind_fn(STA), bind_fn(DPILNY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x83, {bind_fn(STA), bind_fn(SR), [=]() -> byte {return 4 + MZERO;}}},
		{0x93, {bind_fn(STA), bind_fn(SRIY), [=]() -> byte {return 7 + MZERO;}}},
		// stx, sty, stz
		{0x8E, {bind_fn(STX), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x86, {bind_fn(STX), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x96, {bind_fn(STX), bind_fn(DPY), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x8C, {bind_fn(STY), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x84, {bind_fn(STY), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x94, {bind_fn(STY), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x9C, {bind_fn(STZ), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x64, {bind_fn(STZ), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x9E, {bind_fn(STZ), bind_fn(ABSX), [=]() -> byte {return 5 + MZERO;}}},
		{0x74, {bind_fn(STZ), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}}
	};
};

#endif