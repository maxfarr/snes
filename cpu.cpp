#include "common.h"

#include "cpu.hpp"
class SNES_MEMORY;
#include "ram.hpp"

#include <stdio.h>
#include <iostream>
#include <iomanip>

SNES_CPU::SNES_CPU() {
	mem = new SNES_MEMORY();
}

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

void SNES_CPU::init() {
	status.full = 0x00;
	e = 1;
	status.bits.m = 1;
	status.bits.x = 1;
	D = 0x0000;
	DBR = 0x00;
	PC = mem->reset_vector();
	*SH = 0x01;
}

bool SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		// check for m/x/e flags
		updateRegisterWidths();

		// get opcode
		byte opcode = mem->readROM8(K, PC);
		
		// fetch data based on addressing mode
		(this->ops[opcode]).mode();
		// execute op
		(this->ops[opcode]).op();
		
		cyclesRemaining += (this->ops[opcode].cycleCount)();

		iBoundary = false;
		branchTaken = false;
		branchBoundary = false;
		wrap_writes = false;
		
#ifdef DEBUG
		std::cout << "-- executed opcode 0x" << std::hex << (unsigned int)opcode << std::dec << std::endl;
		debugPrint();
#endif
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
	std::cout << "fetched: " << fetched << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(fetched, i);
	std::cout << std::endl;
	std::cout << "K: " << std::hex << HEX_BYTE_PRINT(K) << std::dec << std::endl;
	std::cout << "PC: " << std::hex << std::setw(4) << PC << std::dec << std::endl;
	std::cout << "D: " << std::hex << std::setw(4) << D << std::dec << std::endl;
	std::cout << std::endl;
}

void SNES_CPU::updateRegisterWidths() {
	if(e) {
		status.bits.m = 1;
		status.bits.x = 1;
	}

	if(status.bits.m) {
		*B = 0x00;
	}

	if(status.bits.x) {
		*XH = 0x00;
		*YH = 0x00;
	}
}

void SNES_CPU::push_stack_threebyte(threebyte value) {
	mem->write8(0x00, S, value >> 16);
	S--;
	mem->write8(0x00, S, (value >> 8) & 0xFF);
	S--;
	mem->write8(0x00, S, value & 0xFF);
	S--;
}

void SNES_CPU::push_stack_twobyte(twobyte value) {
	mem->write8(0x00, S, value >> 8);
	S--;
	mem->write8(0x00, S, value & 0xFF);
	S--;
}

void SNES_CPU::push_stack_byte(byte value) {
	mem->write8(0x00, S, value);
	S--;
}

threebyte SNES_CPU::pop_stack_threebyte() {
	S++;
	threebyte result = mem->read24_bank0(S);
	S += 2;
	return result;
}

twobyte SNES_CPU::pop_stack_twobyte() {
	S++;
	twobyte result = mem->read16_bank0(S);
	S++;
	return result;
}

byte SNES_CPU::pop_stack_byte() {
	S++;
	return mem->read8_bank0(S);
}

//
// operations
//

void SNES_CPU::ADC() {
	if(status.bits.d) {
		status.bits.c = 0;
		byte lower_nybble_sum = (*A & 0x0F) + (*fetched_lo & 0x0F) + status.bits.c;
		byte upper_nybble_sum = (*A >> 4) + (*fetched_lo >> 4);

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
			byte lower_nybble_sum_hi = (*B & 0x0F) + (*fetched_hi & 0x0F);
			byte upper_nybble_sum_hi = (*B >> 4) + (*fetched_hi >> 4);

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
			status.bits.z = (C = 0x0000);
			return;
		}
	} else {
		if(status.bits.m) {
			bool final_c = ((twobyte)*A + (*fetched_lo + status.bits.c) > (twobyte)0xFF);
			bool high_bit_pre_adc = getBit(*A, 7);
			
			*A += *fetched_lo;
			*A += status.bits.c;
			
			status.bits.v = ((high_bit_pre_adc == getBit((*fetched_lo + status.bits.c), 7)) && (high_bit_pre_adc != getBit(*A, 7)));
			status.bits.c = final_c;
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
		} else {
			bool final_c = ((threebyte)C + (fetched + status.bits.c) > (threebyte)0xFFFF);
			bool high_bit_pre_adc = getBit(C, 15);
			
			C += fetched;
			C += status.bits.c;
			
			status.bits.v = ((high_bit_pre_adc == getBit((fetched + status.bits.c), 15)) && (high_bit_pre_adc != getBit(C, 15)));
			status.bits.c = final_c;
			status.bits.n = getBit(C, 15);
			status.bits.z = (C == 0x0000);
		}
	}
}

