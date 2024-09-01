#include "dma.h"

static inline void outb(uint16_t port, uint8_t value){
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port){
    uint8_t value;
    asm volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint16_t pagePorts[] = { 0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A };

int dma_setup(uint8_t channel, dma_settings_t *settings) {
    if(channel > 7)
        return 0;

    int16_t addressPort,
            pagePort,
            countPort,
            maskPort,
            resetPort,
            modePort;

    if(channel < 4){
        addressPort = channel * 2;
        countPort = addressPort + 1;

        maskPort = 0x0A;
        resetPort = 0x0C;
        modePort = 0x0B;
    } else {
        addressPort = 0xC0 + channel * 4;
        countPort = addressPort + 2;
        pagePort = channel * 4;

        maskPort = 0x0A;
        resetPort = 0x0C;
        modePort = 0x0B;
    }

    // Mask DMA channel
    outb(maskPort, 0x04 | (channel & 0b11));

    outb(resetPort, 0xFF);                              // Reset the master flip-flop
    outb(addressPort, settings->addrbytes[0]);          // Address 0-7
    outb(addressPort, settings->addrbytes[1]);          // Address 8-15
    outb(pagePorts[channel], settings->addrbytes[2]);   // Address 16-23

    outb(resetPort, 0xFF);                  // Reset the master flip-flop (again!!!)
    outb(countPort, settings->cntbytes[0]); // Count to 0x23ff (low byte)
    outb(countPort, settings->cntbytes[1]); // Count to 0x23ff (high byte),

    // Set mode
    uint8_t mode = 64 | channel;
    if (settings->mode == DMA_WRITE) {
        outb(modePort, mode | 8);
    } else {
        outb(modePort, mode | 4);
    }

    // Unmask DMA channel
    outb(maskPort, (channel & 0b11));

    return 1;
}
