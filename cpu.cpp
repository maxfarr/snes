#include "common.h"

#include "cpu.h"
class SNES_MEMORY;
#include "ram.h"
#include <stdio.h>
#include <iostream>

int main() {
	// test program initializes SNES components individually
	SNES_MEMORY* mem = new SNES_MEMORY();
	std::cout << "reading ROM file" << std::endl;
	mem->openROM("smw");
	//mem->openROM("testrom_3adc");
	//mem->openROM("testrom_sei");
	std::cout << "finished reading file" << std::endl;
	
	SNES_CPU* cpu = new SNES_CPU(mem);
	
	for(int i = 0; i < 10000; i++) {
		if (!cpu->clock()) {
			break;
		}
	}
	
	return 0;
}

SNES_CPU::SNES_CPU(SNES_MEMORY* m) {
	mem = m;
	PC = 0x8000;
	status.full = 0x00;
}

SNES_CPU::~SNES_CPU() {
	//nothing yet
}

bool SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		//get opcode
		byte opcode = mem->readROM8(K, PC);
		if(opcode == 0x00) {
			std::cout << "opcode 0" << std::endl;
			return false;
		}
		
		(this->ops[opcode]).mode();
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
	std::cout << "n v m x d i z c" << std::endl;
	for(int i = 7; i >= 0; i--)
		std::cout << getBit(status.full, i) << " ";
	std::cout << std::endl;
	std::cout << "accumulator: " << C << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(C, i);
	std::cout << std::endl;
	std::cout << "fetched: " << fetched << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(fetched, i);
	std::cout << std::endl << std::endl;
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
	if(status.bits.m) {
		bool final_c = ((twobyte)*A + (*fetched_lo + GET_STATUS_C) > (twobyte)0xFF);
		bool initial_high_bit = getBit(*A, 7);
		
		*A += *fetched_lo;
		*A += GET_STATUS_C;
		
		bool new_high_bit = getBit(*A, 7);
		
		status.bits.c = final_c;
		status.bits.v = ((initial_high_bit == getBit((*fetched_lo + GET_STATUS_C), 7)) && (initial_high_bit != new_high_bit));
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		bool final_c = ((unsigned int)C + (fetched + GET_STATUS_C) > (unsigned int)0xFFFF);
		bool initial_high_bit = getBit(C, 15);
		
		C += fetched;
		C += GET_STATUS_C;
		
		bool new_high_bit = getBit(C, 15);
		
		status.bits.c = final_c;
		status.bits.v = ((initial_high_bit == getBit((fetched + GET_STATUS_C), 15)) && (initial_high_bit != new_high_bit));
		status.bits.n = getBit(C, 15);
		status.bits.z = (C == 0x0000);
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
		byte data = mem->read8(*fetched_addr_bank, *fetched_addr_abs);
		
		status.bits.c = getBit(data, 7);
		data <<= 1;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = mem->read16(*fetched_addr_bank, *fetched_addr_abs);
		
		status.bits.c = getBit(data, 15);
		data <<= 1;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data);
		
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
		status.bits.z = (*A == 0x00);
	}
}

void SNES_CPU::LSR() {
	if(status.bits.m) {
		byte data = mem->read8(*fetched_addr_bank, *fetched_addr_abs);
		
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 7);
		status.bits.z = (data == 0x00);
	} else {
		twobyte data = mem->read16(*fetched_addr_bank, *fetched_addr_abs);
		
		status.bits.c = getBit(data, 0);
		data >>= 1;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data);
		
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
		status.bits.z = (*A == 0x00);
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
}

void SNES_CPU::BRL() {
	PC += (signedtwobyte)fetched;
}

void SNES_CPU::BVC() {
	if(!GET_STATUS_V){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BVS() {
	if(GET_STATUS_V){
		PC += (signedbyte)*fetched_lo;
		branchTaken = true;
	} else branchTaken = false;
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
	} else {
		C = fetched;
	}
}

void SNES_CPU::LDX() {
	if(status.bits.x) {
		*XL = *fetched_lo;
	} else {
		X = fetched;
	}
}

void SNES_CPU::LDY() {
	if(status.bits.x) {
		*YL = *fetched_lo;
	} else {
		Y = fetched;
	}
}

void SNES_CPU::SEC() {
	status.bits.c = 1;
}

void SNES_CPU::SED() {
	status.bits.d = 1;
}

void SNES_CPU::SEI() {
	status.bits.i = 1;
}

void SNES_CPU::STA() {
	if(status.bits.m) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *A);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, C, wrap_writes);
	}
}

void SNES_CPU::STX() {
	if(status.bits.m) {
		mem->write8(*fetched_addr_bank, *fetched_addr_abs, *XL);
	} else {
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, X, wrap_writes);
	}
}

void SNES_CPU::STY() {
	if(status.bits.m) {
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
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

// Direct Page Indirect Long
void SNES_CPU::DPIL() {
	threebyte addr = mem->read24_bank0(D + mem->readROM8(K, PC));
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// Direct Page Indirect, X
void SNES_CPU::DPIX() {
	twobyte addr = mem->read16_bank0(D + mem->readROM8(K, PC) + X);
	
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

// Direct Page Indirect iNdexed, Y
void SNES_CPU::DPINY() {
	twobyte addr = mem->read16_bank0(D + mem->readROM8(K, PC));
	
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr + Y);
	else
		fetched = mem->read16(DBR, addr + Y);
}

// Direct Page Indirect Long iNdexed, Y
void SNES_CPU::DPILNY() {
	threebyte addr = mem->read24_bank0(D + mem->readROM8(K, PC));
	
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

void SNES_CPU::ABS_JMP_JSR() {
	*fetched_addr_bank = K;
	*fetched_addr_abs = mem->readROM16(K, PC);

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
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
	else
		fetched = mem->read16((addr_long & 0xFF0000) >> 16, addr_long & 0xFFFF);
}

// stack relative

void SNES_CPU::SR() {
	twobyte addr = (twobyte)mem->readROM8(K, PC) + S;
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(0x00, addr);
	else
		fetched = mem->read16(0x00, addr);
}

void SNES_CPU::SRIX() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	
	if(status.bits.x)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::SRIY() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	
	if(status.bits.x)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}