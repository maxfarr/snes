#include "common.h"

#include "ram.hpp"
#include "cpu.hpp"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>

void SNES_MEMORY::apply_mirrors(byte& bank, twobyte addr) {
	// mirror low RAM
	if(addr <= 0x1FFF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x7E;
	// mirror ROM
	if(addr >= 0x8000 && bank <= 0x7D) bank += 0x80;
	// mirror PPU registers
	if(addr >= 0x2100 && addr <= 0x21FF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x00;
	// CPU registers
	if(addr >= 0x4200 && addr <= 0x43FF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x00;
}

// todo: rename "addr" either in these functions or down in the readROM functions
byte SNES_MEMORY::read8(byte bank, twobyte addr) {
	apply_mirrors(bank, addr);
	
	byte value = data[addr + (bank << 16)];
#ifdef DEBUG_MEMORY
	std::cout << "read8: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
	" at 0x" << (addr + (bank << 16)) << std::dec << std::endl;
#endif
	return value;
}

byte SNES_MEMORY::read8(threebyte addr) {
	return read8((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

twobyte SNES_MEMORY::read16(byte bank, twobyte addr) {
	apply_mirrors(bank, addr);
	
	threebyte full_addr = addr + (bank << 16);

	twobyte value = (twobyte)data[full_addr] | (data[full_addr + 1] << 8);
#ifdef DEBUG_MEMORY
	std::cout << "read16: read twobyte $" << std::hex << value <<
	" at 0x" << full_addr << std::dec << std::endl;
#endif
	return value;
}

twobyte SNES_MEMORY::read16(threebyte addr) {
	return read16((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

threebyte SNES_MEMORY::read24(byte bank, twobyte addr) {
	apply_mirrors(bank, addr);

	threebyte full_addr = addr + (bank << 16);
	
	threebyte value = (threebyte)data[full_addr];
	value |= (data[full_addr + 1] << 8);
	value |= (data[full_addr + 2] << 16);
#ifdef DEBUG_MEMORY
	std::cout << "read24: read threebyte $" << std::hex << value <<
	" at 0x" << full_addr << std::dec << std::endl;
#endif
	return value;
}

threebyte SNES_MEMORY::read24(threebyte addr) {
	return read24((addr & 0xFF0000) >> 16, addr & 0xFFFF);
}

byte SNES_MEMORY::read8_bank0(twobyte addr) {
	byte bank = 0x00;
	apply_mirrors(bank, addr);

	byte value = data[addr + (bank << 16)];
#ifdef DEBUG_MEMORY
	std::cout << "read8_bank0: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
	" at 0x00" << addr << std::dec << std::endl;
#endif
	return value;
}

twobyte SNES_MEMORY::read16_bank0(twobyte addr) {
	byte bank = 0x00;
	apply_mirrors(bank, addr);
	
	threebyte lo_addr = addr + (bank << 16);
	threebyte hi_addr = (addr + 1) + (bank << 16);
	
	twobyte value = (twobyte)data[lo_addr] | (data[hi_addr] << 8);
#ifdef DEBUG_MEMORY
	std::cout << "read16_bank0: read twobyte $" << std::hex << value <<
	" at 0x00" << addr << std::dec << std::endl;
#endif
	return value;
}

threebyte SNES_MEMORY::read24_bank0(twobyte addr) {
	byte bank = 0x00;
	apply_mirrors(bank, addr);

	threebyte lo_addr = addr + (bank << 16);
	threebyte mid_addr = (addr + 1) + (bank << 16);
	threebyte hi_addr = (addr + 2) + (bank << 16);
	
	threebyte value = (threebyte)data[lo_addr];
	value |= (data[mid_addr] << 8);
	value |= (data[hi_addr] << 16);
#ifdef DEBUG_MEMORY
	std::cout << "read24_bank0: read threebyte $" << std::hex << value <<
	" at 0x00" << addr << std::dec << std::endl;
#endif
	return value;
}

byte SNES_MEMORY::readROM8(byte K, twobyte& PC) {
	apply_mirrors(K, PC);
	threebyte addr = PC | (K << 16);
	
	byte value = data[addr];
#ifdef DEBUG_MEMORY
	std::cout << "readROM8: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

twobyte SNES_MEMORY::readROM16(byte K, twobyte& PC) {
	apply_mirrors(K, PC);
	threebyte addr = PC | (K << 16);
	
	twobyte value = (twobyte)data[addr];
	PC++;
	addr = PC | (K << 16);
	value |= (data[addr] << 8);
#ifdef DEBUG_MEMORY
	std::cout << "readROM16: read twobyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

threebyte SNES_MEMORY::readROM24(byte K, twobyte& PC) {
	apply_mirrors(K, PC);
	threebyte addr = PC | (K << 16);
	
	threebyte value = (threebyte)data[addr];
	PC++;
	addr = PC | (K << 16);
	value |= (data[addr] << 8);
	PC++;
	addr = PC | (K << 16);
	value |= (data[addr] << 16);
#ifdef DEBUG_MEMORY
	std::cout << "readROM24: read threebyte $" << std::hex << value <<
	" at 0x" << addr << std::dec << std::endl;
#endif
	PC++;
	return value;
}

void SNES_MEMORY::write8(byte bank, twobyte addr, byte entry) {
	apply_mirrors(bank, addr);

	threebyte complete_addr = addr + (bank << 16);

	data[complete_addr] = entry;
#ifdef DEBUG_MEMORY
	std::cout << "write8: wrote byte $" << std::hex << HEX_BYTE_PRINT(entry) <<
	" to 0x" << complete_addr << std::dec << std::endl;
#endif
}

void SNES_MEMORY::write16(byte bank, twobyte addr, twobyte entry, bool wrap) {
	apply_mirrors(bank, addr);
	
	threebyte complete_addr = (threebyte)addr + (bank << 16);

	data[complete_addr] = (byte)(entry & 0x00FF);
	data[complete_addr+1] = (byte)((entry & 0xFF00) >> 8);
#ifdef DEBUG_MEMORY
	std::cout << "write16: wrote twobyte $" << std::hex << entry <<
	" to 0x" << std::setw(6) << complete_addr << std::dec << std::endl;
#endif
}

twobyte SNES_MEMORY::brk_vector() {
	return read16_bank0(0xFFE6);
}

twobyte SNES_MEMORY::cop_vector() {
	return read16_bank0(0xFFE4);
}

twobyte SNES_MEMORY::reset_vector() {
	return m_reset_vector;
}

void SNES_MEMORY::openROM(std::string filename) {
	std::memset(&data, 0, SNES_RAM_SIZE);

	std::ifstream f (filename);
	char c;
	byte bank = 0x80;
	twobyte addr = 0x8000;
	while(f.get(c)) {
		threebyte final_addr = (bank << 16) | addr;
		data[final_addr] = c;
#ifdef DEBUG_MEMORY
		std::cout << "openROM: stored byte $" << std::hex << HEX_BYTE_PRINT(c)
		<< " at 0x" << final_addr << std::dec << std::endl;
#endif
		// detect overflow, jump to 0x8000 if needed
		if(addr == 0xFFFF) {
			if(bank == 0xFF) {
				std::cout << "openROM: ran out of memory, exiting" << std::endl;
				return;
			}

			addr = 0x8000;
			bank++;
		} else {
			addr++;
		}
	}

	m_reset_vector = read16_bank0(0xFFFC);
}