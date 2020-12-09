#ifndef _RAM_H
#define _RAM_H

#include <array>
#include "cpu.h"

class SNES_RAM {
public:
	byte read8(twobyte addr);
	twobyte read16(twobyte addr);
private:
	std::array<byte, 1024 * 128> data;
};

#endif
