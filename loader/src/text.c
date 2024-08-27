#include "text.h"
#include <stdarg.h>

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
    register char *ptr = buffer + size - 1;
    *ptr = 0;

    do
    {
        //*--ptr = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        *--ptr = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
        value = value / base;
    } while (value && --size);

    if (negative)
        *--ptr = '-';

    return ptr;
}

int stoi(const char *src) {
    return 0;
}

void snprintf(char *dst, size_t num, const char *format, ...){
    register char *ptr = dst;
    register char c;
    char buffer[33];
    char specifier;
    char padding;
    int width;
    const char *str;
    int length;
    int integer;
    
	va_list ap;
	va_start(ap, format);

    if(num == 0)
        return;

    while (c = *format++) {
        if (c != '%') {
            if(--num == 0) goto limit_reached;
            *ptr++ = c;
            continue;
        }

        if((c = *format++) == 0) goto limit_reached;

        if(c == '0') {
            padding = c;
            if((c = *format++) == 0) goto limit_reached;
        } else {
            padding = ' ';
        }

        width = 0;
        while (c >= '0' && c <= '9'){
            width = width*10 + (c-'0');
            if((c = *format++) == 0) goto limit_reached;
        }

        specifier = c;
        
        switch (specifier) {
            case 'b':
                integer = va_arg(ap, int);
                str = itos(integer, buffer, 33, 2);
                goto print;
            case 'd':
                integer = va_arg(ap, int);
                str = itos(integer, buffer, 33, 10);
                goto print;
            case 'o':
                integer = va_arg(ap, int);
                str = itos(integer, buffer, 33, 8);
                goto print;
            case 'x':
                integer = va_arg(ap, int);
                str = itos(integer, buffer, 33, 16);
                goto print;
            case 's':
                str = va_arg(ap, char*);
            print:
                length = strlen(str);
                width -= length;

                while (width-- > 0){
                    if(--num == 0) goto limit_reached;
                    *ptr++ = padding;
                }

                while (*str){
                    if(--num == 0) goto limit_reached;
                    *ptr++ = *str++;
                }
            break;
            default:
                if(--num == 0) goto limit_reached;
                *ptr++ = c;
                
                if(--num == 0) goto limit_reached;
                *ptr++ = specifier;
            break;
        }
    }

    limit_reached:
    *ptr = 0;

	va_end(ap);
}
