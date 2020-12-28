#include "common.h"

#include "ram.h"
#include "cpu.h"
#include <iostream>
#include <fstream>

byte SNES_MEMORY::read8(byte bank, twobyte addr) {
	if(addr <= 0x1FFF && (bank <= 0x3F || bank >= 0x80)) bank = 0x7E;
	
	return data[addr + (bank << 16)];
}

twobyte SNES_MEMORY::read16(byte bank, twobyte addr) {
	if(addr <= 0x1FFF && bank <= 0x3F) bank = 0x7E;
	
	threebyte complete_addr = addr + (bank << 16);
	
	byte low = data[complete_addr++];
	return low + (data[complete_addr] << 8);
}

threebyte SNES_MEMORY::read24(byte bank, twobyte addr) {
	if(addr <= 0x1FFF && bank <= 0x3F) bank = 0x7E;
	
	threebyte complete_addr = addr + (bank << 16);
	
	threebyte value = (threebyte)data[complete_addr++];
	value |= (data[complete_addr++] << 8);
	return value |= (data[complete_addr] << 16);
}

byte SNES_MEMORY::readROM8(byte& K, twobyte& PC) {
	threebyte complete_addr = PC + (K << 16);
	
	byte value = data[complete_addr];
	advancePC(K, PC);
	return value;
}

twobyte SNES_MEMORY::readROM16(byte& K, twobyte& PC) {
	threebyte complete_addr = PC + (K << 16);
	
	twobyte value = (twobyte)data[complete_addr++];
	if(advancePC(K, PC))
		complete_addr = PC + (K << 16);
	value |= (data[complete_addr] << 8);
	advancePC(K, PC);
	return value;
}

threebyte SNES_MEMORY::readROM24(byte& K, twobyte& PC) {
	threebyte complete_addr = PC + (K << 16);
	
	threebyte value = (threebyte)data[complete_addr++];
	if(advancePC(K, PC))
		complete_addr = PC + (K << 16);
	value |= (data[complete_addr++] << 8);
	if(advancePC(K, PC))
		complete_addr = PC + (K << 16);
	value |= (data[complete_addr] << 16);
	advancePC(K, PC);
	return value;
}

void SNES_MEMORY::write8(byte bank, twobyte addr, byte entry) {
	if(addr <= 0x1FFF && bank <= 0x3F) bank = 0x7E;
	
	threebyte complete_addr = addr + (bank << 16);
	
	if(complete_addr >= 0x7E0000 && complete_addr <= 0x7FFFF) {
		data[complete_addr] = entry;
	}
	
	// throw an error
}

void SNES_MEMORY::write16(byte bank, twobyte addr, twobyte entry) {
	if(addr <= 0x1FFF && bank <= 0x3F) bank = 0x7E;
	
	threebyte complete_addr = addr + (bank << 16);
	
	if(complete_addr >= 0x7E0000 && complete_addr <= 0x7FFFF) {
		data[complete_addr++] = (byte)(entry & 0x00FF);
		data[complete_addr] = (byte)((entry & 0xFF00) >> 8);
	}
	
	// throw an error
}

bool SNES_MEMORY::advancePC(byte& K, twobyte& PC) {
	bool pc_high_bit = getBit(PC, 15);
	PC++;
	
	// detect overflow, increment bank if needed
	if(pc_high_bit && !getBit(PC, 15)) {
		K++;
		return true;
	}
	
	return false;
}

void SNES_MEMORY::branchPC(byte& K, twobyte& PC, byte n) {
	threebyte PC_full = PC + (K << 16);
	PC_full += (signed char)n;
	
	K = (PC_full & 0xFF0000) >> 16;
	PC = PC_full & 0xFFFF;
}

void SNES_MEMORY::openROM(std::string filename) {
	std::ifstream f (filename);
	char c;
	threebyte addr = 0x008000;
	while(f.get(c)) {
		data[addr++] = c;
		// detect overflow, jump to 0x8000 if needed
		if(!(addr & 0xFFFF)) addr |= 0x8000;
	}
}