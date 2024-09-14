#ifndef FS_FAT_H
#define FS_FAT_H

#include <fs/fat/readonly.h>

/**
 * Contains all paremeters needed to create a new FAT FS
 */
struct FATCreateParams {
    uint8_t oemName[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numberOfFatCopies;
    uint16_t numberOfRootEntries;
    uint16_t __reverved0__;
    uint8_t mediaDescriptor;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t numberOfSectors;
    uint8_t driveNumber;
    uint32_t volumeSerialNumber;
    uint8_t volumeLabel[11];
}  __attribute__((__packed__));

/**
 * Uses the information in the header to create a new filesystem on the device
 * 
 * @param device 
 * @param header
 * @return 
 */
int fat_create(struct FATContext *ctx, size_t size, const struct BlockDevice *device, struct FATCreateParams *parameters);

/**
 * Set the reserved sectors with the new content
 * 
 * @param ctx           The context
 * @param startIndex    The first sector
 * @param endIndex      The last sector
 * @param source        Byte containing the content
 * @param size          Size of the source in bytes
 */
int fat_set_reserved(struct FATContext *ctx, uint32_t startIndex, uint32_t endIndex, const void *source, size_t size);

#endif