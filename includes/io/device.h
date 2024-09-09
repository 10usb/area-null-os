#ifndef IO_DEVICE_H
#define IO_DEVICE_H

#include <stdint.h>
#include <stddef.h>

typedef enum BlockDeviceAction {
    BLOCK_DEVICE_OPEN = 1,
    BLOCK_DEVICE_CLOSE = 2,
    BLOCK_DEVICE_FLUSH = 3
} bdaction_t;

struct BlockDevice {
    size_t size;
    int blockSize;
    int (*action)(const struct BlockDevice*, bdaction_t action);
    uint32_t (*read)(const struct BlockDevice*, uint32_t index, uint32_t count, void *address);
    uint32_t (*write)(const struct BlockDevice*, uint32_t index, uint32_t count, const void *address);
};

#endif