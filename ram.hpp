#ifndef _RAM_H
#define _RAM_H

#include "common.h"
#include "cpu_apu_io.hpp"
#include "cpu_bus.hpp"

#include <array>
#include <string>
#include <iostream>

class SNES_MEMORY: public CPU_BUS {
public:
	SNES_MEMORY(CPU_APU_IO* apu_io) : apu_io(apu_io) {};

	byte read(threebyte addr) override;
	void write(threebyte addr, byte entry) override;
	
	bool openROM(std::string filename);
private:
	CPU_APU_IO* apu_io;
	void apply_mirrors(byte& bank, twobyte addr);

	std::array<byte, SNES_RAM_SIZE> data;
};

#endif //_RAM_H