void SNES_CPU::AND() {
	if(status.bits.m) {
		*A &= *fetched_lo;
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C &= fetched;
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::ASL() {
	if(status.bits.m) {
		byte data = *fetched_lo;
		
		status.bits.c = getBit(data, 7);
		data <<= 1;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = fetched;
		
		status.bits.c = getBit(data, 15);
		data <<= 1;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data, wrap_writes);
		
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
	if(status.bits.m) {
		byte data = *fetched_lo;
		
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = fetched;
		
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data, wrap_writes);
		
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
	if(!status.bits.c){
		PC += *fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BCS() {
	if(status.bits.c){
		PC += *fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BEQ() {
	if(status.bits.z){
		PC += *fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BIT() {
	if(status.bits.m) {
		byte data = *A;
		data &= *fetched_lo;
		
		status.bits.n = getBit(*A, 7);
		status.bits.v = getBit(*A, 6);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = C;
		data &= fetched;
		
		status.bits.n = getBit(C, 15);
		status.bits.v = getBit(C, 14);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::BITIMM() {
	if(status.bits.m) {
		byte data = *A;
		data &= *fetched_lo;
		
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = C;
		data &= fetched;
		
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::BMI() {
	if(status.bits.n){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BNE() {
	if(!status.bits.z){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BPL() {
	if(!status.bits.n){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BRA() {
	PC += (signedbyte)*fetched_lo;
	branchTaken = true;
}

void SNES_CPU::BRL() {
	PC += (signedtwobyte)fetched;
	branchTaken = true;
}

void SNES_CPU::BVC() {
	if(!status.bits.v){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BVS() {
	if(status.bits.v){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BRK() {
	push_stack_byte(K);
	push_stack_twobyte(PC);
	push_stack_byte(status.full);

	DBR = 0x00;

	status.bits.d = 0;
	status.bits.i = 0;

	K = 0x00;
	PC = mem->brk_vector();
}

void SNES_CPU::COP() {
	push_stack_byte(K);
	push_stack_twobyte(PC);
	push_stack_byte(status.full);

	DBR = 0x00;

	status.bits.d = 0;
	status.bits.i = 0;

	K = 0x00;
	PC = mem->cop_vector();
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
	if(status.bits.m) {
		byte A_copy = *A;
		
		status.bits.c = (A_copy >= *fetched_lo);
		
		A_copy -= *fetched_lo;
		
		status.bits.n = getBit(A_copy, 7);
		status.bits.z = (A_copy == 0x00);
	} else {
		twobyte C_copy = C;
		
		status.bits.c = (C_copy >= fetched);
		
		C_copy -= fetched;
		
		status.bits.n = getBit(C_copy, 15);
		status.bits.z = (C_copy == 0x0000);
	}
}

void SNES_CPU::CPX() {
	if(status.bits.x) {
		byte X_copy = *XL;
		
		status.bits.c = (X_copy >= *fetched_lo);
		
		X_copy -= *fetched_lo;
		
		status.bits.n = getBit(X_copy, 7);
		status.bits.z = (X_copy == 0x00);
	} else {
		twobyte X_copy = X;
		
		status.bits.c = (X_copy >= fetched);
		
		X_copy -= fetched;
		
		status.bits.n = getBit(X_copy, 15);
		status.bits.z = (X_copy == 0x0000);
	}
}

void SNES_CPU::CPY() {
	if(status.bits.x) {
		byte Y_copy = *YL;
		
		status.bits.c = (Y_copy >= *fetched_lo);
		
		Y_copy -= *fetched_lo;
		
		status.bits.n = getBit(Y_copy, 7);
		status.bits.z = (Y_copy == 0x00);
	} else {
		twobyte Y_copy = Y;
		
		status.bits.c = (Y_copy >= fetched);
		
		Y_copy -= fetched;
		
		status.bits.n = getBit(Y_copy, 15);
		status.bits.z = (Y_copy == 0x0000);
	}
}

void SNES_CPU::DEC() {
	if(status.bits.m) {
		byte data = mem->read8(*fetched_addr_bank, *fetched_addr_abs);
		
		data--;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = mem->read16(*fetched_addr_bank, *fetched_addr_abs);
		
		data--;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data);
		
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
	if(status.bits.m) {
		*A ^= *fetched_lo;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C ^= fetched;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::INC() {
	if(status.bits.m) {
		byte data = mem->read8(*fetched_addr_bank, *fetched_addr_abs);
		
		data++;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = mem->read16(*fetched_addr_bank, *fetched_addr_abs);
		
		data++;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data);
		
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
	PC = fetched;
}

void SNES_CPU::JML() {
	K = jump_long_addr >> 16;
	PC = jump_long_addr & 0xFFFF;
}

void SNES_CPU::JSR() {
	PC--;
	push_stack_twobyte(PC);
	PC = fetched;
}

void SNES_CPU::JSL() {
	PC--;
	push_stack_byte(K);
	push_stack_twobyte(PC);
	K = jump_long_addr >> 16;
	PC = jump_long_addr & 0xFFFF;
}

void SNES_CPU::LDA() {
	if(status.bits.m) {
		*A = *fetched_lo;

		status.bits.n = getBit(*fetched_lo, 7);
		status.bits.z = (*fetched_lo == 0x00);
	} else {
		C = fetched;

		status.bits.n = getBit(fetched, 15);
		status.bits.z = (fetched == 0x0000);
	}
}

void SNES_CPU::LDX() {
	if(status.bits.x) {
		*XL = *fetched_lo;

		status.bits.n = getBit(*fetched_lo, 7);
		status.bits.z = (*fetched_lo == 0x00);
	} else {
		X = fetched;

		status.bits.n = getBit(fetched, 15);
		status.bits.z = (fetched == 0x0000);
	}
}

void SNES_CPU::LDY() {
	if(status.bits.x) {
		*YL = *fetched_lo;

		status.bits.n = getBit(*fetched_lo, 7);
		status.bits.z = (*fetched_lo == 0x00);
	} else {
		Y = fetched;

		status.bits.n = getBit(fetched, 15);
		status.bits.z = (fetched == 0x0000);
	}
}

void SNES_CPU::MVN() {
	byte source_bank = *fetched_lo;
	byte dest_bank = *fetched_hi;

	while(C != 0xFFFF) {
		mem->write8(dest_bank, Y, mem->read8(source_bank, X));
		X++;
		Y++;
		C--;
	}
}

void SNES_CPU::MVP() {
	byte source_bank = *fetched_lo;
	byte dest_bank = *fetched_hi;

	while(C != 0xFFFF) {
		mem->write8(dest_bank, Y, mem->read8(source_bank, X));
		X--;
		Y--;
		C--;
	}
}

void SNES_CPU::ORA() {
	if(status.bits.m) {
		*A |= *fetched_lo;

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C |= fetched;

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::PEA() {
	push_stack_twobyte(fetched);
}

void SNES_CPU::PEI() {
	push_stack_twobyte(fetched);
}

void SNES_CPU::PER() {
	push_stack_twobyte(fetched);
}

// push

void SNES_CPU::PHA() {
	if(status.bits.m) {
		push_stack_twobyte(C);
	} else {
		push_stack_byte(*A);
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
	push_stack_byte(status.full);
}

void SNES_CPU::PHX() {
	if(status.bits.x) {
		push_stack_twobyte(X);
	} else {
		push_stack_byte(*XL);
	}
}

void SNES_CPU::PHY() {
	if(status.bits.x) {
		push_stack_twobyte(Y);
	} else {
		push_stack_byte(*YL);
	}
}

// pull

void SNES_CPU::PLA() {
	if(status.bits.m) {
		C = pop_stack_twobyte();

		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	} else {
		*A = pop_stack_byte();

		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
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
	status.full = pop_stack_byte();

	if(e) {
		status.bits.m = 1;
		status.bits.x = 1;
	}
}

void SNES_CPU::PLX() {
	if(status.bits.m) {
		X = pop_stack_twobyte();

		status.bits.n = getBit(X, 15);
		status.bits.z = (X == 0x0000);
	} else {
		*XL = pop_stack_byte();

		status.bits.n = getBit(*XL, 7);
		status.bits.z = (*XL == 0x00);
	}
}

void SNES_CPU::PLY() {
	if(status.bits.m) {
		Y = pop_stack_twobyte();

		status.bits.n = getBit(Y, 15);
		status.bits.z = (Y == 0x0000);
	} else {
		*YL = pop_stack_byte();

		status.bits.n = getBit(*YL, 7);
		status.bits.z = (*YL == 0x00);
	}
}

void SNES_CPU::REP() {
	status.full &= ~(*fetched_lo);

	if(e) {
		status.bits.m = 1;
		status.bits.x = 1;
	}
}

void SNES_CPU::ROL() {
	bool c_pre_shift = status.bits.c;
	if(status.bits.m) {
		byte data = *fetched_lo;
		
		status.bits.c = getBit(data, 7);
		data <<= 1;
		data |= c_pre_shift;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = fetched;
		
		status.bits.c = getBit(data, 15);
		data <<= 1;
		data |= c_pre_shift;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data, wrap_writes);
		
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
	bool c_pre_shift = status.bits.c;
	status.bits.c = getBit(fetched, 0);
	if(status.bits.m) {
		byte data = *fetched_lo;
		
		data >>= 1;
		data |= (c_pre_shift << 7);
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = fetched;
		
		data <<= 1;
		data |= (c_pre_shift << 15);
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data, wrap_writes);
		
		status.bits.n = getBit(data, 15);
		status.bits.z = (data == 0x0000);
	}
}

void SNES_CPU::RORA() {
	bool c_pre_shift = status.bits.c;
	status.bits.c = getBit(C, 0);
	if(status.bits.m) {
		*A <<= 1;
		*A |= (c_pre_shift << 7);
		
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		C <<= 1;
		C |= (c_pre_shift << 15);
		
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
	}
}

void SNES_CPU::RTI() {
	status.full = pop_stack_byte();
	PC = pop_stack_twobyte();

	if(!e) {
		K = pop_stack_byte();
	}
}

void SNES_CPU::RTS() {
	PC = pop_stack_twobyte();
	PC++;

	K = pop_stack_byte();
}

void SNES_CPU::RTL() {
	PC = pop_stack_twobyte();
	PC++;
}

void SNES_CPU::SBC() {
	if(status.bits.d) {
		status.bits.c = 1;
		byte lower_nybble_diff = (*A & 0x0F) - (*fetched_lo & 0x0F) - (status.bits.c ? 0 : 1);
		byte upper_nybble_diff = (*A >> 4) - (*fetched_lo >> 4);

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
			byte lower_nybble_diff_hi = (*B & 0x0F) - (*fetched_hi & 0x0F);
			byte upper_nybble_diff_hi = (*B >> 4) - (*fetched_hi >> 4);

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
			bool final_c = (*A >= *fetched_lo);
			bool high_bit_pre_sbc = getBit(*A, 7);
			
			*A -= *fetched_lo;
			*A -= (status.bits.c ? 0 : 1);
			
			status.bits.v = ((high_bit_pre_sbc != getBit(*fetched_lo + (status.bits.c ? 0 : 1), 7))
							&& (high_bit_pre_sbc != getBit(*A, 7)));
			status.bits.c = final_c;
			status.bits.n = getBit(*A, 7);
			status.bits.z = (*A == 0x00);
		} else {
			bool final_c = (C >= fetched);
			bool high_bit_pre_sbc = getBit(C, 15);
			
			C -= fetched;
			C -= (status.bits.c ? 0 : 1);
			
			status.bits.v = ((high_bit_pre_sbc != getBit(fetched + (status.bits.c ? 0 : 1), 15))
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
	status.full |= *fetched_lo;

	if(status.bits.x) {
		*XH = 0x00;
		*XL = 0x00;
	}
}

void SNES_CPU::STA() {
	if(status.bits.m) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *A);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, C, wrap_writes);
	}
}

void SNES_CPU::STX() {
	if(status.bits.x) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *XL);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, X, wrap_writes);
	}
}

void SNES_CPU::STY() {
	if(status.bits.x) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *YL);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, Y, wrap_writes);
	}
}

void SNES_CPU::STZ() {
	if(status.bits.m) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *A);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, C, wrap_writes);
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

	status.bits.n = getBit(C, 15);
	status.bits.z = (C == 0x0000);
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

	status.bits.n = getBit(X, 15);
	status.bits.z = (X == 0x0000);
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
	if(status.bits.m) {
		byte data = *fetched_lo;

		status.bits.z = ((*A & data) == 0x00);

		for(int i = 0; i < 8; i++) {
			if(getBit(*A, i)) {
				data &= ~(1 << i);
			}
		}

		mem->write8(DBR, *fetched_addr_abs, data);
	} else {
		twobyte data = fetched;

		status.bits.z = ((C & data) == 0x0000);

		for(int i = 0; i < 16; i++) {
			if(getBit(C, i)) {
				data &= ~(1 << i);
			}
		}

		mem->write16(DBR, *fetched_addr_abs, data, wrap_writes);
	}
}

void SNES_CPU::TSB() {
	if(status.bits.m) {
		byte data = *fetched_lo;

		status.bits.z = ((*A & data) == 0x00);

		for(int i = 0; i < 8; i++) {
			if(getBit(*A, i)) {
				data |= (1 << i);
			}
		}

		mem->write8(DBR, *fetched_addr_abs, data);
	} else {
		twobyte data = fetched;

		status.bits.z = ((C & data) == 0x0000);

		for(int i = 0; i < 16; i++) {
			if(getBit(C, i)) {
				data |= (1 << i);
			}
		}

		mem->write16(DBR, *fetched_addr_abs, data, wrap_writes);
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
	bool carry = status.bits.c;
	status.bits.c = e;
	e = carry;
	if(e) {
		status.bits.m = 1;
		status.bits.x = 1;
	}
}

//
// addressing modes
//

// immediate

void SNES_CPU::IMM_M() {
	if(status.bits.m)
		*fetched_lo = mem->readROM8(K, PC);
	else
		fetched = mem->readROM16(K, PC);
}

void SNES_CPU::IMM_X() {
	if(status.bits.x)
		*fetched_lo = mem->readROM8(K, PC);
	else
		fetched = mem->readROM16(K, PC);
}

void SNES_CPU::IMM8() {
	*fetched_lo = mem->readROM8(K, PC);
}

void SNES_CPU::IMM16() {
	fetched = mem->readROM16(K, PC);
}

// direct page

void SNES_CPU::DP() {
	*fetched_addr_bank = 0x00;
	*fetched_addr_abs = D + mem->readROM8(K, PC);
	wrap_writes = true;

	if(status.bits.m) {
		*fetched_lo = mem->read8_bank0(*fetched_addr_abs);
	} else {
		fetched = mem->read16_bank0(*fetched_addr_abs);
	}
}

void SNES_CPU::DP16() {
	*fetched_addr_bank = 0x00;
	*fetched_addr_abs = D + mem->readROM8(K, PC);
	wrap_writes = true;

	fetched = mem->read16_bank0(*fetched_addr_abs);
}

// Direct Page Indexed, X
void SNES_CPU::DPX() {
	*fetched_addr_bank = 0x00;
	*fetched_addr_abs = D + mem->readROM8(K, PC) + (status.bits.x ? *XL : X);
	wrap_writes = true;
	
	if(status.bits.m) {
		*fetched_lo = mem->read8_bank0(*fetched_addr_abs);
	} else {
		fetched = mem->read16_bank0(*fetched_addr_abs);
	}
}

// Direct Page Indexed, Y
void SNES_CPU::DPY() {
	*fetched_addr_bank = 0x00;
	*fetched_addr_abs = D + mem->readROM8(K, PC) + (status.bits.x ? *YL : Y);
	wrap_writes = true;

	if(status.bits.m) {
		*fetched_lo = mem->read8_bank0(*fetched_addr_abs);
	} else {
		fetched = mem->read16_bank0(*fetched_addr_abs);
	}
}

// indirect

// Direct Page Indirect
void SNES_CPU::DPI() {
	twobyte addr = mem->read16_bank0(D + mem->readROM8(K, PC));

	*fetched_addr_bank = DBR;
	*fetched_addr_abs = addr;

	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

// Direct Page Indirect Long
void SNES_CPU::DPIL() {
	threebyte addr = mem->read24_bank0(D + mem->readROM8(K, PC));

	fetched_addr = addr;

	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// Direct Page Indirect, X
void SNES_CPU::DPIX() {
	twobyte addr = mem->read16_bank0(D + mem->readROM8(K, PC) + X);

	*fetched_addr_bank = DBR;
	*fetched_addr_abs = addr;
	
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

// Direct Page Indirect iNdexed, Y
void SNES_CPU::DPINY() {
	twobyte addr = mem->read16_bank0(D + mem->readROM8(K, PC));

	*fetched_addr_bank = DBR;
	*fetched_addr_abs = addr;
	
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr + Y);
	else
		fetched = mem->read16(DBR, addr + Y);
}

// Direct Page Indirect Long iNdexed, Y
void SNES_CPU::DPILNY() {
	threebyte addr = mem->read24_bank0(D + mem->readROM8(K, PC));

	fetched_addr = addr;
	
	if(status.bits.m)
		*fetched_lo = mem->read8(addr + Y);
	else
		fetched = mem->read16(addr + Y);
}

// absolute

void SNES_CPU::ABS() {
	*fetched_addr_bank = DBR;
	*fetched_addr_abs = mem->readROM16(K, PC);
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, *fetched_addr_abs);
	else
		fetched = mem->read16(DBR, *fetched_addr_abs);
}

void SNES_CPU::ABSI() {
	fetched = mem->read16_bank0(mem->readROM16(K, PC));
}

void SNES_CPU::ABSIL() {
	jump_long_addr = mem->read24_bank0(mem->readROM16(K, PC));
}

void SNES_CPU::ABSIX() {
	twobyte HHLL = mem->readROM16(K, PC);
	twobyte addr = mem->read8(K, HHLL + X);
	addr |= (mem->read8(K, HHLL + X + 1) << 8);

	fetched = addr;
}

void SNES_CPU::ABSL() {
	threebyte addr = mem->readROM24(K, PC);

	fetched_addr = addr;

	if(status.bits.m)
		*fetched_lo = mem->read8(addr);
	else
		fetched = mem->read16(addr);
}

void SNES_CPU::ABSL_JML_JSL() {
	jump_long_addr = mem->readROM24(K, PC);
}

void SNES_CPU::ABSX() {
	threebyte addr_long = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	
	if(status.bits.x)
		addr_long += *XL;
	else
		addr_long += X;
	
	fetched_addr = addr_long;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	else
		fetched = mem->read16((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	
	iBoundary = (DBR != ((addr_long & 0xFF0000) >> 16));
}

void SNES_CPU::ABSY() {
	threebyte addr_long = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	
	if(status.bits.x)
		addr_long += *YL;
	else
		addr_long += Y;
	
	fetched_addr = addr_long;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	else
		fetched = mem->read16((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	
	iBoundary = (DBR != ((addr_long & 0xFF0000) >> 16));
}

void SNES_CPU::ABSLX() {
	threebyte addr_long = mem->readROM24(K, PC);
	
	if(status.bits.x)
		addr_long += *XL;
	else
		addr_long += X;

	fetched_addr = addr_long;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	else
		fetched = mem->read16((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
}

void SNES_CPU::ABSLY() {
	threebyte addr_long = mem->readROM24(K, PC);
	
	if(status.bits.x)
		addr_long += *YL;
	else
		addr_long += Y;

	fetched_addr = addr_long;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	else
		fetched = mem->read16((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
}

// stack relative

void SNES_CPU::SR() {
	twobyte addr = (twobyte)mem->readROM8(K, PC) + S;

	*fetched_addr_bank = 0x00;
	*fetched_addr_abs = addr;
	wrap_writes = true;

	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8_bank0(addr);
	else
		fetched = mem->read16_bank0(addr);
}

void SNES_CPU::SRIY() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	
	if(status.bits.x)
		addr += *YL;
	else
		addr += Y;

	fetched_addr = addr;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}