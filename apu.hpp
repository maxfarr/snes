#ifndef _APU_H
#define _APU_H

#include "common.h"

#include "spc700.hpp"
#include "dsp.hpp"
#include "cpu_apu_io.hpp"

class SNES_APU {
public:
    SNES_APU(CPU_APU_IO* cpu_io);
    bool clock();
private:
    CPU_APU_IO* cpu_io;
    SPC700 cpu;
    SNES_DSP dsp;
};

#endif //_APU_H