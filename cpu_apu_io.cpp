#include "cpu_apu_io.hpp"

void CPU_APU_IO::writeAPU(size_t port, byte data) {
    ports[port][0] = data;
}

byte CPU_APU_IO::readAPU(size_t port) {
    return ports[port][1];
}

void CPU_APU_IO::writeCPU(size_t port, byte data) {
    ports[port][1] = data;
}

byte CPU_APU_IO::readCPU(size_t port) {
    return ports[port][0];
}