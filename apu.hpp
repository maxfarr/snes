#ifndef _APU_H
#define _APU_H

#include "common.h"
#include "spc700.hpp"
#include "dsp.hpp"

class SNES_APU {
public:
    SNES_APU();
private:
    SPC700 cpu;
    SNES_DSP dsp;
};

#endif //_APU_H