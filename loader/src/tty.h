#ifndef TTY_H
#define TTY_H

#include <stdint.h>

#define TTY_BLACK        0x0
#define TTY_BLUE         0x1
#define TTY_GREEN        0x2
#define TTY_CYAN         0x3
#define TTY_RED          0x4
#define TTY_PURPLE       0x5
#define TTY_BROWN        0x6
#define TTY_GRAY         0x7
#define TTY_DARKGRAY     0x8
#define TTY_LIGHT_BLUE   0x9
#define TTY_LIGHT_GREEN  0xA
#define TTY_LIGHT_CYAAN  0xB
#define TTY_LIGHT_RED    0xC
#define TTY_LIGHT_PURPLE 0xD
#define TTY_YELLOW       0xE
#define TTY_WHITE        0xF

typedef struct {
    int foreground : 4;
    int background : 4;
} __attribute__((packed)) tty_color_t;

typedef struct {
    char value;
    tty_color_t color;
} __attribute__((packed)) ttychar_t;

void tty_init(int width, int height, void *address);
void tty_setcolors(int background, int foreground);
void tty_setcolor(tty_color_t color);
tty_color_t tty_getcolor();
void tty_setpos(int row, int column);
void tty_fixed_header(int lines);

void tty_clear();
void tty_put(char c);
void tty_puts(const char *str);
void tty_scroll(int lines, int start);

typedef enum {
    TTY_LEFT,
    TTY_CENTER,
    TTY_RIGHT
} tty_align_t;

void tty_print(int row, int column, int width, const char *str, int background, int foreground, tty_align_t align);

#endif