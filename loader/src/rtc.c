#include "rtc.h"
#include "memory.h"
#include "tty.h"
#include "text.h"

static inline void outb(uint16_t port, uint8_t val){
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port){
    uint8_t returnVal;
    asm volatile ("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static inline void rtc_fetch(register time_t *time){
    // Wait till busy flag is gone
    // do {
    //   outb(CMOS_ADDR, 0x0A);
    // } while (inb(CMOS_DATA) & 0x80);

    outb(CMOS_ADDR, 0x00);
    time->second = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x02);
    time->minute = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x04);
    time->hour = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x06);
    time->weekday = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x07);
    time->day = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x08);
    time->month = inb(CMOS_DATA);

    outb(CMOS_ADDR, 0x09);
    time->year = inb(CMOS_DATA);
}

static char buffer[30];

int rtc_get(time_t *time) {
    // Use local stack variable for quick access
    time_t last, current;
    rtc_fetch(&current);

    int reads = 0;
    do {
        memcpy(&last, &current, sizeof(time_t));
        rtc_fetch(&current);
        reads++;
    } while(memcmp(&current, &last, sizeof(time_t)) != 0);

    outb(CMOS_ADDR, 0x0B);
    uint8_t flags = inb(CMOS_DATA);

    if (!(flags & 0x04)) {
        current.second = (current.second & 0x0F) + ((current.second / 16) * 10);
        current.minute = (current.minute & 0x0F) + ((current.minute / 16) * 10);
        current.hour = ( (current.hour & 0x0F) + (((current.hour & 0x70) / 16) * 10) ) | (current.hour & 0x80);
        current.day = (current.day & 0x0F) + ((current.day / 16) * 10);
        current.month = (current.month & 0x0F) + ((current.month / 16) * 10);
        current.year = (current.year & 0x0F) + ((current.year / 16) * 10);
    }

    // Convert 12 hour clock to 24 hour clock if necessary
    if (!(flags & 0x02) && (current.hour & 0x80)) {
        current.hour = ((current.hour & 0x7F) + 12) % 24;
    }

    // Now set the result
    *time = current;

    return reads;
}