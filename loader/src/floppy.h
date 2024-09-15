#ifndef FLOPPY_H
#define FLOPPY_H

#include <driver/floppy.h>

#include <stdint.h>
#include <stddef.h>
#include "interrupts.h" 
#include "tty.h" 
#include "text.h"
#include "dma.h"

enum FloppyRegister {
    FDC_SA = 0x3F0, // Status register A (read-only)
    FDC_SB = 0x3F1, // Status register B (read-only)
    FDC_DO = 0x3F2, // Digital Output register
    FDC_MS = 0x3F4, // Main Status register (read-only)
    FDC_DS = 0x3F4, // Datarate Select register (write-only)
    FDC_IO = 0x3F5, // Data IO register
    FDC_DI = 0x3F7, // Digital Input register (read-only)
    FDC_CC = 0x3F7, // Configuration Control register (write-only)
};

enum FloppyCommand {
   CMD_READ_TRACK           = 2, // generates IRQ6
   CMD_SPECIFY              = 3, // * set drive parameters
   CMD_SENSE_DRIVE_STATUS   = 4,
   CMD_WRITE_DATA           = 5, // * write to the disk
   CMD_READ_DATA            = 6, // * read from the disk
   CMD_RECALIBRATE          = 7, // * seek to cylinder 0
   CMD_SENSE_INTERRUPT      = 8, // * ack IRQ6, get status of last command
   CMD_WRITE_DELETED_DATA   = 9,
   CMD_READ_ID              = 10, // generates IRQ6
   CMD_READ_DELETED_DATA    = 12,
   CMD_FORMAT_TRACK         = 13, // *
   CMD_DUMPREG              = 14,
   CMD_SEEK                 = 15, // * Seek heads to cylinder X
   CMD_VERSION              = 16, // * used during initialization, once
   CMD_SCAN_EQUAL           = 17,
   CMD_PERPENDICULAR_MODE   = 18, // * used during initialization, once, maybe
   CMD_CONFIGURE            = 19, // * set controller parameters
   CMD_LOCK                 = 20, // * protect controller params from a reset
   CMD_VERIFY               = 22,
   CMD_SCAN_LOW_OR_EQUAL    = 25,
   CMD_SCAN_HIGH_OR_EQUAL   = 29
};

enum SectorSize {
    SECTOR_SIZE_128 = 0,
    SECTOR_SIZE_256 = 1,
    SECTOR_SIZE_512 = 2,
    SECTOR_SIZE_1K  = 3,
    SECTOR_SIZE_2K  = 4,
    SECTOR_SIZE_4K  = 5,
    SECTOR_SIZE_8K  = 6,
    SECTOR_SIZE_16K = 7,
};


#define DOR_RESET   0x04
#define DOR_DMA     0x08
#define DOR_MOTER_A 0x10
#define DOR_MOTER_B 0x20
#define DOR_MOTER_C 0x40
#define DOR_MOTER_D 0x80

#define MSR_BUSY_A     0x01
#define MSR_BUSY_B     0x02
#define MSR_BUSY_C     0x04
#define MSR_BUSY_D     0x08
#define MSR_CMD_BUSY   0x10
#define MSR_DMA        0x20
#define MSR_DIRECTION  0x40
#define MSR_IO_IDLE    0x80

#define IRQ_FLOPPY 6
#define FLPY_SECTORS_PER_TRACK 18

#define MULTI_TRACK 128
#define DOUBLE_DENSITY 64
#define SKIP_DELETED 32

struct CHS {
    uint16_t track;
    uint16_t head;
    uint16_t sector;
};

struct SenseResult {
    uint8_t st0;
    uint8_t track;
} __attribute__((packed));


struct FloppyDevice {
    struct BlockDevice device;
    uint8_t drive;
};

static int floppy_sense_interrupt(struct SenseResult *result);

static inline void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port){
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
#endif