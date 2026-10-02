#include "common.h"

#include "cpu.hpp"
#include "cpu_bus.hpp"
#include "ram.hpp"
#include "cpu_apu_io.hpp"

#include <stdio.h>
#include <iostream>
#include <iomanip>

SNES_CPU::SNES_CPU(CPU_BUS* bus): bus(bus) {};

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

void SNES_CPU::setP(byte value) {
	status.full = value;

	if (e) {
		status.bits.m = 1;
		status.bits.x = 1;
	}

	if (status.bits.x) {
		*XH = 0x00;
		*YH = 0x00;
	}
}

byte SNES_CPU::getP() {
	return status.full;
}

void SNES_CPU::setE(bool value) {
	e = value;
	if (e) {
		status.bits.m = 1;
		status.bits.x = 1;

		*XH = 0x00;
		*YH = 0x00;

		*SH = 0x01;
	}
}

void SNES_CPU::init() {
	setP(0x34);
	setE(1);
	D = 0x0000;
	DBR = 0x00;
	PC = read16_bank0(0xFFFC);
}

byte SNES_CPU::executeNextCommand() {
	// get opcode
	byte opcode = fetch8();

	iBoundary = false;
	branchTaken = false;
	branchBoundary = false;
	ea = 0x000000; // effective address
	immediate = false; // immediate addr mode
	ea_wrap_bank0 = false; // wrap operand reads/writes to bank 0
	
	// fetch data based on addressing mode
	(this->ops[opcode]).mode();
	// execute op
	(this->ops[opcode]).op();
	
	byte cycles = (this->ops[opcode].cycleCount)();
	
#ifdef DEBUG
	std::cout << "-- executed opcode 0x" << std::hex << (unsigned int)opcode << std::dec << " (" << (this->ops[opcode]).name << ")" << std::endl;
	debugPrint();
#endif

	return cycles;
}

bool SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		cyclesRemaining = executeNextCommand();
	}
	cyclesRemaining--;
	return true;
}

void SNES_CPU::debugPrint() {
	std::cout << "status flags: " << std::endl;
	std::cout << "n v m x d i z c (e)" << std::endl;
	for(int i = 7; i >= 0; i--)
		std::cout << getBit(status.full, i) << " ";
	std::cout << " " << e << std::endl;
	std::cout << "accumulator: " << C << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(C, i);
	std::cout << std::endl;
	std::cout << "ea: " << ea << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(ea, i);
	std::cout << std::endl;
	std::cout << "K: " << std::hex << HEX_BYTE_PRINT(K) << std::dec << std::endl;
	std::cout << "PC: " << std::hex << std::setw(4) << PC << std::dec << std::endl;
	std::cout << "D: " << std::hex << std::setw(4) << D << std::dec << std::endl;
	std::cout << std::endl;
}

void SNES_CPU::decS() {
	if (e) (*SL)--;
	else S--;
}
void SNES_CPU::incS() {
	if (e) (*SL)++;
	else S++;
}

void SNES_CPU::push_stack_threebyte(threebyte value) {
	write8(0x00, S, value >> 16);
	decS();
	write8(0x00, S, (value >> 8) & 0xFF);
	decS();
	write8(0x00, S, value & 0xFF);
	decS();
}

void SNES_CPU::push_stack_twobyte(twobyte value) {
	write8(0x00, S, value >> 8);
	decS();
	write8(0x00, S, value & 0xFF);
	decS();
}

void SNES_CPU::push_stack_byte(byte value) {
	write8(0x00, S, value);
	decS();
}

threebyte SNES_CPU::pop_stack_threebyte() {
	byte lo = pop_stack_byte();
	byte md = pop_stack_byte();
	byte hi = pop_stack_byte();
	return (hi << 16) | (md << 8) | lo;
}

twobyte SNES_CPU::pop_stack_twobyte() {
	byte lo = pop_stack_byte();
	byte hi = pop_stack_byte();
	return (hi << 8) | lo;
}

byte SNES_CPU::pop_stack_byte() {
	incS();
	return read8(S);
}

//
// operations
//

