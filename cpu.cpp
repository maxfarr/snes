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
		
		(*(this->ops[opcode]).mode)();
		(*(this->ops[opcode]).op)();
		
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

void SNES_CPU::ADC16() {
	bool final_c = ((unsigned int)C + (fetched + status.bits.c) > (unsigned int)0xFFFF);
	bool initial_high_bit = getBit(C, 15);
	
	C += (fetched + status.bits.c);
	
	bool new_high_bit = getBit(C, 15);
	
	status.bits.c = final_c;
	status.bits.v = ((initial_high_bit == getBit((fetched + status.bits.c), 15)) && (initial_high_bit != new_high_bit));
	status.bits.n = getBit(C, 15);
	status.bits.z = (C == 0x0000);
}

void SNES_CPU::ADC8() {
	bool final_c = ((twobyte)*A + (*fetched_lo + status.bits.c) > (twobyte)0xFF);
	bool initial_high_bit = getBit(*A, 7);
	
	*A += (*fetched_lo + status.bits.c);
	
	bool new_high_bit = getBit(*A, 7);
	
	status.bits.c = final_c;
	status.bits.v = ((initial_high_bit == getBit((*fetched_lo + status.bits.c), 7)) && (initial_high_bit != new_high_bit));
	status.bits.n = getBit(*A, 7);
	status.bits.z = (*A == 0x00);
}

//
// addressing modes
//

// immediate

void SNES_CPU::IMM16() {
	fetched = mem->readROM16(K, PC);
}

void SNES_CPU::IMM8() {
	*fetched_lo = mem->readROM8(K, PC);
}

// direct page

void SNES_CPU::DP16() {
	fetched = mem->read16(0x00, D + mem->readROM8(K, PC));
}

void SNES_CPU::DP8() {
	*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC));
}

void SNES_CPU::DPX16() {
	fetched = mem->read16(0x00, D + mem->readROM8(K, PC) + X);
}

void SNES_CPU::DPX8() {
	*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC) + X);
}

void SNES_CPU::DPY16() {
	fetched = mem->read16(0x00, D + mem->readROM8(K, PC) + Y);
}

void SNES_CPU::DPY8() {
	*fetched_lo = mem->read8(0x00, D + mem->readROM8(K, PC) + Y);
}

// indirect

void SNES_CPU::DPI16() {
	twobyte addr = mem->read16(0x00, D + mem->readROM8(K, PC));
	fetched = mem->read16(DBR, addr);
}

void SNES_CPU::DPI8() {
	twobyte addr = mem->read16(0x00, D + mem->readROM8(K, PC));
	*fetched_lo = mem->read8(DBR, addr);
}

void SNES_CPU::DPIL16() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIL8() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIX16() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIX8() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	addr += X;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIY16() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	addr += Y;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPIY8() {
	threebyte addr = mem->read16(0x00, D + mem->readROM8(K, PC)) + (DBR << 16);
	addr += Y;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILX16() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILX8() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	addr += X;
	*fetched_lo  = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILY16() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	addr += Y;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::DPILY8() {
	threebyte addr = mem->read24(0x00, D + mem->readROM8(K, PC));
	addr += Y;
	*fetched_lo  = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// absolute

void SNES_CPU::ABS16() {
	twobyte addr = mem->readROM16(K, PC);
	fetched = mem->read16(DBR, addr);
}

void SNES_CPU::ABS8() {
	twobyte addr = mem->readROM16(K, PC);
	*fetched_lo = mem->read8(DBR, addr);
}

void SNES_CPU::ABSL16() {
	threebyte addr = mem->readROM24(K, PC);
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSL8() {
	threebyte addr = mem->readROM24(K, PC);
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSX16() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSX8() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	addr += X;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSY16() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	addr += Y;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSY8() {
	threebyte addr = (twobyte)mem->readROM16(K, PC) + (DBR << 16);
	addr += Y;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLX16() {
	threebyte addr = mem->readROM24(K, PC);
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLX8() {
	threebyte addr = mem->readROM24(K, PC);
	addr += X;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLY16() {
	threebyte addr = mem->readROM24(K, PC);
	addr += Y;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::ABSLY8() {
	threebyte addr = mem->readROM24(K, PC);
	addr += Y;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

// stack relative

void SNES_CPU::SR16() {
	twobyte addr = (twobyte)mem->readROM8(K, PC) + S;
	fetched = mem->read16(0x00, addr);
}

void SNES_CPU::SR8() {
	twobyte addr = (twobyte)mem->readROM8(K, PC) + S;
	*fetched_lo = mem->read8(0x00, addr);
}

void SNES_CPU::SRIX16() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::SRIX8() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	addr += X;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::SRIY16() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	addr += X;
	fetched = mem->read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::SRIY8() {
	threebyte addr = ((twobyte)mem->readROM8(K, PC) + S) + (DBR << 16);
	addr += X;
	*fetched_lo = mem->read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

void SNES_CPU::setM() {
	ADC = bind_fn(ADC8);
	
	IMM = bind_fn(IMM8);
	
	DP = bind_fn(DP8);
	DPX = bind_fn(DPX8);
	DPY = bind_fn(DPY8);
	
	DPI = bind_fn(DPI8);
	DPIL = bind_fn(DPIL8);
	DPIX = bind_fn(DPIX8);
	DPIY = bind_fn(DPIY8);
	DPILX = bind_fn(DPILX8);
	DPILY = bind_fn(DPILY8);
	
	ABS = bind_fn(ABS8);
	ABSL = bind_fn(ABSL8);
	ABSX = bind_fn(ABSX8);
	ABSY = bind_fn(ABSY8);
	ABSLX = bind_fn(ABSLX8);
	ABSLY = bind_fn(ABSLY8);
}

void SNES_CPU::clearM() {
	ADC = bind_fn(ADC16);
	
	IMM = bind_fn(IMM16);
	
	DP = bind_fn(DP16);
	DPX = bind_fn(DPX16);
	DPY = bind_fn(DPY16);
	
	DPI = bind_fn(DPI16);
	DPIL = bind_fn(DPIL16);
	DPIX = bind_fn(DPIX16);
	DPIY = bind_fn(DPIY16);
	DPILX = bind_fn(DPILX16);
	DPILY = bind_fn(DPILY16);
	
	ABS = bind_fn(ABS16);
	ABSL = bind_fn(ABSL16);
	ABSX = bind_fn(ABSX16);
	ABSY = bind_fn(ABSY16);
	ABSLX = bind_fn(ABSLX16);
	ABSLY = bind_fn(ABSLY16);
}

void SNES_CPU::setI() {
	
}

void SNES_CPU::clearI() {
	
}