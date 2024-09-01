#ifndef DRIVER_FLOPPY_H
#define DRIVER_FLOPPY_H

#include <io/device.h>

void floppy_init();
void floppy_reset();

size_t floppy_get_device_size();

int floppy_get_device(uint8_t index, struct BlockDevice *device);

#endif