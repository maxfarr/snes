#ifndef _RAM_H
#define _RAM_H

#include "common.h"

#include <array>
#include <string>
#include "cpu.h"

// loROM implementation for now
// to do: turn into abstract class and implement multiple mappers
class SNES_MEMORY {
public:
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
	
	void openROM(std::string filename);
private:
	void apply_mirrors(byte& bank, twobyte addr);
	std::array<byte, 1024 * 64 * 256> data;
};

#endif
