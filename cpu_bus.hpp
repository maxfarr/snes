#ifndef _CPU_BUS_H
#define _CPU_BUS_H

#include "common.h"

class CPU_BUS {
public:
    virtual byte read(threebyte addr) = 0;
    virtual void write(threebyte addr, byte entry) = 0;

    virtual ~CPU_BUS() = default;
};

#endif // _CPU_BUS_H