#ifndef _SPC700_H
#define _SPC700_H

#include "common.h"
#include "aram.hpp"

class SPC700 {
private:
    byte A;
    byte X;
    byte Y;
    byte SP;
    byte PC;
    byte PSW;

    SNES_ARAM ram;
};

#endif //_SPC700_H