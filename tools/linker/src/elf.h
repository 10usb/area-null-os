#ifndef ELF_H
#define ELF_H

#include <exec/elf.h>

/**
 * Get the last error
 * 
 * @return 
 */
const char *elf_last_error();

/**
 * Print all values in the context
 * 
 * @param context 
 */
void elf_print(struct ELFContext *context);

/**
 * Opens a file 
 * 
 * @param path 
 * @return 
 */
struct ELFContext *elf_open(const char *path);

/**
 * Frees up the context
 * 
 * @param context 
 */
void elf_close(struct ELFContext *context);

/**
 * Save the contents to a file
 * 
 * @param context 
 * @param path 
 * @return 
 */
int elf_save(struct ELFContext *context, const char *path);

/**
 * Relocates the progbits
 * 
 * @param context 
 * @param origin 
 * @param alignment 
 * @return 
 */
int elf_relocate(struct ELFContext *context, uint32_t origin, uint32_t alignment);

#endif