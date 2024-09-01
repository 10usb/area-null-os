#include "interrupts.h"
#include "tty.h"
#include "text.h"

#define MASTER_CTRL 0x20
#define MASTER_DATA 0x21
#define SLAVE_CTRL 0xA0
#define SLAVE_DATA 0xA1

#define OFFSET 0x20
#define IRQ_ACK 0x20

#define ICW4_8086 0x01
#define ICW1_INIT 0x11

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

static const char *irg_names[16] = {
    "Timer",                                //  0
    "Keyboard",                             //  1
    "Cascade",                              //  2
    "COM2",                                 //  3
    "COM1",                                 //  4
    "LPT2",                                 //  5
    "Floppy Disk",                          //  6
    "LPT1",                                 //  7
    "Real-time clock",                      //  8
    "Legacy SCSI / NIC",                    //  9
    "SCSI / NIC (1)",                       // 10
    "SCSI / NIC (2)",                       // 11
    "PS2 Mouse",                            // 12
    "FPU / Coprocessor / Inter-processor",  // 13
    "Primary ATA HDD",                      // 14
    "Secondary ATA HDD",                    // 15
};

static char buffer[33];
static irq_vector_t handlers[15];
static void irq_handler(isr_frame_t *frame);

typedef union {
    uint16_t value;
    struct {
        uint8_t master;
        uint8_t slave;
    }  __attribute__((packed));
} irq_mask_t;

static inline irq_mask_t irg_get_mask() {
    irq_mask_t mask;
    mask.master = inb(MASTER_DATA);
    mask.slave = inb(SLAVE_DATA);
    return mask;
}

static inline void irg_set_mask(irq_mask_t mask) {
	outb(MASTER_DATA, mask.master);
	outb(SLAVE_DATA, mask.slave);
}

static void irq_print_mask(irq_mask_t mask) {
    tty_color_t current = tty_getcolor();

    for(int i = 0; i < 16; i++) {
        if (mask.value & (1 << i)) {
            tty_setcolors(current.background, TTY_RED);
            tty_puts("disabled ");
        } else {
            tty_setcolors(current.background, TTY_GREEN);
            tty_puts("enabled  ");
        }
        tty_setcolor(current);
        tty_puts(irg_names[i]);
        tty_put('\n');
    }
}

void irq_init() {
    // Get the current mask
    irq_mask_t mask = irg_get_mask();

    // Print for debugging
    //irq_print_mask(mask);

    // Starts the initialization sequence (in cascade mode)
	outb(MASTER_CTRL, ICW1_INIT);
	outb(SLAVE_CTRL, ICW1_INIT);

	outb(MASTER_DATA, OFFSET);      // ICW2: Master PIC vector offset
	outb(SLAVE_DATA, OFFSET + 8);   // ICW2: Slave PIC vector offset

	outb(MASTER_DATA, 4);           // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(SLAVE_DATA, 2);            // ICW3: tell Slave PIC its cascade identity (0000 0010)
	
	outb(MASTER_DATA, ICW4_8086);   // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	outb(SLAVE_DATA, ICW4_8086);

    // Restore saved masks.
    // irg_set_mask(mask);

    // Disabled all except the cascade of the 2 IC's
    irg_set_mask((irq_mask_t){~0x4});

    // Install irg isc handler
    for(int index = OFFSET; index < OFFSET+15; index++)
        isr_install(index, irq_handler);

    // Now we remapped, installed handler and disabled all,
    // it's save to turn interupts back on.
    sti;
}

void irg_enable(int index, int enabled) {
    uint16_t port;

    if(index < 8) {
        port = MASTER_DATA;
    } else {
        port = SLAVE_DATA;
        index -= 8;
    }

    uint8_t value;
    if(enabled) {
        value = inb(port) & ~(1 << index);
    } else {
        value = inb(port) | (1 << index);
    }
    outb(port, value);  
}

void irq_install(int index, irq_vector_t callback) {
    handlers[index] = callback;
}

static void irq_handler(isr_frame_t *frame) {
    int irq_number = frame->number - OFFSET;
    
    register irq_vector_t callback = handlers[irq_number];

    if (callback) {
        callback(irq_number, frame);
    } else {
        tty_puts(itos(irq_number, buffer, 30, 16));
        tty_puts(" - IRQ Unhandeled\n");
    }

    // send EOI
    if (irq_number >= 8)
        outb(SLAVE_CTRL, IRQ_ACK);
    outb(MASTER_CTRL, IRQ_ACK);
}
