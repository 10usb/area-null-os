#ifndef ELF_H
#define ELF_H

#include <exec/elf.h>

const char *elflasterror();

void elfprint(struct ELFContext *context);

struct ELFContext *elfopen(const char *path);

void elfclose(struct ELFContext *context);

#endif