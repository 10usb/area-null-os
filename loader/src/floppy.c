#include "floppy.h"

static char buffer[50];
static int interrupt_flag = 0;
static int sense_interrupt = 0;
static struct SenseResult lastSense;

/**
 * When a interrupt is triggers we set the interrupt_flag to high.
 * And if it was a command that requires a sense interrupt perform
 * that here.
 */
static void floppy_irq_handler(int index, isr_frame_t *frame) {
    interrupt_flag = 1;

    if(sense_interrupt){
        sense_interrupt = 0;
        floppy_sense_interrupt(&lastSense);
    }
}

/**
 * Writes a command to the floppy controller and sets the interrupt_flag to low
 */
static int floppy_io_iwrite(size_t size, void *buffer, int sense) {
    register uint8_t *ptr = buffer;

    while (size > 0){
        int attempts = 100;
        // Wait till the IO register is ready to accept new data
        while((inb(FDC_MS) & MSR_IO_IDLE) == 0){
            if(--attempts <= 0)
                return 0;
        }
        if(size == 1)
            asm("cli");
        outb(FDC_IO, *ptr++);
        size--;
    }

    sense_interrupt = sense;
    interrupt_flag = 0;
    asm("sti\nhlt");
    return 1;
}

/**
 * Writes a command to the floppy controller
 */
static int floppy_io_write(size_t size, void *buffer) {
    register uint8_t *ptr = buffer;

    while (size > 0){
        int attempts = 100;
        // Wait till the IO register is ready to accept new data
        while((inb(FDC_MS) & MSR_IO_IDLE) == 0){
            if(--attempts <= 0)
                return 0;
        }
        outb(FDC_IO, *ptr++);
        size--;
    }
    return 1;
}

/**
 * Read from the floppy controlelr
 */
static int floppy_io_read(size_t size, void *buffer) {
    register uint8_t *ptr = buffer;

    int read;
    uint8_t msr;
    for(;;) {
        msr = inb(FDC_MS);

        if((msr & (MSR_DIRECTION | MSR_CMD_BUSY)) == 0)
            break;

        read++;

        if(size-- > 0)
            *ptr++ = inb(FDC_IO);
    }

    return read;
}

/**
 * Some commands don't have a response so wee need to perform
 * a sens to reset the controller so it accepts new actions
 */
static int floppy_sense_interrupt(struct SenseResult *result) {
    result->st0 = CMD_SENSE_INTERRUPT;
    if(!floppy_io_write(1, result))
        return 0;

    if(!floppy_io_read(2, result))
        return 0;

    return 1;
}

/**
 * Move the head to the write location
 */
static int floppy_seek(uint8_t drive, struct CHS chs) {
    snprintf(buffer, 50, "Seek to: %d - %d - %d\n", chs.track, chs.head, chs.sector);
    tty_puts(buffer);

    uint8_t cmd[3];
    cmd[0] = CMD_SEEK;
    cmd[1] = (chs.head << 2) | (drive & 3);
    cmd[2] = chs.track;
    if(!floppy_io_iwrite(3, cmd, 1))
        return 0;

    // Wait for interrupt (maybe in a more CPU friendly manner)
    while (!interrupt_flag);

    snprintf(buffer, 50, "LastSense: %x - %d\n", lastSense.st0, lastSense.track);
    tty_puts(buffer);

    return 1;
}

/**
 * Convert a sector index to an Cyliner-Heads-Sector address
 */
void floppy_index_to_chs(uint32_t index, struct CHS *chs) {
	chs->track = index / (FLPY_SECTORS_PER_TRACK * 2);
    chs->head = (index % (FLPY_SECTORS_PER_TRACK * 2) ) / FLPY_SECTORS_PER_TRACK;
	chs->sector = index % FLPY_SECTORS_PER_TRACK + 1;
}


/**
 * We are required to implement this
 */
static int floppy_action(const struct BlockDevice*, bdaction_t action){
    return 0;
}

/**
 * Read the given sectors
 */
static uint32_t floppy_read(const struct BlockDevice *device, uint32_t index, uint32_t count, void *address) {
    struct FloppyDevice *fd = (void*)device;

    struct CHS chs;
    floppy_index_to_chs(index, &chs);
    floppy_seek(fd->drive, chs);

    dma_settings_t settings;
    settings.mode = DMA_READ;
    settings.address = (uint32_t)address;
    settings.count = count * 512;
    dma_setup(2, &settings);

    uint8_t cmd[10];
    cmd[0] = CMD_READ_DATA | 128 | 64 | 32;
    cmd[1] = fd->drive;
    cmd[2] = chs.track;
    cmd[3] = chs.head;
    cmd[4] = chs.sector;
    cmd[5] = 2; // Sector size of 512
    cmd[6] = ((chs.sector+1) >= FLPY_SECTORS_PER_TRACK) ? FLPY_SECTORS_PER_TRACK : (chs.sector+1);
    cmd[7] = 27; // ?? Gap length of a 3.5" floppy
    cmd[8] = 0xff; // ?? not used data length, because sector size has been filled
    if(!floppy_io_iwrite(9, cmd, 0))
        return 0;

    // Wait for interrupt (maybe in a more CPU friendly manner)
    while (!interrupt_flag);
    
    // Read the result
    if(!floppy_io_read(7, cmd))
        return 0;

    snprintf(buffer, 30, "%02x %02x %02x %02x %02x %02x %02x\n",
        cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]);
    tty_puts(buffer);

    // uint8_t msr = inb(FDC_MS);
    // uint8_t dor = inb(FDC_DO);

    // snprintf(buffer, 30, "%02x %02x\n", msr, dor);
    // tty_puts(buffer);
    return 1;
}

/**
 * Write the given sectors
 */
static uint32_t floppy_write(const struct BlockDevice*, uint32_t index, uint32_t count, const void *address) {
    return 0;
}

/**
 * Size needed to store a block device
 */
size_t floppy_get_device_size() {
    return sizeof(struct FloppyDevice);
}

/**
 * Get a floppy device handle
 */
int floppy_get_device(uint8_t index, struct BlockDevice *device){
    register struct FloppyDevice *fd = (void*)device;

    fd->device.size = sizeof(struct FloppyDevice);
    fd->device.blockSize = 512;
    fd->device.action = floppy_action;
    fd->device.read = floppy_read;
    fd->device.write = floppy_write;
    fd->drive = index;
    return 1;
}

void floppy_init() {
    irq_install(IRQ_FLOPPY, floppy_irq_handler);
    irg_enable(IRQ_FLOPPY, 1);
}

void floppy_reset() {
    // - Set flags to reset controller
    // - Unset flags to reset controller
    // - Wait for IRQ
    // - Set transfer speed (compatible with floppy size)
    // - Command specify
    // - Calibrate (homing needle)
}