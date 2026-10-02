#include "common.h"
#include "flat_memory.hpp"

#include <unordered_map>

byte FLAT_MEMORY::read(threebyte addr) {
    auto it = data.find(addr);
    return it == data.end() ? 0 : it->second;
}

void FLAT_MEMORY::write(threebyte addr, byte entry) {
    data[addr] = entry;
}