#ifndef MEMORY_H
#define MEMORY_H

#include <types.h>

/**
 * This file contains function to allocate and modify memory
 */

/**
 * Allocates memory with the requested size of 0 on failure
 */
void *memory_aquire(size_t count);

/**
 * Frees memory the has been allocated
 */
void memory_release(void* address);

/**
 * Sets every byte in the memory buffer to a value
 */
static inline void *memory_set(void *destination, uint8_t value, size_t count) {
    register uint8_t *d = destination;
    register size_t num = count;

    while (num-- > 0)
        *d++ = value;

    return destination;
}

/**
 * Copies the context from 1 memory pointer into an other
 */
static inline void *memory_copy(void *destination, const void *source, size_t count) {
    register uint8_t *d = destination;
    register const uint8_t *s = source;
    register size_t num = count;

    while (num-- > 0)
        *d++ = *s++;

    return destination;
}

/**
 * Compare 2 memory segments up to a given length
 */
static inline int memory_compare(const void *a, const void *b, size_t count) {
    register const uint8_t *ptr1 = a;
    register const uint8_t *ptr2 = b;

    while (count-- > 0){
        if (*ptr1 != *ptr2)
            return *ptr1 - *ptr2;

        ptr1++;
        ptr2++;
    }

    return 0;
}

#endif