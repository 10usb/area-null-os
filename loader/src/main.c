#include "tty.h"
#include "text.h"
#include "interrupts.h"
#include "memory.h"
#include "rtc.h"
#include <driver/floppy.h>
#include <fs/fat/readonly.h>

#define unused __attribute__ ((unused))

static char buffer[50];

void myhandler(unused isr_frame_t *frame) {
    tty_color_t current = tty_getcolor();

    tty_setcolors(TTY_YELLOW, TTY_BLACK);
    tty_puts("Hello world, from myhandler\n");
    
    tty_setcolor(current);
}

int count = 0;

static char timer_buffer[30];

void timer(unused int index, unused isr_frame_t *frame) {
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

    struct BlockDevice *device = (void*)0x100000;
    
    if(!floppy_get_device(0, device)) {
        tty_puts("Failed to load floppy drive");
    }

    struct FATContext *ctx =  (void*)0x200000;
    int resultCode;
    if((resultCode = fat_init_context(ctx, 0x100000, device)) != FAT_SUCCESS){
        snprintf(buffer, 30, "resultCode: %d\n", resultCode);
        tty_puts(buffer);
        return;
    }

    snprintf(buffer, 50, "Bytes per sector           %8d\n", ctx->header->bytesPerSector);tty_puts(buffer);
    snprintf(buffer, 50, "Sectors per track          %8d\n", ctx->header->sectorsPerTrack);tty_puts(buffer);
    snprintf(buffer, 50, "Number of heads            %8d\n", ctx->header->numberOfHeads);tty_puts(buffer);
    snprintf(buffer, 50, "Media descriptor           %8x\n", ctx->header->mediaDescriptor);tty_puts(buffer);
    snprintf(buffer, 50, "First sector               %8d\n", ctx->header->hiddenSectors);tty_puts(buffer);
    snprintf(buffer, 50, "Number of sectors          %8d\n", ctx->header->smallNumberOfSectors ? ctx->header->smallNumberOfSectors : ctx->header->largeNumberOfSectors);tty_puts(buffer);
    snprintf(buffer, 50, "Reserved sectors           %8d\n", ctx->header->reservedSectors);tty_puts(buffer);
    snprintf(buffer, 50, "Sectors per FAT            %8d\n", ctx->header->sectorsPerFat);tty_puts(buffer);
    snprintf(buffer, 50, "Fat copies                 %8d\n", ctx->header->numberOfFatCopies);tty_puts(buffer);
    snprintf(buffer, 50, "Sectors per cluster        %8d\n", ctx->header->sectorsPerCluster);tty_puts(buffer);
    snprintf(buffer, 50, "Root entries               %8d\n", ctx->header->numberOfRootEntries);tty_puts(buffer);

    uint32_t value;
    value = fat_next_cluster(ctx, 0);
    snprintf(buffer, 50, "FAT0: %3x\n", value);
    tty_puts(buffer);

    value = fat_next_cluster(ctx, 1);
    snprintf(buffer, 50, "FAT1: %3x\n", value);
    tty_puts(buffer);

    struct FATDirectoryEntry directory;
    int32_t count = fat_find_file(ctx, &directory, 1, "");
    if (count <= 0) {
        tty_puts("Directory not found\n");
    }
    snprintf(buffer, 50, "Found %d entries\n", count);
    tty_puts(buffer);
}