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
		
		cyclesRemaining += (this->ops[opcode].cycleCount)();
		
		(this->ops[opcode]).mode();
		(this->ops[opcode]).op();
		
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

// ADC

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

//
// addressing modes
//

// immediate

void SNES_CPU::IMM() {
	if(status.bits.m)
		*fetched_lo = mem->readROM8(K, PC);
	else
		fetched = mem->readROM16(K, PC);
}

// direct page

void SNES_CPU::DP() {
	if(status.bits.m)
		*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC));
	else
		fetched = mem->read16(0x00, D + mem->readROM8(K, PC));
}

void SNES_CPU::DPX() {
	if(status.bits.m)
		*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC) + X);
	else
		fetched = mem->read16(0x00, D + mem->readROM8(K, PC) + X);
}

void SNES_CPU::DPY() {
	if(status.bits.m)
		*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC) + Y);
	else
		fetched = mem->read16(0x00, D + mem->readROM8(K, PC) + Y);
}

// indirect

void SNES_CPU::DPI() {
	twobyte addr = mem->read16(0x00, D + mem->readROM8(K, PC));
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

void SNES_CPU::DPIL() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIX() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	
	if(status.bits.i)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIY() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	
	if(status.bits.i)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILX() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	
	if(status.bits.i)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILY() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	
	if(status.bits.i)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// absolute

void SNES_CPU::ABS() {
	twobyte addr = mem->readROM16(K, PC);
	if(status.bits.m)
		*fetched_lo = mem->read8(DBR, addr);
	else
		fetched = mem->read16(DBR, addr);
}

void SNES_CPU::ABSL() {
	threebyte addr = mem->readROM24(K, PC);
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSX() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	
	if(status.bits.i)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSY() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	
	if(status.bits.i)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLX() {
	threebyte addr = mem->readROM24(K, PC);
	
	if(status.bits.i)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLY() {
	threebyte addr = mem->readROM24(K, PC);
	
	if(status.bits.i)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// stack relative

void SNES_CPU::SR() {
	twobyte addr = (twobyte)mem->readROM8(K, PC) + S;
	if(status.bits.m)
		*fetched_lo = mem->read8(0x00, addr);
	else
		fetched = mem->read16(0x00, addr);
}

void SNES_CPU::SRIX() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	
	if(status.bits.i)
		addr += *XL;
	else
		addr += X;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::SRIY() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	
	if(status.bits.i)
		addr += *YL;
	else
		addr += Y;
	
	if(status.bits.m)
		*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
	else
		fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}