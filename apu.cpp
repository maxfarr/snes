#include "apu.hpp"
#include "cpu_apu_io.hpp"

SNES_APU::SNES_APU(CPU_APU_IO* cpu_io) : cpu_io(cpu_io) {
    
}

bool SNES_APU::clock() {
    return cpu.clock();
}