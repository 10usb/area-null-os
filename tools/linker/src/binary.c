#include "binary.h"

int binary_save_elf(struct ELFContext *context, const char *path) {
    // - Check if context has programm headers so we know has functional code
    // - Of the LOAD segments find the lowest address
    // - Write contents of segement to offset address - lowest address
    return 0;
}