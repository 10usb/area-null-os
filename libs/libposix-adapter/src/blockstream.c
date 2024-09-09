#include <driver/posix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StreamBlockDevice {
	struct BlockDevice device;
	FILE * handle;
};

/**
 * Peform close or flush
 */
static int posix_stream_device_action(const struct BlockDevice *device, bdaction_t action){
	struct StreamBlockDevice *sbd = (void*)device;;

	if(sbd->handle == 0)
		return 0;

	switch (action) {
		case BLOCK_DEVICE_CLOSE:
			if(fclose(sbd->handle)!=0)
				return 0;
			sbd->handle = 0;
			return 1;
		case BLOCK_DEVICE_FLUSH:
			return fflush(sbd->handle) == 0;
	}

    return 0;
}

/**
 * Read the given sectors
 */
static uint32_t posix_stream_device_read(const struct BlockDevice *device, uint32_t index, uint32_t count, void *address) {
    struct StreamBlockDevice *sbd = (void*)device;

	if(sbd->handle == 0)
		return 0;

	if(fseek(sbd->handle, index * device->blockSize, SEEK_SET) != 0)
		return 0;

	return fread(address, device->blockSize, count, sbd->handle);
}

/**
 * Write the given sectors
 */
static uint32_t posix_stream_device_write(const struct BlockDevice *device, uint32_t index, uint32_t count, const void *address) {
	struct StreamBlockDevice *sbd = (void*)device;

	if(sbd->handle == 0)
		return 0;

	if(fseek(sbd->handle, index * device->blockSize, SEEK_SET) != 0)
		return 0;

	return fwrite(address, device->blockSize, count, sbd->handle);
}

size_t posix_stream_device_size(){
	return sizeof(struct StreamBlockDevice);
}

int posix_get_stream_device(struct BlockDevice *device, const char* filename, unsigned int blockSize) {
	FILE* handle = fopen(filename, "r");
	if (handle) {
		handle = freopen(filename, "r+b", handle);
	} else {
		handle = fopen(filename, "w+b");
	}

	if(!handle)
		return 0;

	struct StreamBlockDevice *wrapper = (void*)device;
	wrapper->device.size		= sizeof(struct StreamBlockDevice);
	wrapper->device.blockSize 	= blockSize;
	wrapper->device.action		= posix_stream_device_action;
	wrapper->device.read		= posix_stream_device_read;
	wrapper->device.write		= posix_stream_device_write;
	wrapper->handle				= handle;

	return 1;
}

int posix_create_stream_device(struct BlockDevice *device, const char* filename, uint16_t blocksize, uint32_t count){
	FILE* handle = fopen(filename, "w+b");
	if(!handle)
		return 0;

	void *emptySector = malloc(blocksize);
	memset(emptySector, 0, blocksize);

	for (uint32_t index = 0; index < count; index++) {
		if (fwrite(emptySector, 1, blocksize, handle) != blocksize) {
			free(emptySector);
			fclose(handle);
			return 0;
		}
	}

	free(emptySector);
	fclose(handle);

	return posix_get_stream_device(device, filename, blocksize);
}