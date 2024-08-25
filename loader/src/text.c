#include "text.h"

int strlen(const char *text) {
    register const char *ptr = text;
    while (*ptr)
        ptr++;
    return ptr - text;
}

int strncpy(char *destination, const char *source, size_t size) {
    register char *dst = destination;
    register const char *end = destination + size - 1;
    register const char *src = source;
    
    while (dst < end && *src)
        *dst++ = *src++;

    *dst = 0;

    return dst - destination;
}

const char *itos(int value, char *buffer, size_t size, int base) {
    int negative = value < 0;
    register char *ptr = buffer + size;
    *ptr = 0;

    do
    {
        //*--ptr = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        *--ptr = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
        value = value / base;
    } while (value);

    if (negative)
        *--ptr = '-';

    return ptr;
}

int stoi(const char *src) {
    return 0;
}
