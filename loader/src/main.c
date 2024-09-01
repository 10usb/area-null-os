#include "tty.h"
#include "text.h"
#include "interrupts.h"
#include "memory.h"
#include "rtc.h"
#include <driver/floppy.h>


static char buffer[50];

void myhandler(isr_frame_t *frame) {
    tty_color_t current = tty_getcolor();

    tty_setcolors(TTY_YELLOW, TTY_BLACK);
    tty_puts("Hello world, from my myhandler\n");
    
    tty_setcolor(current);
}

int count = 0;

static char timer_buffer[30];

void timer(int index, isr_frame_t *frame) {
    if((count++ % 10) == 0){
        time_t time;
        rtc_get(&time);
        snprintf(timer_buffer, 30, "%02d-%02d-%02d %02d:%02d:%02d", time.day, time.month, time.year, time.hour, time.minute, time.second);
        tty_print(0, 61, 19, timer_buffer, TTY_WHITE, TTY_BLACK, TTY_CENTER);
    }
}

static inline uint8_t inb(uint16_t port){
    uint8_t returnVal;
    asm volatile ("inb %1, %0": "=a"(returnVal) : "Nd"(port));
    return returnVal;
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
    
    irq_install(0, timer);
    //irg_enable(0, 1);

    isr_install(0x25, myhandler);
    __asm__ volatile ("int %0" : : "i"(0x25));


    floppy_init();

    struct BlockDevice *fd = (void*)0x100000;
    
    if(!floppy_get_device(0, fd)) {
        tty_puts("Failed to load floppy drive");
    }

    uint8_t *address = (void*)(0x100000 + 100);
    address[510] = 0x33;
    address[511] = 0x44;
    snprintf(buffer, 30, "%02x %02x\n", address[510], address[511]);
    tty_puts(buffer);

    uint32_t read = fd->read(fd, 0, 1, address);

    // We should see 0x55 0xAA
    snprintf(buffer, 30, "%02x %02x\n", address[510], address[511]);
    tty_puts(buffer);
}