void SNES_CPU::ADC() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;              // replaces *fetched_lo
	byte data_hi = (byte)(data >> 8);       // replaces *fetched_hi
	
	if(status.bits.d) {
		byte lower_nybble_sum = (*A & 0x0F) + (data_lo & 0x0F) + status.bits.c;
		byte upper_nybble_sum = (*A >> 4) + (data_lo >> 4);

		if(lower_nybble_sum > 0x09) {
			lower_nybble_sum += 0x06;
			lower_nybble_sum &= 0x0F;
			upper_nybble_sum++;
		}

		if(status.bits.m) {
			if(upper_nybble_sum > 0x09) {
				upper_nybble_sum += 0x06;
				upper_nybble_sum &= 0x0F;
				status.bits.c = 1;
			}

			*A = (upper_nybble_sum << 4) | lower_nybble_sum;
			
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
			return;
		} else {
			byte lower_nybble_sum_hi = (*B & 0x0F) + (data_hi & 0x0F);
			byte upper_nybble_sum_hi = (*B >> 4) + (data_hi >> 4);

			if(upper_nybble_sum > 0x09) {
				upper_nybble_sum += 0x06;
				upper_nybble_sum &= 0x0F;
				lower_nybble_sum_hi++;
			}

			if(lower_nybble_sum_hi > 0x09) {
				lower_nybble_sum_hi += 0x06;
				lower_nybble_sum_hi &= 0x0F;
				upper_nybble_sum_hi++;
			}

			if(upper_nybble_sum_hi > 0x09) {
				upper_nybble_sum_hi += 0x06;
				upper_nybble_sum_hi &= 0x0F;
				status.bits.c = 1;
			}

			C =  (twobyte)(upper_nybble_sum_hi << 12) | (lower_nybble_sum_hi << 8) | (upper_nybble_sum << 4) | lower_nybble_sum;
		
			status.bits.n = getBit(C, 15);
			status.bits.z = (C == 0x0000);
			return;
		}
	} else {
		if(status.bits.m) {
			bool final_c = ((twobyte)*A + (data_lo + status.bits.c) > (twobyte)0xFF);
			byte a_before = *A;
			
			*A += data_lo;
			*A += status.bits.c;
			
			status.bits.v = getBit(~(a_before ^ data_lo) & (a_before ^ *A), 7);
			status.bits.c = final_c;
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
		} else {
			bool final_c = ((threebyte)C + (data + status.bits.c) > (threebyte)0xFFFF);
			twobyte c_before = C;
			
			C += data;
			C += status.bits.c;
			
			status.bits.v = getBit(~(c_before ^ data) & (c_before ^ C), 15);
			status.bits.c = final_c;
			status.bits.n = getBit(C, 15);
			status.bits.z = (C == 0x0000);
		}
	}
}

