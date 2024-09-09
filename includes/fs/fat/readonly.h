#ifndef FS_FAT_READONLY_H
#define FS_FAT_READONLY_H

#include <fs/fat/structure.h>

#define FAT_SUCCESS		             0
#define FAT_ERR_MINIMUM_SIZE		-1
#define FAT_ERR_INVALID_FAT32 		-2
#define FAT_ERR_INVALID_FAT12_OR_16	-3
#define FAT_ERR_FAILED_READ	        -4
#define FAT_ERR_FAILED_READ_FAT     -5

/**
 * Reads the headers from the device and prepares the context for use.
 * 
 * @param ctx 
 * @param size 
 * @param device 
 * @return Larger then 0 for success
 */
int fat_init_context(struct FATContext *ctx, size_t size, const struct BlockDevice *device);

#define FAT12_EOC 0x0FF8
#define FAT16_EOC 0xFFF8
#define FAT32_EOC 0x0FFFFFF8

/**
 * To get the next cluster of a given cluster
 * 
 * @param ctx   The context
 * @param index The cluster index to get the next entry of
 * @return The value of the next cluster or a EOC mark
 */
uint32_t fat_next_cluster(struct FATContext *ctx, uint32_t index);

/**
 * To test if a cluster index value it an end of cluster mark
 * 
 * @param ctx   The context
 * @param index The cluster index value to test
 * @return 1 if the cluster index is a EOC mark
 */
int fat_is_eoc(struct FATContext *ctx, uint32_t index);

/**
 * Read the contents of a cluster
 * 
 * @param ctx   The context
 * @param index The cluster index
 * @param dst   The the buffer to copy the contents to
 * @param size  Number of bytes to read into the dst buffer
 * @return The number of bytes read
 */
size_t fat_read_cluster(struct FATContext *ctx, uint32_t index, void *dst, size_t size);

/**
 * When it find the file it will return it entry, when it has a long name and size would
 * fit the name, the entries will be filled with does entries, otherwise it will return
 * only the entry with the dos name. When file found is a directory it will load the
 * entries of the directory up to the size.
 * 
 *  @param ctx 		The context
 *  @param entries 	Pointer to single entry or array of entries
 *  @param size 	Size available in the entries
 *  @param path 	A null-delimited string with the short entry path to be found
 *  @return Number of entries found
 */
int32_t fat_find_file(struct FATContext *ctx, struct FATDirectoryEntry *entries, int32_t size, const char *path);

#endif