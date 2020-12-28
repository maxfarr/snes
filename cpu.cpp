#include "common.h"

#include "cpu.h"
class SNES_MEMORY;
#include "ram.h"
#include <stdio.h>
#include <iostream>

#define DEBUG

int main() {
	// test program initializes SNES components individually
	SNES_MEMORY* mem = new SNES_MEMORY();
	std::cout << "reading ROM file" << std::endl;
	mem->openROM("testrom_3adc");
	std::cout << "read file" << std::endl;
	
	SNES_CPU* cpu = new SNES_CPU(mem);
	
	for(int i = 0; i < 9; i++) {
		cpu->clock();
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

void SNES_CPU::clock() {
	if(cyclesRemaining == 0) {
		//get opcode
		byte opcode = mem->readROM8(K, PC);
		
		(this->ops[opcode]).mode();
		(this->ops[opcode]).op();
		
		cyclesRemaining += (this->ops[opcode].cycleCount)();
		
#ifdef DEBUG
		std::cout << "-- executed opcode 0x" << std::hex << (unsigned int)opcode << std::dec << std::endl;
		debugPrint();
#endif
	}
	cyclesRemaining--;
}

void SNES_CPU::debugPrint() {
	std::cout << "status flags: " << std::endl;
	std::cout << "n v m x d i z c" << std::endl;
	for(int i = 7; i >= 0; i--)
		std::cout << getBit(status.full, i) << " ";
	std::cout << std::endl;
	std::cout << "accumulator: " << std::endl;
	for(int i = 15; i >= 0; i--)
		std::cout << getBit(C, i);
	std::cout << std::endl << std::endl;
}

//
// operations
//

void SNES_CPU::ADC() {
	if(status.bits.m) {
		bool final_c = ((twobyte)*A + (*fetched_lo + status.bits.c) > (twobyte)0xFF);
		bool initial_high_bit = getBit(*A, 7);
		
		*A += (*fetched_lo + status.bits.c);
		
		bool new_high_bit = getBit(*A, 7);
		
		status.bits.c = final_c;
		status.bits.v = ((initial_high_bit == getBit((*fetched_lo + status.bits.c), 7)) && (initial_high_bit != new_high_bit));
		status.bits.n = getBit(*A, 7);
		status.bits.z = (*A == 0x00);
	} else {
		bool final_c = ((unsigned int)C + (fetched + status.bits.c) > (unsigned int)0xFFFF);
		bool initial_high_bit = getBit(C, 15);
		
		C += (fetched + status.bits.c);
		
		bool new_high_bit = getBit(C, 15);
		
		status.bits.c = final_c;
		status.bits.v = ((initial_high_bit == getBit((fetched + status.bits.c), 15)) && (initial_high_bit != new_high_bit));
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
	} else {
		twobyte data = mem->read16(*fetched_addr_bank, *fetched_addr_abs);
		
		status.bits.c = getBit(data, 15);
		data <<= 1;
		
		mem->write16(*fetched_addr_bank, *fetched_addr_abs, data);
		
		status.bits.n = getBit(data, 15);
	}
}

void SNES_CPU::ASLA() {
	if(status.bits.m) {
		status.bits.c = getBit(*A, 7);
		*A <<= 1;
		
		status.bits.n = getBit(*A, 7);
	} else {
		status.bits.c = getBit(C, 15);
		C <<= 1;
		
		status.bits.n = getBit(C, 15);
	}
}

void SNES_CPU::BCC() {
	if(!status.bits.c){
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BCS() {
	if(status.bits.c){
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BEQ() {
	if(status.bits.z){
		mem->branchPC(K, PC, *fetched_lo);
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
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BNE() {
	if(!status.bits.z){
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BPL() {
	if(!status.bits.n){
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BRA() {
	mem->branchPC(K, PC, *fetched_lo);
}

void SNES_CPU::BRL() {
	PC += fetched;
}

void SNES_CPU::BVC() {
	if(!status.bits.v){
		mem->branchPC(K, PC, *fetched_lo);
		branchTaken = true;
	} else branchTaken = false;
}

void SNES_CPU::BVS() {
	if(status.bits.v){
		mem->branchPC(K, PC, *fetched_lo);
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
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(0x00, *fetched_addr_abs);
	else
		fetched = mem->read16(0x00, *fetched_addr_abs);
}

void SNES_CPU::DPX() {
	twobyte addr = D + mem->readROM8(K, PC) + (status.bits.x ? *XL : X);
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(0x00, addr);
	else
		fetched = mem->read16(0x00, addr);
}

void SNES_CPU::DPY() {
	twobyte addr_abs = D + mem->readROM8(K, PC) + (status.bits.x ? *YL : Y);
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(0x00, addr_abs);
	else
		fetched = mem->read16(0x00, addr_abs);
}

// indirect

void SNES_CPU::DPI() {
	twobyte addr = mem->read16(0x00, D + mem->readROM8(K, PC));
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

void SNES_CPU::DPIL() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIX() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	
	if(status.bits.x)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIY() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	
	if(status.bits.x)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	
	iBoundary = (DBR != ((addr & 0xFF0000) >> 16));
}

void SNES_CPU::DPXI() {
	twobyte addr_abs = D + mem->readROM8(K, PC) + (status.bits.x ? *XL : X);
	twobyte ptr_abs = mem->read16(0x00, addr_abs);
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(DBR, ptr_abs);
	else
		fetched = mem->read16(DBR, ptr_abs);
}

void SNES_CPU::DPYI() {
	twobyte addr_abs = D + mem->readROM8(K, PC) + (status.bits.x ? *YL : Y);
	twobyte ptr_abs = mem->read16(0x00, addr_abs);
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(DBR, ptr_abs);
	else
		fetched = mem->read16(DBR, ptr_abs);
}

void SNES_CPU::DPILX() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	
	if(status.bits.x)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILY() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	
	if(status.bits.x)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// absolute

void SNES_CPU::ABS() {
	*fetched_addr_bank = DBR;
	*fetched_addr_abs = mem->readROM16(K, PC);
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8(DBR, *fetched_addr_abs);
	else
		fetched = mem->read16(DBR, *fetched_addr_abs);
}

void SNES_CPU::ABSL() {
	threebyte addr = mem->readROM24(K, PC);
	if(status.bits.m && status.bits.x)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSX() {
	threebyte addr_long = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	
	if(status.bits.x)
		addr_long += *XL;
	else
		addr_long += X;
	
	
	fetched_addr = addr_long;
	
	if(status.bits.m && status.bits.x)
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