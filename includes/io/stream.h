#ifndef IO_STREAM_H
#define IO_STREAM_H

#include <stdint.h>
#include <stddef.h>

typedef enum StreamOrigin {
	STREAM_SEEK_SET = 0,
	STREAM_SEEK_CURRENT = 1,
	STREAM_SEEK_END = 2,
} streamorigin_t;

typedef struct Stream {
	int size;
	int (*seek)(struct Stream *handle, size_t position, streamorigin_t origin);
	size_t (*tell)(struct Stream *handle);
	size_t (*read)(struct Stream *handle, size_t size, void *address);
	size_t (*write)(struct Stream *handle, size_t size, const void *address);
	int (*close)(struct Stream *handle);
} stream_t;

#endif