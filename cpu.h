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
#define MZERO				(status.bits.m ? 0 : 1)
#define XZERO				(status.bits.x ? 0 : 1)
#define EZERO				(e ? 0 : 1)

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
	// utils
	void updateRegisterWidths();

	//
	// operations
	//
	
	void ADC();
	
	void AND();
	
	void ASL(); void ASLA();
	
	void BCC(); void BCS(); void BEQ(); void BMI();
	void BNE(); void BPL(); void BRA(); void BRL();
	void BVC(); void BVS();
	
	void BIT(); void BITIMM();

	void BRK(); void COP();
	
	void CLC(); void CLD();
	void CLI(); void CLV();

	void CMP();
	void CPX();
	void CPY();

	void DEC();
	void DECA();
	void DEX();
	void DEY();

	void EOR();

	void INC();
	void INCA();
	void INX();
	void INY();

	void JMP();
	void JML();
	void JSR();
	void JSL();

	void LDA();
	void LDX();
	void LDY();

	void LSR(); void LSRA();

	void MVN();
	void MVP();

	void NOP() {return;};

	void ORA();

	void PEA();
	void PEI();
	void PER();

	void PHA(); void PHB(); void PHD(); void PHK();
	void PHP(); void PHX(); void PHY();

	void PLA(); void PLB(); void PLD();
	void PLP(); void PLX(); void PLY();

	void REP();

	void ROL();
	void ROR();

	void RTI();

	void RTS();
	void RTL();

	void SBC();

	void SEC();
	void SEI();
	void SED();

	void SEP();

	void STA();
	void STX();
	void STY();
	void STZ();

	void TAX(); void TAY(); void TCD(); void TCS();
	void TDC(); void TSC(); void TSX(); void TXA();
	void TXS(); void TXY(); void TYA(); void TYX();

	void TRB();
	void TSB();

	void WAI();

	void XBA();
	void XCE();

	//
	// hardware interrupts
	//

	void ABORT();
	void IRQ();
	void NMI();
	void RESET();
	
	//
	// addressing modes
	//
	
	// implied
	void IMP() {return;};
	
	// immediate
	void IMM_M(); void IMM_X();
	
	void IMM8(); void IMM16();
	
	// direct page
	void DP(); void DP16();
	
	void DPX(); void DPY();
	
	// indirect
	void DPI();
	
	void DPIL();

	void DPIX();
	
	void DPINY(); void DPILNY();
	
	// absolute
	void ABS(); void ABS_JMP_JSR();
	
	void ABSL(); void ABSL_JML_JSL();
	
	void ABSX(); void ABSY();
	
	void ABSLX(); void ABSLY();

	void ABSI(); void ABSIX();

	void ABSIL();
	
	// stack relative
	void SR();
	
	void SRIX(); void SRIY();
	
	// memory
	SNES_MEMORY* mem;

	// accumulator
	twobyte C = 0x0000;
	//byte* B = (byte*)&C;
	//byte* A = B + 1;
	byte* A = (byte*)&C;
	byte* B = A + 1;
	
	// data bank
	byte DBR = 0x00;
	
	// direct page
	twobyte D = 0x0000;
	//byte* DH = (byte*)&D;
	//byte* DL = DH + 1;
	byte* DL = (byte*)&D;
	byte* DH = DL + 1;
	
	// program bank
	byte K = 0x00;
	
	// program counter
	twobyte PC = 0x0000;
	//byte* PCH = (byte*)&PC;
	//byte* PCL = PCH + 1;
	byte* PCL = (byte*)&PC;
	byte* PCH = PCL + 1;
	
	// stack pointer
	twobyte S = 0x0000;
	//byte* SH = (byte*)&S;
	//byte* SL = SH + 1;
	byte* SL = (byte*)&S;
	byte* SH = SL + 1;

	void push_stack_threebyte(threebyte value);
	void push_stack_twobyte(twobyte value);
	void push_stack_byte(byte value);
	threebyte pop_stack_threebyte();
	twobyte pop_stack_twobyte();
	byte pop_stack_byte();
	
	// X
	twobyte X = 0x0000;
	//byte* XH = (byte*)&X;
	//byte* XL = XH + 1;
	byte* XL = (byte*)&X;
	byte* XH = XL + 1;
	
	// Y
	twobyte Y = 0x0000;
	//byte* YH = (byte*)&Y;
	//byte* YL = YH + 1;
	byte* YL = (byte*)&Y;
	byte* YH = YL + 1;
	
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
	//byte* fetched_hi = (byte*)&fetched;
	//byte* fetched_lo = fetched_hi + 1;
	byte* fetched_lo = (byte*)&fetched;
	byte* fetched_hi = fetched_lo + 1;
	
	threebyte fetched_addr;
	byte* fetched_addr_lo = (byte*)&fetched_addr;
	byte* fetched_addr_page = fetched_addr_lo + 1;
	byte* fetched_addr_bank = fetched_addr_page + 1;
	twobyte* fetched_addr_abs = (twobyte*)&fetched_addr;

	threebyte jump_long_addr;
	
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
		{0x61, {bind_fn(ADC), bind_fn(DPIX), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x63, {bind_fn(ADC), bind_fn(SR), [=]() -> byte {return 5 + MZERO;}}},
		{0x65, {bind_fn(ADC), bind_fn(DP), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x67, {bind_fn(ADC), bind_fn(DPIL), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x69, {bind_fn(ADC), bind_fn(IMM_M), [=]() -> byte {return 3 + MZERO;}}},
		{0x6D, {bind_fn(ADC), bind_fn(ABS), [=]() -> byte {return 5 + MZERO;}}},
		{0x6F, {bind_fn(ADC), bind_fn(ABSL), [=]() -> byte {return 6 + MZERO;}}},
		{0x71, {bind_fn(ADC), bind_fn(DPINY), [=]() -> byte {return 6 + MZERO + DLNONZERO + iBoundary;}}},
		{0x72, {bind_fn(ADC), bind_fn(DPI), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x73, {bind_fn(ADC), bind_fn(SRIY), [=]() -> byte {return 8 + MZERO;}}},
		{0x75, {bind_fn(ADC), bind_fn(DPX), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0x77, {bind_fn(ADC), bind_fn(DPILNY), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x79, {bind_fn(ADC), bind_fn(ABSY), [=]() -> byte {return 5 + MZERO + iBoundary;}}},
		{0x7D, {bind_fn(ADC), bind_fn(ABSX), [=]() -> byte {return 5 + MZERO + iBoundary;}}},
		{0x7F, {bind_fn(ADC), bind_fn(ABSLX), [=]() -> byte {return 6 + MZERO;}}},
		// and
		{0x21, {bind_fn(AND), bind_fn(DPIX), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x23, {bind_fn(AND), bind_fn(SR), [=]() -> byte {return 5 + MZERO;}}},
		{0x25, {bind_fn(AND), bind_fn(DP), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x27, {bind_fn(AND), bind_fn(DPIL), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x29, {bind_fn(AND), bind_fn(IMM_M), [=]() -> byte {return 3 + MZERO;}}},
		{0x2D, {bind_fn(AND), bind_fn(ABS), [=]() -> byte {return 5 + MZERO;}}},
		{0x2F, {bind_fn(AND), bind_fn(ABSL), [=]() -> byte {return 6 + MZERO;}}},
		{0x31, {bind_fn(AND), bind_fn(DPINY), [=]() -> byte {return 6 + MZERO + DLNONZERO + iBoundary;}}},
		{0x32, {bind_fn(AND), bind_fn(DPI), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x33, {bind_fn(AND), bind_fn(SRIY), [=]() -> byte {return 8 + MZERO;}}},
		{0x35, {bind_fn(AND), bind_fn(DPX), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0x37, {bind_fn(AND), bind_fn(DPILNY), [=]() -> byte {return 7 + MZERO + DLNONZERO;}}},
		{0x39, {bind_fn(AND), bind_fn(ABSY), [=]() -> byte {return 5 + MZERO + iBoundary;}}},
		{0x3D, {bind_fn(AND), bind_fn(ABSX), [=]() -> byte {return 5 + MZERO + iBoundary;}}},
		{0x3F, {bind_fn(AND), bind_fn(ABSLX), [=]() -> byte {return 6 + MZERO;}}},
		// asl
		{0x06, {bind_fn(ASL), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + (2 * MZERO);}}},
		{0x0A, {bind_fn(ASLA), bind_fn(IMP), [=]() -> byte {return 2;}}},
		{0x0E, {bind_fn(ASL), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0x16, {bind_fn(ASL), bind_fn(DPX), [=]() -> byte {return 5 + DLNONZERO + (2 * MZERO);}}},
		{0x1E, {bind_fn(ASL), bind_fn(ABSX), [=]() -> byte {return 7 + (2 * MZERO);}}},
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
		// interrupts
		{0x00, {bind_fn(BRK), bind_fn(IMP), []() -> byte {return 7;}}},
		{0x02, {bind_fn(COP), bind_fn(IMP), []() -> byte {return 7;}}},
		// clear flags
		{0x18, {bind_fn(CLC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xD8, {bind_fn(CLD), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x58, {bind_fn(CLI), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xB8, {bind_fn(CLV), bind_fn(IMP), []() -> byte {return 2;}}},
		// cmp
		{0xC9, {bind_fn(CMP), bind_fn(IMM_M), [=]() -> byte {return 2 + MZERO;}}},
		{0xCD, {bind_fn(CMP), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0xCF, {bind_fn(CMP), bind_fn(ABSL), [=]() -> byte {return 5 + MZERO;}}},
		{0xC5, {bind_fn(CMP), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0xD2, {bind_fn(CMP), bind_fn(DPI), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0xC7, {bind_fn(CMP), bind_fn(DPIL), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xDD, {bind_fn(CMP), bind_fn(ABSX), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0xDF, {bind_fn(CMP), bind_fn(ABSLX), [=]() -> byte {return 5 + MZERO;}}},
		{0xD9, {bind_fn(CMP), bind_fn(ABSY), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0xD5, {bind_fn(CMP), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0xC1, {bind_fn(CMP), bind_fn(DPIX), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xD1, {bind_fn(CMP), bind_fn(DPINY), [=]() -> byte {return 5 + MZERO + DLNONZERO + iBoundary;}}},
		{0xD7, {bind_fn(CMP), bind_fn(DPILNY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xC3, {bind_fn(CMP), bind_fn(SR), [=]() -> byte {return 4 + MZERO;}}},
		{0xD3, {bind_fn(CMP), bind_fn(SRIY), [=]() -> byte {return 7 + MZERO;}}},
		// cpx
		{0xE0, {bind_fn(CPX), bind_fn(IMM_X), [=]() -> byte {return 2 + MZERO;}}},
		{0xEC, {bind_fn(CPX), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0xE4, {bind_fn(CPX), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		// cpy
		{0xC0, {bind_fn(CPY), bind_fn(IMM_X), [=]() -> byte {return 2 + MZERO;}}},
		{0xCC, {bind_fn(CPY), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0xC4, {bind_fn(CPY), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		// dec, dex, dey
		{0x3A, {bind_fn(DECA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xC6, {bind_fn(DEC), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + (2 * MZERO);}}},
		{0xCE, {bind_fn(DEC), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0xD6, {bind_fn(DEC), bind_fn(DPX), [=]() -> byte {return 6 + DLNONZERO + (2 * MZERO);}}},
		{0xDE, {bind_fn(DEC), bind_fn(ABSX), [=]() -> byte {return 7 + (2 * MZERO);}}},
		{0xCA, {bind_fn(DEX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x88, {bind_fn(DEY), bind_fn(IMP), []() -> byte {return 2;}}},
		// eor
		{0x49, {bind_fn(EOR), bind_fn(IMM_M), [=]() -> byte {return 2 + MZERO;}}},
		{0x4D, {bind_fn(EOR), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x4F, {bind_fn(EOR), bind_fn(ABSL), [=]() -> byte {return 5 + MZERO;}}},
		{0x45, {bind_fn(EOR), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x52, {bind_fn(EOR), bind_fn(DPI), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0x47, {bind_fn(EOR), bind_fn(DPIL), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x5D, {bind_fn(EOR), bind_fn(ABSX), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0x5F, {bind_fn(EOR), bind_fn(ABSLX), [=]() -> byte {return 5 + MZERO;}}},
		{0x59, {bind_fn(EOR), bind_fn(ABSY), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0x55, {bind_fn(EOR), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x41, {bind_fn(EOR), bind_fn(DPIX), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x51, {bind_fn(EOR), bind_fn(DPINY), [=]() -> byte {return 5 + MZERO + DLNONZERO + iBoundary;}}},
		{0x57, {bind_fn(EOR), bind_fn(DPILNY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x43, {bind_fn(EOR), bind_fn(SR), [=]() -> byte {return 4 + MZERO;}}},
		{0x53, {bind_fn(EOR), bind_fn(SRIY), [=]() -> byte {return 7 + MZERO;}}},
		// inc, inx, iny
		{0x1A, {bind_fn(INCA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xEE, {bind_fn(INC), bind_fn(DP), [=]() -> byte {return 5 + DLNONZERO + (2 * MZERO);}}},
		{0xE6, {bind_fn(INC), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0xFE, {bind_fn(INC), bind_fn(ABSX), [=]() -> byte {return 7 + (2 * MZERO);}}},
		{0xF6, {bind_fn(INC), bind_fn(DPX), [=]() -> byte {return 6 + DLNONZERO + (2 * MZERO);}}},
		{0xE8, {bind_fn(INX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xC8, {bind_fn(INY), bind_fn(IMP), []() -> byte {return 2;}}},
		// jmp, jml
		{0x4C, {bind_fn(JMP), bind_fn(ABS_JMP_JSR), []() -> byte {return 3;}}},
		{0x6C, {bind_fn(JMP), bind_fn(ABSI), []() -> byte {return 5;}}},
		{0x7C, {bind_fn(JMP), bind_fn(ABSIX), []() -> byte {return 6;}}},
		{0x5C, {bind_fn(JML), bind_fn(ABSL_JML_JSL), []() -> byte {return 4;}}},
		{0xDC, {bind_fn(JML), bind_fn(ABSIL), []() -> byte {return 6;}}},
		// jsr, jsl
		{0x20, {bind_fn(JSR), bind_fn(ABS_JMP_JSR), []() -> byte {return 6;}}},
		{0xFC, {bind_fn(JSR), bind_fn(ABSIX), []() -> byte {return 8;}}},
		{0x22, {bind_fn(JSL), bind_fn(ABSL_JML_JSL), []() -> byte {return 8;}}},
		// lda
		{0xA9, {bind_fn(LDA), bind_fn(IMM_M), [=]() -> byte {return 2 + MZERO;}}},
		{0xAD, {bind_fn(LDA), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0xAF, {bind_fn(LDA), bind_fn(ABSL), [=]() -> byte {return 5 + MZERO;}}},
		{0xA5, {bind_fn(LDA), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0xB2, {bind_fn(LDA), bind_fn(DPI), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0xA7, {bind_fn(LDA), bind_fn(DPIL), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xBD, {bind_fn(LDA), bind_fn(ABSX), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0xBF, {bind_fn(LDA), bind_fn(ABSLX), [=]() -> byte {return 5 + MZERO;}}},
		{0xB9, {bind_fn(LDA), bind_fn(ABSY), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0xB5, {bind_fn(LDA), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0xA1, {bind_fn(LDA), bind_fn(DPIX), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xB1, {bind_fn(LDA), bind_fn(DPINY), [=]() -> byte {return 5 + MZERO + DLNONZERO + iBoundary;}}},
		{0xB7, {bind_fn(LDA), bind_fn(DPILNY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0xA3, {bind_fn(LDA), bind_fn(SR), [=]() -> byte {return 4 + MZERO;}}},
		{0xB3, {bind_fn(LDA), bind_fn(SRIY), [=]() -> byte {return 7 + MZERO;}}},
		// ldx
		{0xA2, {bind_fn(LDX), bind_fn(IMM_X), [=]() -> byte {return 2 + XZERO;}}},
		{0xAE, {bind_fn(LDX), bind_fn(ABS), [=]() -> byte {return 4 + XZERO;}}},
		{0xA6, {bind_fn(LDX), bind_fn(DP), [=]() -> byte {return 3 + XZERO + DLNONZERO;}}},
		{0xBE, {bind_fn(LDX), bind_fn(ABSY), [=]() -> byte {return 4 + XZERO + iBoundary;}}},
		{0xB6, {bind_fn(LDX), bind_fn(DPY), [=]() -> byte {return 4 + XZERO + DLNONZERO;}}},
		// ldy
		{0xA0, {bind_fn(LDY), bind_fn(IMM_X), [=]() -> byte {return 2 + XZERO;}}},
		{0xAC, {bind_fn(LDY), bind_fn(ABS), [=]() -> byte {return 4 + XZERO;}}},
		{0xA4, {bind_fn(LDY), bind_fn(DP), [=]() -> byte {return 3 + XZERO + DLNONZERO;}}},
		{0xBC, {bind_fn(LDY), bind_fn(ABSX), [=]() -> byte {return 4 + XZERO + DLNONZERO;}}},
		{0xB4, {bind_fn(LDY), bind_fn(DPX), [=]() -> byte {return 4 + XZERO + DLNONZERO;}}},
		// mvn, mvp
		{0x54, {bind_fn(MVN), bind_fn(IMM16), []() -> byte {return 7;}}},
		{0x44, {bind_fn(MVP), bind_fn(IMM16), []() -> byte {return 7;}}},
		// nop
		{0xEA, {bind_fn(NOP), bind_fn(IMP), []() -> byte {return 2;}}},
		// ora
		{0x09, {bind_fn(ORA), bind_fn(IMM_M), [=]() -> byte {return 2 + MZERO;}}},
		{0x0D, {bind_fn(ORA), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x0F, {bind_fn(ORA), bind_fn(ABSL), [=]() -> byte {return 5 + MZERO;}}},
		{0x05, {bind_fn(ORA), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x12, {bind_fn(ORA), bind_fn(DPI), [=]() -> byte {return 5 + MZERO + DLNONZERO;}}},
		{0x07, {bind_fn(ORA), bind_fn(DPIL), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x1D, {bind_fn(ORA), bind_fn(ABSX), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0x1F, {bind_fn(ORA), bind_fn(ABSLX), [=]() -> byte {return 5 + MZERO;}}},
		{0x19, {bind_fn(ORA), bind_fn(ABSY), [=]() -> byte {return 4 + MZERO + iBoundary;}}},
		{0x15, {bind_fn(ORA), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		{0x01, {bind_fn(ORA), bind_fn(DPIX), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x11, {bind_fn(ORA), bind_fn(DPINY), [=]() -> byte {return 5 + MZERO + DLNONZERO + iBoundary;}}},
		{0x17, {bind_fn(ORA), bind_fn(DPILNY), [=]() -> byte {return 6 + MZERO + DLNONZERO;}}},
		{0x03, {bind_fn(ORA), bind_fn(SR), [=]() -> byte {return 4 + MZERO;}}},
		{0x13, {bind_fn(ORA), bind_fn(SRIY), [=]() -> byte {return 7 + MZERO;}}},
		// pea, pei, per
		{0xF4, {bind_fn(PEA), bind_fn(IMM16), []() -> byte {return 5;}}},
		{0xD4, {bind_fn(PEI), bind_fn(DP16), [=]() -> byte {return 6 + DLNONZERO;}}},
		{0x62, {bind_fn(PER), bind_fn(IMM16), []() -> byte {return 6;}}},
		// push to stack
		{0x48, {bind_fn(PHA), bind_fn(IMP), [=]() -> byte {return 3 + MZERO;}}},
		{0x8B, {bind_fn(PHB), bind_fn(IMP), []() -> byte {return 3;}}},
		{0x0B, {bind_fn(PHD), bind_fn(IMP), []() -> byte {return 4;}}},
		{0x4B, {bind_fn(PHK), bind_fn(IMP), []() -> byte {return 3;}}},
		{0x08, {bind_fn(PHP), bind_fn(IMP), []() -> byte {return 3;}}},
		{0xDA, {bind_fn(PHX), bind_fn(IMP), [=]() -> byte {return 3 + XZERO;}}},
		{0x5A, {bind_fn(PHY), bind_fn(IMP), [=]() -> byte {return 3 + XZERO;}}},
		// pull from stack
		{0x68, {bind_fn(PLA), bind_fn(IMP), [=]() -> byte {return 4 + MZERO;}}},
		{0xAB, {bind_fn(PLB), bind_fn(IMP), []() -> byte {return 4;}}},
		{0x2B, {bind_fn(PLD), bind_fn(IMP), []() -> byte {return 5;}}},
		{0x28, {bind_fn(PLP), bind_fn(IMP), []() -> byte {return 4;}}},
		{0xFA, {bind_fn(PLX), bind_fn(IMP), [=]() -> byte {return 4 + XZERO;}}},
		{0x7A, {bind_fn(PLY), bind_fn(IMP), [=]() -> byte {return 4 + XZERO;}}},
		// rep
		{0xC2, {bind_fn(REP), bind_fn(IMM8), []() -> byte {return 3;}}},
		// rol, ror

		// rti, rts, rtl
		{0x40, {bind_fn(RTI), bind_fn(IMP), [=]() -> byte {return 6 + EZERO;}}},
		{0x60, {bind_fn(RTS), bind_fn(IMP), []() -> byte {return 6;}}},
		{0x6B, {bind_fn(RTL), bind_fn(IMP), []() -> byte {return 6;}}},
		// sbc
		{0xE9, {bind_fn(SBC), bind_fn(IMM_M), [=]() -> byte {return 2 + MZERO;}}},
		{0xED, {bind_fn(SBC), bind_fn(ABS), [=]() -> byte {return 2 + MZERO;}}},
		{0xEF, {bind_fn(SBC), bind_fn(ABSL), [=]() -> byte {return 2 + MZERO;}}},
		{0xE5, {bind_fn(SBC), bind_fn(DP), [=]() -> byte {return 2 + MZERO;}}},
		{0xF2, {bind_fn(SBC), bind_fn(DPI), [=]() -> byte {return 2 + MZERO;}}},
		{0xE7, {bind_fn(SBC), bind_fn(DPIL), [=]() -> byte {return 2 + MZERO;}}},
		{0xFD, {bind_fn(SBC), bind_fn(ABSX), [=]() -> byte {return 2 + MZERO;}}},
		{0xFF, {bind_fn(SBC), bind_fn(ABSLX), [=]() -> byte {return 2 + MZERO;}}},
		{0xF9, {bind_fn(SBC), bind_fn(ABSY), [=]() -> byte {return 2 + MZERO;}}},
		{0xF5, {bind_fn(SBC), bind_fn(DPX), [=]() -> byte {return 2 + MZERO;}}},
		{0xE1, {bind_fn(SBC), bind_fn(DPIX), [=]() -> byte {return 2 + MZERO;}}},
		{0xF1, {bind_fn(SBC), bind_fn(DPINY), [=]() -> byte {return 2 + MZERO;}}},
		{0xF7, {bind_fn(SBC), bind_fn(DPILNY), [=]() -> byte {return 2 + MZERO;}}},
		{0xE3, {bind_fn(SBC), bind_fn(SR), [=]() -> byte {return 2 + MZERO;}}},
		{0xF3, {bind_fn(SBC), bind_fn(SRIY), [=]() -> byte {return 2 + MZERO;}}},
		// sec, sed, sei
		{0x38, {bind_fn(SEC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x78, {bind_fn(SEI), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xF8, {bind_fn(SED), bind_fn(IMP), []() -> byte {return 2;}}},
		// sep
		{0xE2, {bind_fn(SEP), bind_fn(IMM8), []() -> byte {return 3;}}},
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
		// stx
		{0x8E, {bind_fn(STX), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x86, {bind_fn(STX), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x96, {bind_fn(STX), bind_fn(DPY), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		// sty
		{0x8C, {bind_fn(STY), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x84, {bind_fn(STY), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x94, {bind_fn(STY), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		// stz
		{0x9C, {bind_fn(STZ), bind_fn(ABS), [=]() -> byte {return 4 + MZERO;}}},
		{0x64, {bind_fn(STZ), bind_fn(DP), [=]() -> byte {return 3 + MZERO + DLNONZERO;}}},
		{0x9E, {bind_fn(STZ), bind_fn(ABSX), [=]() -> byte {return 5 + MZERO;}}},
		{0x74, {bind_fn(STZ), bind_fn(DPX), [=]() -> byte {return 4 + MZERO + DLNONZERO;}}},
		// transfer registers
		{0xAA, {bind_fn(TAX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xA8, {bind_fn(TAY), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x5B, {bind_fn(TCD), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x1B, {bind_fn(TCS), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x7B, {bind_fn(TDC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x3B, {bind_fn(TSC), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xBA, {bind_fn(TSX), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x8A, {bind_fn(TXA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x9A, {bind_fn(TXS), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x9B, {bind_fn(TXY), bind_fn(IMP), []() -> byte {return 2;}}},
		{0x98, {bind_fn(TYA), bind_fn(IMP), []() -> byte {return 2;}}},
		{0xBB, {bind_fn(TYX), bind_fn(IMP), []() -> byte {return 2;}}},
		// trb, tsb
		{0x1C, {bind_fn(TRB), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0x14, {bind_fn(TRB), bind_fn(DP), [=]() -> byte {return 5 + (2 * MZERO) + DLNONZERO;}}},
		{0x0C, {bind_fn(TSB), bind_fn(ABS), [=]() -> byte {return 6 + (2 * MZERO);}}},
		{0x04, {bind_fn(TSB), bind_fn(DP), [=]() -> byte {return 5 + (2 * MZERO) + DLNONZERO;}}},
		// xba, xce
		{0xEB, {bind_fn(XBA), bind_fn(IMP), []() -> byte {return 3;}}},
		{0xFB, {bind_fn(XCE), bind_fn(IMP), []() -> byte {return 2;}}}
	};
};

#endif