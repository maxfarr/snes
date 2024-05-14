#ifndef _ARAM_H
#define _ARAM_H

#include "common.h"

#include <array>

class SNES_ARAM {
public:
    SNES_ARAM();
private:
    std::array<byte, SNES_ARAM_SIZE> data;
};

#endif //_ARAM_H