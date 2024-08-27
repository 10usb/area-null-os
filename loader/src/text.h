#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include <stddef.h>

int strlen(const char *text);
int strncpy(char *destination, const char *source, size_t size);
const char *itos(int value, char *buffer, size_t size, int base);
int stoi(const char *src);

void snprintf(char *dst, size_t num, const char * format, ...);

#endif