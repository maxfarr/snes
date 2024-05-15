#ifndef _CPU_APU_IO_H
#define _CPU_APU_IO_H

#include "common.h"
#include <iostream>

class CPU_APU_IO {
public:
    void writeAPU(size_t port, byte data);
    byte readAPU(size_t port);

    void writeCPU(size_t port, byte data);
    byte readCPU(size_t port);

private:
    // in the following ports,
    // byte 0 is APU -> CPU (APU writes, CPU reads)
    // byte 1 is CPU -> APU (CPU writes, APU reads)
    byte ports[4][2];
};

#endif // _CPU_APU_IO_H