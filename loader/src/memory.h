#ifndef MEM_H
#define MEM_H


#include <stdint.h>
#include <stddef.h>

int mem_unlocked();

void *memset(void *dst, uint8_t value, size_t num);

void *memcpy(void *dst, void *src, size_t num);
int memcmp(void *ptr1, void *ptr2, size_t num);

#endif