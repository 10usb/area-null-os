#include "tty.h"
#include "text.h"

static struct {
    tty_color_t color;
    uint32_t width;
    uint32_t height;
    ttychar_t *start;
    ttychar_t *end;
    ttychar_t *cursor;
    uint32_t fixed;
} screen;

void tty_init(int width, int height, void *address) {
    screen.width = width;
    screen.height = height;
    screen.start = address;
    screen.end = screen.start + screen.width * screen.height;
    screen.cursor = address;
    screen.color.foreground = TTY_WHITE;
    screen.color.background = TTY_BLACK;
}

void tty_setcolors(int background, int foreground) {
    screen.color.background = background;
    screen.color.foreground = foreground;
}

void tty_setcolor(tty_color_t color) {
    screen.color = color;
}

tty_color_t tty_getcolor() {
    return screen.color;
}

void tty_setpos(int row, int column) {
    screen.cursor = screen.start + row * screen.width + column;
}

void tty_fixed_header(int lines) {
    screen.fixed = lines;
}

void tty_clear() {
    register ttychar_t *ptr = screen.start;
    register ttychar_t *end = screen.end;
    register ttychar_t blank = {
        .color = screen.color,
        .value = ' '
    };
    
    while (ptr < end)
        *ptr++ = blank;
}

void tty_put(char c) {
    // Before we can print anything to the screen we need
    // to make sure there is an empty line
    if(screen.cursor >= screen.end)
        tty_scroll(1, screen.fixed);

    // If we encounter a new line, fill the rest of the line
    if (c == '\n') {
        uint32_t remain = screen.width - (screen.cursor - screen.start) % screen.width;

        while (remain > 0) {
            screen.cursor->value = ' ';
            screen.cursor->color = screen.color;
            screen.cursor++;
            remain--;
        }
    } else {
        screen.cursor->value = c;
        screen.cursor->color = screen.color;
        screen.cursor++;
    }
}

void tty_puts(const char *str) {
    while (*str)
        tty_put(*str++);
}

void tty_scroll(int lines, int start) {
    register ttychar_t *dst = screen.start + (screen.width * start);
    register ttychar_t *src = dst + (screen.width * lines);
    register ttychar_t blank = {
        .color = screen.color,
        .value = ' '
    };

    while(src < screen.end)
        *dst++ = *src++;

    while (dst < screen.end)
        *dst++ = blank;

    screen.cursor-= screen.width * lines;
    if(screen.cursor < screen.start)
        screen.cursor = screen.start;
}

void tty_print(int row, int column, int width, const char *str, int background, int foreground, tty_align_t align) {
    int length = strlen(str);

    int padding;
    if (length >= width || align == TTY_LEFT) {
        padding = 0;
    } else switch (align) {
        case TTY_CENTER: padding = (width - length) / 2; break;
        case TTY_RIGHT: padding = width - length; break;
    }

    register ttychar_t *dst = screen.start + (screen.width * row) + column;
    register ttychar_t blank = {
        .color.background = background,
        .color.foreground = foreground,
        .value = ' '
    };

    while (padding--) {
        *dst++ = blank;
        width--;
    }

    while (*str) {
        dst->value = *str++;
        dst->color.background = background;
        dst->color.foreground = foreground;
        dst++;
        width--;
    }
    
    while (width--)
        *dst++ = blank;
}
