#include "common.h"
#include "flat_memory.hpp"

byte FLAT_MEMORY::read(threebyte addr) {
    return data[addr];
}

void FLAT_MEMORY::write(threebyte addr, byte entry) {
    data[addr] = entry;
}