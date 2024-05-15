#ifndef _SNES_H
#define _SNES_H

#include "common.h"

#include "cpu.hpp"
#include "apu.hpp"
#include "cpu_apu_io.hpp"

class SNES {
public:
    SNES();
    ~SNES();

    void run();
private:
    CPU_APU_IO cpu_apu_io;
    SNES_CPU cpu;
    SNES_APU apu;
    
    bool ready;
};

#endif //_SNES_H