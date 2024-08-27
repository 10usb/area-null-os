#include "memory.h"
#include <stdint.h>

int mem_unlocked(){
    uint16_t *sector1 = (uint16_t*)0x7DFE;
    uint16_t *sector2 = (uint16_t*)0x107DFE;
    *sector2 = 0x0;
    *sector1 = 0xAA55;
    return *sector2 != *sector1;
}

void *memset(register void *dst, register uint8_t value, register size_t n) {
    register uint8_t *d = dst;

    while (n-- > 0)
        *d++ = value;

    return dst;
}

void *memcpy(void *dst, void *src, size_t num) {
    register uint8_t *d = dst;
    register uint8_t *s = src;

    while (num-- > 0)
        *d++ = *s++;

    return dst;
}

int memcmp(register void *ptr1, register void *ptr2, register size_t num) {
    register uint8_t *a = ptr1;
    register uint8_t *b = ptr2;

    while (num-- > 0){
        if (*a != *b)
            return *a - *b;

        a++;
        b++;
    }

    return 0;
}