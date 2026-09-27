#ifndef _FLAT_MEMORY_H
#define _FLAT_MEMORY_H

#include "common.h"
#include "cpu_bus.hpp"

#include <array>

class FLAT_MEMORY : public CPU_BUS {
public:
    byte read(threebyte addr) override;
    void write(threebyte addr, byte entry) override;

    ~FLAT_MEMORY() override = default;

private:
    std::array<byte, SNES_RAM_SIZE> data;
};

#endif // _FLAT_MEMORY_H