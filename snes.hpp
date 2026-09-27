#ifndef _SNES_H
#define _SNES_H

#include "common.h"

#include "cpu.hpp"
#include "apu.hpp"
#include "cpu_apu_io.hpp"
#include "ram.hpp"

#include <memory>

class SNES {
public:
    SNES();
    ~SNES();

    void run();
private:
    CPU_APU_IO cpu_apu_io;
    std::unique_ptr<SNES_CPU> cpu;
    std::unique_ptr<SNES_APU> apu;
    std::unique_ptr<SNES_MEMORY> mem;
    
    bool ready;
};

#endif //_SNES_H