void SNES_CPU::AND() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		*A &= data_lo;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C &= data;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::ASL() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		byte data = data_lo;
		
		status.bits.c = getBit(data, 7);
		data <<= 1;
		
		writeEA8(data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		status.bits.c = getBit(data, 15);
		data <<= 1;
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::ASLA() {
	if(status.bits.m) {
		status.bits.c = getBit(*A, 7);
		*A <<= 1;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		status.bits.c = getBit(C, 15);
		C <<= 1;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::LSR() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		byte data = data_lo;
		
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		writeEA8(data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::LSRA() {
	if(status.bits.m) {
		status.bits.c = getBit(*A, 0);
		*A >>= 1;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		status.bits.c = getBit(C, 0);
		C >>= 1;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::BCC() {
	byte data = readEA8();
	
	if(!status.bits.c){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BCS() {
	byte data = readEA8();

	if(status.bits.c){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BEQ() {
	byte data = readEA8();

	if(status.bits.z){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BIT() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		byte a_val = *A;
		a_val &= data_lo;
		
		status.bits.n = getBit(data_lo, 7);
		status.bits.v = getBit(data_lo, 6);
		status.bits.z = (a_val == 0x00);
	} else {
		twobyte c_val = C;
		c_val &= data;
		
		status.bits.n = getBit(data, 15);
		status.bits.v = getBit(data, 14);
		status.bits.z = (c_val == 0x0000);
	}
}

void SNES_CPU::BITIMM() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		byte a_val = *A;
		a_val &= data_lo;
		
		status.bits.z = (a_val == 0x00);
	} else {
		twobyte c_val = C;
		c_val &= data;
		
		status.bits.z = (c_val == 0x0000);
	}
}

void SNES_CPU::BMI() {
	byte data = readEA8();

	if(status.bits.n){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BNE() {
	byte data = readEA8();

	if(!status.bits.z){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BPL() {
	byte data = readEA8();

	if(!status.bits.n){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BRA() {
	byte data = readEA8();
	PC += (signedbyte)data;
	branchTaken = true;
}

void SNES_CPU::BRL() {
	twobyte data = readEA16();
	PC += (signedtwobyte)data;
	branchTaken = true;
}

void SNES_CPU::BVC() {
	byte data = readEA8();

	if(!status.bits.v){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BVS() {
	byte data = readEA8();

	if(status.bits.v){
		PC += (signedbyte)data;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BRK() {
	readEA8(); // advance PC

	if (e) {
		push_stack_twobyte(PC);
		push_stack_byte(getP());

		status.bits.d = 0;
		status.bits.i = 1;

		K = 0x00;
		PC = irq_vector();
	} else {
		push_stack_byte(K);
		push_stack_twobyte(PC);
		push_stack_byte(getP());

		status.bits.d = 0;
		status.bits.i = 1;

		K = 0x00;
		PC = brk_vector();
	}
	
}

void SNES_CPU::COP() {
	readEA8(); // advance PC

	push_stack_byte(K);
	push_stack_twobyte(PC);
	push_stack_byte(getP());

	status.bits.d = 0;
	status.bits.i = 1;

	K = 0x00;
	PC = cop_vector();
}

void SNES_CPU::CLC() {
	status.bits.c = 0;
}

void SNES_CPU::CLD() {
	status.bits.d = 0;
}

void SNES_CPU::CLI() {
	status.bits.i = 0;
}

void SNES_CPU::CLV() {
	status.bits.v = 0;
}

void SNES_CPU::CMP() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		byte A_copy = *A;
		
		status.bits.c = (A_copy >= data_lo);
		
		A_copy -= data_lo;
		
		status.bits.n = getBit(A_copy, 7);
		status.bits.z = (A_copy == 0x00);
	} else {
		twobyte C_copy = C;
		
		status.bits.c = (C_copy >= data);
		
		C_copy -= data;
		
		status.bits.n = getBit(C_copy, 15);
		status.bits.z = (C_copy == 0x0000);
	}
}

void SNES_CPU::CPX() {
	twobyte data = status.bits.x ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.x) {
		byte X_copy = *XL;
		
		status.bits.c = (X_copy >= data_lo);
		
		X_copy -= data_lo;
		
		status.bits.n = getBit(X_copy, 7);
		status.bits.z = (X_copy == 0x00);
	} else {
		twobyte X_copy = X;
		
		status.bits.c = (X_copy >= data);
		
		X_copy -= data;
		
		status.bits.n = getBit(X_copy, 15);
		status.bits.z = (X_copy == 0x0000);
	}
}

void SNES_CPU::CPY() {
	twobyte data = status.bits.x ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.x) {
		byte Y_copy = *YL;
		
		status.bits.c = (Y_copy >= data_lo);
		
		Y_copy -= data_lo;
		
		status.bits.n = getBit(Y_copy, 7);
		status.bits.z = (Y_copy == 0x00);
	} else {
		twobyte Y_copy = Y;
		
		status.bits.c = (Y_copy >= data);
		
		Y_copy -= data;
		
		status.bits.n = getBit(Y_copy, 15);
		status.bits.z = (Y_copy == 0x0000);
	}
}

void SNES_CPU::DEC() {
	if(status.bits.m) {
		byte data = readEA8();
		
		data--;
		
		writeEA8(data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = readEA16();
		
		data--;
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::DECA() {
	if(status.bits.m) {
		(*A)--;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C--;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::DEX() {
	if(status.bits.x) {
		(*XL)--;
		
		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	} else {
		X--;
		
		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	}
}

void SNES_CPU::DEY() {
	if(status.bits.x) {
		(*YL)--;
		
		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	} else {
		Y--;
		
		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	}
}

void SNES_CPU::EOR() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		*A ^= data_lo;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C ^= data;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::INC() {
	if(status.bits.m) {
		byte data = readEA8();
		
		data++;
		
		writeEA8(data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = readEA16();
		
		data++;
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::INCA() {
	if(status.bits.m) {
		(*A)++;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C++;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::INX() {
	if(status.bits.x) {
		(*XL)++;
		
		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	} else {
		X++;
		
		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	}
}

void SNES_CPU::INY() {
	if(status.bits.x) {
		(*YL)++;
		
		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	} else {
		Y++;
		
		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	}
}

void SNES_CPU::JMP() {
	PC = (twobyte)ea;
}

void SNES_CPU::JML() {
	K = ea >> 16;
	PC = (twobyte)ea;
}

void SNES_CPU::JSR() {
	PC--;
	push_stack_twobyte(PC);
	PC = (twobyte)ea;
}

void SNES_CPU::JSL() {
	PC--;
	push_stack_byte(K);
	push_stack_twobyte(PC);
	K = ea >> 16;
	PC = (twobyte)ea;
}

void SNES_CPU::LDA() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		*A = data_lo;

		status.bits.n = getBit(data_lo, 7);
		status.bits.z = (data_lo == 0x00);
	} else {
		C = data;

		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::LDX() {
	twobyte data = status.bits.x ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.x) {
		*XL = data_lo;

		status.bits.n = getBit(data_lo, 7);
		status.bits.z = (data_lo == 0x00);
	} else {
		X = data;

		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::LDY() {
	twobyte data = status.bits.x ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.x) {
		*YL = data_lo;

		status.bits.n = getBit(data_lo, 7);
		status.bits.z = (data_lo == 0x00);
	} else {
		Y = data;

		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::MVN() {
	twobyte data = readEA16();
	byte dest = (byte)data;
	byte src = data >> 8;

	DBR = dest;
	write8(dest, Y, read8(src, X));
	if (status.bits.x) { (*XL)++; (*YL)++; }
	else               { X++; Y++; }
	C--;

	if (C != 0xFFFF) PC -= 3;
}

void SNES_CPU::MVP() {
	twobyte data = readEA16();
	byte dest = (byte)data;
	byte src = data >> 8;

	DBR = dest;
	write8(dest, Y, read8(src, X));
	if (status.bits.x) { (*XL)--; (*YL)--; }
	else               { X--; Y--; }
	C--;

	if (C != 0xFFFF) PC -= 3;
}

void SNES_CPU::ORA() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		*A |= data_lo;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C |= data;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::PEA() {
	push_stack_twobyte(readEA16());
}

void SNES_CPU::PEI() {
	push_stack_twobyte(readEA16());
}

void SNES_CPU::PER() {
	twobyte offset = readEA16();
	push_stack_twobyte(PC + offset);
}

// push

void SNES_CPU::PHA() {
	if(status.bits.m) {
		push_stack_byte(*A);
	} else {
		push_stack_twobyte(C);
	}
}

void SNES_CPU::PHB() {
	push_stack_byte(DBR);
}

void SNES_CPU::PHD() {
	push_stack_twobyte(D);
}

void SNES_CPU::PHK() {
	push_stack_byte(K);
}

void SNES_CPU::PHP() {
	push_stack_byte(getP());
}

void SNES_CPU::PHX() {
	if(status.bits.x) {
		push_stack_byte(*XL);
	} else {
		push_stack_twobyte(X);
	}
}

void SNES_CPU::PHY() {
	if(status.bits.x) {
		push_stack_byte(*YL);
	} else {
		push_stack_twobyte(Y);
	}
}

// pull

void SNES_CPU::PLA() {
	if(status.bits.m) {
		*A = pop_stack_byte();

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C = pop_stack_twobyte();

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::PLB() {
	DBR = pop_stack_byte();

	status.bits.n = getBit(DBR, 7);
	status.bits.z = (DBR == 0x00);
}

void SNES_CPU::PLD() {
	D = pop_stack_twobyte();

	status.bits.n = getBit(D, 15);
	status.bits.z = (D == 0x0000);
}

void SNES_CPU::PLP() {
	setP(pop_stack_byte());
}

void SNES_CPU::PLX() {
	if(status.bits.x) {
		*XL = pop_stack_byte();

		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	} else {
		X = pop_stack_twobyte();

		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	}
}

void SNES_CPU::PLY() {
	if(status.bits.x) {
		*YL = pop_stack_byte();

		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	} else {
		Y = pop_stack_twobyte();

		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	}
}

void SNES_CPU::REP() {
	byte data = readEA8();
	setP(getP() & ~data);
}

void SNES_CPU::ROL() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	bool c_pre_shift = status.bits.c;
	if(status.bits.m) {
		status.bits.c = getBit(data_lo, 7);
		data_lo <<= 1;
		data_lo |= c_pre_shift;
		
		writeEA8(data_lo);
		
		status.bits.n = getBit(data_lo, 7);
		status.bits.z = (data_lo == 0x00);
	} else {
		status.bits.c = getBit(data, 15);
		data <<= 1;
		data |= c_pre_shift;
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::ROLA() {
	bool c_pre_shift = status.bits.c;
	if(status.bits.m) {
		status.bits.c = getBit(*A, 7);
		*A <<= 1;
		*A |= c_pre_shift;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		status.bits.c = getBit(C, 15);
		C <<= 1;
		C |= c_pre_shift;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::ROR() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	bool c_pre_shift = status.bits.c;
	status.bits.c = getBit(data, 0);
	if(status.bits.m) {
		data_lo >>= 1;
		data_lo |= (c_pre_shift << 7);
		
		writeEA8(data_lo);
		
		status.bits.n = getBit(data_lo, 7);
		status.bits.z = (data_lo == 0x00);
	} else {
		data >>= 1;
		data |= (c_pre_shift << 15);
		
		writeEA16(data);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::RORA() {
	bool c_pre_shift = status.bits.c;
	status.bits.c = getBit(C, 0);
	if(status.bits.m) {
		*A >>= 1;
		*A |= (c_pre_shift << 7);
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C >>= 1;
		C |= (c_pre_shift << 15);
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::RTI() {
	setP(pop_stack_byte());
	PC = pop_stack_twobyte();

	if(!e) {
		K = pop_stack_byte();
	}
}

void SNES_CPU::RTS() {
	PC = pop_stack_twobyte();
	PC++;
}

void SNES_CPU::RTL() {
	PC = pop_stack_twobyte();
	PC++;

	K = pop_stack_byte();
}

void SNES_CPU::SBC() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;
	byte data_hi = data >> 8;

	if(status.bits.d) {
		byte lower_nybble_diff = (*A & 0x0F) - (data_lo & 0x0F) - (status.bits.c ? 0 : 1);
		byte upper_nybble_diff = (*A >> 4) - (data_lo >> 4);

		if(lower_nybble_diff > 0x09) {
			lower_nybble_diff -= 0x06;
			lower_nybble_diff &= 0x0F;
			upper_nybble_diff--;
		}

		if(status.bits.m) {
			if(upper_nybble_diff > 0x09) {
				upper_nybble_diff -= 0x06;
				upper_nybble_diff &= 0x0F;
				status.bits.c = 0;
			}

			*A = (upper_nybble_diff << 4) | lower_nybble_diff;
			
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
			return;
		} else {
			byte lower_nybble_diff_hi = (*B & 0x0F) - (data_hi & 0x0F);
			byte upper_nybble_diff_hi = (*B >> 4) - (data_hi >> 4);

			if(upper_nybble_diff > 0x09) {
				upper_nybble_diff -= 0x06;
				upper_nybble_diff &= 0x0F;
				lower_nybble_diff_hi--;
			}

			if(lower_nybble_diff_hi > 0x09) {
				lower_nybble_diff_hi -= 0x06;
				lower_nybble_diff_hi &= 0x0F;
				upper_nybble_diff_hi--;
			}

			if(upper_nybble_diff_hi > 0x09) {
				upper_nybble_diff_hi -= 0x06;
				upper_nybble_diff_hi &= 0x0F;
				status.bits.c = 0;
			}

			C =  (twobyte)(upper_nybble_diff_hi << 12) | (lower_nybble_diff_hi << 8) | (upper_nybble_diff << 4) | lower_nybble_diff;
		
			status.bits.n = getBit(C, 15);
			status.bits.z = (C = 0x0000);
			return;
		}
	} else {
		if(status.bits.m) {
			bool final_c = (*A >= data_lo);
			bool high_bit_pre_sbc = getBit(*A, 7);
			
			*A -= data_lo;
			*A -= (status.bits.c ? 0 : 1);
			
			status.bits.v = ((high_bit_pre_sbc != getBit(data_lo + (status.bits.c ? 0 : 1), 7))
							&& (high_bit_pre_sbc != getBit(*A, 7)));
			status.bits.c = final_c;
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
		} else {
			bool final_c = (C >= data);
			bool high_bit_pre_sbc = getBit(C, 15);
			
			C -= data;
			C -= (status.bits.c ? 0 : 1);
			
			status.bits.v = ((high_bit_pre_sbc != getBit(data + (status.bits.c ? 0 : 1), 15))
							&& (high_bit_pre_sbc != getBit(C, 15)));
			status.bits.c = final_c;
			status.bits.n = getBit(C, 15);
			status.bits.z = (C == 0x0000);
		}
	}
}

void SNES_CPU::SEC() {
	status.bits.c = 1;
}

void SNES_CPU::SEI() {
	status.bits.i = 1;
}

void SNES_CPU::SED() {
	status.bits.d = 1;
}

void SNES_CPU::SEP() {
	byte data = readEA8();
	setP(getP() | data);
}

void SNES_CPU::STA() {
	if(status.bits.m) {
		writeEA8(*A);
	} else {
		writeEA16(C);
	}
}

void SNES_CPU::STX() {
	if(status.bits.x) {
		writeEA8(*XL);
	} else {
		writeEA16(X);
	}
}

void SNES_CPU::STY() {
	if(status.bits.x) {
		writeEA8(*YL);
	} else {
		writeEA16(Y);
	}
}

void SNES_CPU::STZ() {
	if(status.bits.m) {
		writeEA8(0);
	} else {
		writeEA16(0);
	}
}

void SNES_CPU::TAX() {
	if(status.bits.x) {
		*XL = *A;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		X = C;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::TAY() {
	if(status.bits.x) {
		*YL = *A;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		Y = C;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::TCD() {
	D = C;

	status.bits.n = getBit(C, 15);
	status.bits.z = (C == 0x0000);
}

void SNES_CPU::TCS() {
	S = C;

	if (e) {
		*SH = 0x01;
	}
}

void SNES_CPU::TDC() {
	C = D;

	status.bits.n = getBit(D, 15);
	status.bits.z = (D == 0x0000);
}

void SNES_CPU::TSC() {
	C = S;

	status.bits.n = getBit(S, 15);
	status.bits.z = (S == 0x0000);
}

void SNES_CPU::TSX() {
	if(status.bits.x) {
		*XL = *SL;

		status.bits.n = getBit(*SL, 7);
		status.bits.z = (*SL == 0x00);
	} else {
		X = S;

		status.bits.n = getBit(S, 15);
		status.bits.z = (S == 0x0000);
	}
}

void SNES_CPU::TXA() {
	if(status.bits.m) {
		*A = *XL;

		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	} else {
		C = X;

		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	}
}

void SNES_CPU::TXS() {
	S = X;

	if (e) {
		*SH = 0x01;
	}
}

void SNES_CPU::TXY() {
	if(status.bits.x) {
		*YL = *XL;

		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	} else {
		Y = X;

		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	}
}

void SNES_CPU::TYA() {
	if(status.bits.m) {
		*A = *YL;

		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	} else {
		C = Y;

		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	}
}

void SNES_CPU::TYX() {
	if(status.bits.x) {
		*XL = *YL;

		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	} else {
		X = Y;

		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	}
}

void SNES_CPU::TRB() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		status.bits.z = ((*A & data_lo) == 0x00);

		data_lo &= ~*A;

		writeEA8(data_lo);
	} else {
		status.bits.z = ((C & data) == 0x0000);

		data &= ~C;

		writeEA16(data);
	}
}

void SNES_CPU::TSB() {
	twobyte data = status.bits.m ? readEA8() : readEA16();
	byte data_lo = (byte)data;

	if(status.bits.m) {
		status.bits.z = ((*A & data_lo) == 0x00);

		data_lo |= *A;

		writeEA8(data_lo);
	} else {
		status.bits.z = ((C & data) == 0x0000);

		data |= C;

		writeEA16(data);
	}
}

void SNES_CPU::WAI() {
	std::cout << "called WAI" << std::endl;
}

void SNES_CPU::XBA() {
	byte B_pre_swap = *B;
	
	*B = *A;
	*A = B_pre_swap;

	status.bits.n = getBit(B_pre_swap, 7);
	status.bits.z = (B_pre_swap == 0x00);
}

void SNES_CPU::XCE() {
	bool c = status.bits.c;
	status.bits.c = e;
	setE(c);
}

//
// addressing modes
//

// immediate

void SNES_CPU::IMM() {
	immediate = true;
}

// direct page

void SNES_CPU::DP() {
	ea = dpAddr(fetch8());
	ea_wrap_bank0 = true;
}

// Direct Page Indexed, X
void SNES_CPU::DPX() {
	ea = dpAddr(fetch8() + (status.bits.x ? *XL : X));
	ea_wrap_bank0 = true;
}

// Direct Page Indexed, Y
void SNES_CPU::DPY() {
	ea = dpAddr(fetch8() + (status.bits.x ? *YL : Y));
	ea_wrap_bank0 = true;
}

// indirect

// Direct Page Indirect
void SNES_CPU::DPI() {
	byte ll = fetch8();
	byte lo = read8(dpAddr(ll));
	byte hi = read8(dpAddr(ll + 1));
	ea = (DBR << 16) | (hi << 8) | lo;
}

// Direct Page Indirect Long
void SNES_CPU::DPIL() {
	byte ll = fetch8();
	byte lo = read8(dpAddr(ll));
	byte md = read8(dpAddr(ll + 1));
	byte hi = read8(dpAddr(ll + 2));
	ea = (hi << 16) | (md << 8) | lo;
}

// Direct Page Indirect, X
void SNES_CPU::DPIX() {
	byte ll = fetch8();
	byte lo = read8(dpAddr(ll + X));
	byte hi = read8(dpAddr(ll + X + 1));
	ea = (DBR << 16) | (hi << 8) | lo;
}

// Direct Page Indirect iNdexed, Y
void SNES_CPU::DPINY() {
	byte ll = fetch8();
	byte lo = read8(dpAddr(ll));
	byte hi = read8(dpAddr(ll + 1));
	threebyte ptr = ((DBR << 16) | (hi << 8) | lo);
	ea = (ptr + Y) & 0xFFFFFF;
	iBoundary = !status.bits.x || ((ptr & 0xFF00) != ((ptr + Y) & 0xFF00));
}

// Direct Page Indirect Long iNdexed, Y
void SNES_CPU::DPILNY() {
	byte ll = fetch8();
	byte lo = read8(dpAddr(ll));
	byte md = read8(dpAddr(ll + 1));
	byte hi = read8(dpAddr(ll + 2));
	ea = (((hi << 16) | (md << 8) | lo) + Y) & 0xFFFFFF;
}

// absolute

void SNES_CPU::ABS() {
	ea = (DBR << 16) | fetch16();
}

void SNES_CPU::ABSI() {
	ea = read16_bank0(fetch16());
}

void SNES_CPU::ABSIL() {
	ea = read24_bank0(fetch16());
}

void SNES_CPU::ABSIX() {
	twobyte a = fetch16() + X;
	ea = read8(K, a) | (read8(K, (twobyte)(a + 1)) << 8);
}

void SNES_CPU::ABSL() {
	ea = fetch24();
}

void SNES_CPU::ABSX() {
	threebyte base = (DBR << 16) | fetch16();
	ea = (base + X) & 0xFFFFFF;
	iBoundary = !status.bits.x || ((base & 0xFF00) != ((base + X) & 0xFF00));
}

void SNES_CPU::ABSY() {
	threebyte base = (DBR << 16) | fetch16();
	ea = (base + Y) & 0xFFFFFF;
	iBoundary = !status.bits.x || ((base & 0xFF00) != ((base + Y) & 0xFF00));
}

void SNES_CPU::ABSLX() {
	ea = (fetch24() + X) & 0xFFFFFF;
}

// stack relative

void SNES_CPU::SR() {
	ea = (twobyte)(S + fetch8());
	ea_wrap_bank0 = true;
}

void SNES_CPU::SRIY() {
	twobyte ptr = read16_bank0(S + fetch8());
	ea = (((DBR << 16) | ptr) + Y) & 0xFFFFFF;
}

// bus

byte SNES_CPU::read8(byte bank, twobyte addr) {
	return read8(addr + (bank << 16));
}

byte SNES_CPU::read8(threebyte addr) {
	byte value = bus->read(addr & 0xFFFFFF);
#ifdef DEBUG_MEMORY
	std::cout << "read8: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	return value;
}

twobyte SNES_CPU::read16(byte bank, twobyte addr) {
	return read16(addr + (bank << 16));
}

twobyte SNES_CPU::read16(threebyte addr) {
	twobyte value = bus->read(addr & 0xFFFFFF);
	value |= bus->read((addr + 1) & 0xFFFFFF) << 8;
#ifdef DEBUG_MEMORY
	std::cout << "read16: read twobyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	return value;
}

threebyte SNES_CPU::read24(byte bank, twobyte addr) {
	return read24(addr + (bank << 16));
}

threebyte SNES_CPU::read24(threebyte addr) {
	threebyte value = bus->read(addr & 0xFFFFFF);
	value |= bus->read((addr + 1) & 0xFFFFFF) << 8;
	value |= bus->read((addr + 2) & 0xFFFFFF) << 16;
#ifdef DEBUG_MEMORY
	std::cout << "read24: read threebyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	return value;
}

twobyte SNES_CPU::read16_bank0(twobyte addr) {
	twobyte value = bus->read((twobyte)addr);
	value |= bus->read((twobyte)(addr + 1)) << 8;
#ifdef DEBUG_MEMORY
	std::cout << "read16_bank0: read twobyte $" << std::hex << value <<
	" at 0x00" << addr << std::dec << std::endl;
#endif
	return value;
}

threebyte SNES_CPU::read24_bank0(twobyte addr) {
	threebyte value = bus->read((twobyte)addr);
	value |= bus->read((twobyte)(addr + 1)) << 8;
	value |= bus->read((twobyte)(addr + 2)) << 16;
#ifdef DEBUG_MEMORY
	std::cout << "read24_bank0: read threebyte $" << std::hex << value <<
	" at 0x00" << addr << std::dec << std::endl;
#endif
	return value;
}

void SNES_CPU::write8(byte bank, twobyte addr, byte entry) {
	write8(addr + (bank << 16), entry);
}

void SNES_CPU::write8(threebyte addr, byte entry) {
	bus->write(addr, entry);
#ifdef DEBUG_MEMORY
	std::cout << "write8: wrote byte $" << std::hex << HEX_BYTE_PRINT(entry) <<
	" to 0x" << addr << std::dec << std::endl;
#endif
}

void SNES_CPU::write16(byte bank, twobyte addr, twobyte entry) {
	write16(addr + (bank << 16), entry);
}

void SNES_CPU::write16(threebyte addr, twobyte entry) {
	bus->write(addr & 0xFFFFFF, (byte)(entry & 0x00FF));
	bus->write((addr + 1) & 0xFFFFFF, (byte)((entry & 0xFF00) >> 8));
#ifdef DEBUG_MEMORY
	std::cout << "write16: wrote twobyte $" << std::hex << entry <<
	" to 0x" << std::setw(6) << addr << std::dec << std::endl;
#endif
}

void SNES_CPU::write16_bank0(twobyte addr, twobyte entry) {
	bus->write((twobyte)addr, (byte)entry);
	bus->write((twobyte)(addr + 1), (byte)(entry >> 8));
#ifdef DEBUG_MEMORY
	std::cout << "write16_bank0: wrote twobyte $" << std::hex << entry <<
	" to 0x00" << addr << std::dec << std::endl;
#endif
}

byte SNES_CPU::readEA8() {
	if (immediate) {
		return fetch8();
	} else {
		return read8(ea);
	}
}

twobyte SNES_CPU::readEA16() {
	if (immediate) {
		return fetch16();
	} else {
		return ea_wrap_bank0 ? read16_bank0((twobyte)ea) : read16(ea);
	}
}

void SNES_CPU::writeEA8(byte entry) {
	write8(ea, entry);
}

void SNES_CPU::writeEA16(twobyte entry) {
	ea_wrap_bank0 ? write16_bank0((twobyte)ea, entry) : write16(ea, entry);
}

byte SNES_CPU::fetch8() {
	threebyte addr = PC | (K << 16);
	byte value = bus->read(addr);
#ifdef DEBUG_MEMORY
	std::cout << "fetch8: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

twobyte SNES_CPU::fetch16() {
	threebyte addr = PC | (K << 16);
	twobyte value = (twobyte)bus->read(addr);
	PC++;
	addr = PC | (K << 16);
	value |= (bus->read(addr) << 8);
#ifdef DEBUG_MEMORY
	std::cout << "fetch16: read twobyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

threebyte SNES_CPU::fetch24() {
	threebyte addr = PC | (K << 16);
	threebyte value = (threebyte)bus->read(addr);
	PC++;
	addr = PC | (K << 16);
	value |= (bus->read(addr) << 8);
	PC++;
	addr = PC | (K << 16);
	value |= (bus->read(addr) << 16);
#ifdef DEBUG_MEMORY
	std::cout << "fetch24: read threebyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

twobyte SNES_CPU::dpAddr(twobyte offset) {
	if (e && *DL == 0x00) return D | (offset & 0xFF);
	return D + offset;
}

twobyte SNES_CPU::irq_vector() {
      return read16_bank0(e ? 0xFFFE : 0xFFEE);
}

twobyte SNES_CPU::brk_vector() {
	return read16_bank0(0xFFE6);
}

twobyte SNES_CPU::cop_vector() {
	return read16_bank0(0xFFE4);
}