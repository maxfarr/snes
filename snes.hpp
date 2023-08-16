#ifndef _SNES_H
#define _SNES_H

#include "common.h"

#include "cpu.hpp"
#include "ppu.hpp"

class SNES {
public:
    SNES();
    ~SNES();

    void run();
private:
    SNES_CPU cpu;
    SNES_PPU ppu;
};

#endif //_SNES_H