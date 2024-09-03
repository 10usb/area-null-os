#ifndef MEMORY_H
#define MEMORY_H

#include <types.h>

/**
 * This file contains function to allocate and modify memory
 */

/**
 * Allocates memory with the requested size of 0 on failure
 */
void * memory_alloc(int);

/**
 * Frees memory the has been allocated
 */
void memory_free(void*);

/**
 * Sets every byte in the memory buffer to a value
 */
void memory_set(void*, char, int);

/**
 * Copies the context from 1 memory pointer into an other
 */
void memory_copy(void*, const void*, int);

/**
 * Compare 2 memory segments up to a given length
 */
int memory_compare(void *, void*, size_t);

#endif