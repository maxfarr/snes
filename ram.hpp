#ifndef _RAM_H
#define _RAM_H

#include "common.h"
#include "cpu_apu_io.hpp"

#include <array>
#include <string>
#include <iostream>

// loROM implementation for now
// to do: turn into abstract class and implement multiple mappers
class SNES_MEMORY {
public:
	SNES_MEMORY(CPU_APU_IO* apu_io) : apu_io(apu_io) {};

	byte read8(byte bank, twobyte addr);
	byte read8(threebyte addr);
	byte read8_bank0(twobyte addr);

	twobyte read16(byte bank, twobyte addr);
	twobyte read16(threebyte addr);
	twobyte read16_bank0(twobyte addr);

	threebyte read24(byte bank, twobyte addr);
	threebyte read24(threebyte addr);
	threebyte read24_bank0(twobyte addr);
	
	byte readROM8(byte K, twobyte& PC);
	twobyte readROM16(byte K, twobyte& PC);
	threebyte readROM24(byte K, twobyte& PC);
	
	void write8(byte bank, twobyte addr, byte entry);
	void write16(byte bank, twobyte addr, twobyte entry, bool wrap = false);

	twobyte brk_vector();
	twobyte cop_vector();
	twobyte reset_vector();

	void override_reset_vector(twobyte addr) {m_reset_vector = addr; std::cout<<"hi: "<<addr<<std::endl;};
	
	bool openROM(std::string filename);
private:
	CPU_APU_IO* apu_io;
	void apply_mirrors(byte& bank, twobyte addr);

	std::array<byte, SNES_RAM_SIZE> data;
	twobyte m_reset_vector;
};

#endif //_RAM_H