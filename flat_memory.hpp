#ifndef _FLAT_MEMORY_H
#define _FLAT_MEMORY_H

#include "common.h"
#include "cpu_bus.hpp"

#include <unordered_map>

class FLAT_MEMORY : public CPU_BUS {
public:
    byte read(threebyte addr) override;
    void write(threebyte addr, byte entry) override;

    ~FLAT_MEMORY() override = default;

    void clear() {
        data.clear();
    }
private:
    std::unordered_map<threebyte, byte> data;
};

#endif // _FLAT_MEMORY_H