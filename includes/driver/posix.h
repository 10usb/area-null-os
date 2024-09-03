#ifndef DRIVER_POSIX_H
#define DRIVER_POSIX_H

#include <io/device.h>

size_t posix_stream_device_size();

int posix_get_stream_device(struct BlockDevice *device, const char* filename, unsigned int blocksize);

#endif