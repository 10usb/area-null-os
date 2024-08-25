#include "tty.h"
#include "text.h"
#include "interrupts.h"
#include "memory.h"

char buffer[30];

void myhandler(isr_frame_t *frame) {
    tty_color_t current = tty_getcolor();

    tty_setcolors(TTY_YELLOW, TTY_BLACK);
    tty_puts("Hello world, from my myhandler\n");
    
    tty_setcolor(current);
}

void main(){
    tty_init(80, 25, (void*)0xb8000);
    tty_clear();

    tty_setcolors(TTY_WHITE, TTY_BLACK);
    tty_puts(" AreaNull OS ");

    tty_setcolors(TTY_BLACK, TTY_WHITE);
    tty_put('\n');

    tty_fixed_header(1);

    if(!mem_unlocked()) {
        tty_setcolors(TTY_RED, TTY_WHITE);
        tty_puts("A20 gate hasn't been unlocked\n");
        return;
    }

    tty_puts("A20 gate has been unlocked\n");


    isr_init();
    irq_init();
    

    isr_install(0x25, myhandler);
    __asm__ volatile ("int %0" : : "i"(0x25));
}