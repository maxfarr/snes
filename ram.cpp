#include "common.h"

#include "ram.hpp"
#include "cpu.hpp"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>

void SNES_MEMORY::apply_mirrors(byte& bank, twobyte offset) {
	// mirror low RAM
	if(offset <= 0x1FFF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x7E;
	// mirror ROM
	if(offset >= 0x8000 && bank <= 0x7D) bank += 0x80;
	// mirror PPU registers
	if(offset >= 0x2100 && offset <= 0x21FF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x00;
	// CPU registers
	if(offset >= 0x4200 && offset <= 0x43FF && (bank <= 0x3F || (bank >= 0x80 && bank <= 0xBF))) bank = 0x00;
}

byte SNES_MEMORY::read(threebyte addr) {
	byte bank = (addr >> 16) & 0xFF;
	twobyte offset = addr & 0xFFFF;
	apply_mirrors(bank, offset);

	threebyte mirrored_addr = offset + (bank << 16);
	byte value = data[mirrored_addr];
#ifdef DEBUG_MEMORY
      std::cout << "read: read byte $" << std::hex << HEX_BYTE_PRINT(value) <<
      " at 0x" << mirrored_addr << " (requested 0x" << addr << ")" << std::dec << std::endl;
#endif
	return value;
}

void SNES_MEMORY::write(threebyte addr, byte entry) {
	byte bank = (addr >> 16) & 0xFF;
	twobyte offset = addr & 0xFFFF;
	apply_mirrors(bank, offset);

	threebyte mirrored_addr = offset + (bank << 16);
	data[mirrored_addr] = entry;
}

bool SNES_MEMORY::openROM(std::string filename) {
	std::memset(&data, 0, SNES_RAM_SIZE);

	std::ifstream f (filename);
	char c;
	byte bank = 0x80;
	twobyte addr = 0x8000;
	size_t count = 0;
	while(f.get(c)) {
		count++;
		threebyte final_addr = (bank << 16) | addr;
		data[final_addr] = c;
#ifdef DEBUG_ROM
		std::cout << "openROM: stored byte $" << std::hex << HEX_BYTE_PRINT(c)
		<< " at 0x" << final_addr << std::dec << std::endl;
#endif
		// detect overflow, jump to 0x8000 if needed
		if(addr == 0xFFFF) {
			if(bank == 0xFF) {
				std::cout << "openROM: ran out of memory, exiting" << std::endl;
				return false;
			}

			addr = 0x8000;
			bank++;
		} else {
			addr++;
		}
	}

	if(count == 0) {
		std::cout << "openROM: file empty" << std::endl;
		return false;
	}

	//m_reset_vector = read16_bank0(0xFFFC);
	return true;
}