#include "memory.h"
#include <stdint.h>

int mem_unlocked(){
    uint16_t *sector1 = (uint16_t*)0x7DFE;
    uint16_t *sector2 = (uint16_t*)0x107DFE;
    *sector2 = 0x0;
    *sector1 = 0xAA55;
    return *sector2 != *sector1;
}