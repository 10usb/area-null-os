#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include <stddef.h>

enum DMAMode {
    DMA_READ = 0,
    DMA_WRITE = 1
};

typedef struct {
    uint8_t mode;
    union {
        uint16_t count;
        uint8_t cntbytes[2];
    };
    union {
        uint32_t address;
        uint8_t addrbytes[3];
    };
} dma_settings_t;

int dma_setup(uint8_t channel, dma_settings_t *settings);

#endif