#ifndef BINARY_H
#define BINARY_H

#include "elf.h"

/**
 * Save the contents to a file in flat binary format
 * 
 * @param context 
 * @param path 
 * @return 
 */
int binary_save_elf(struct ELFContext *context, const char *path);

#endif