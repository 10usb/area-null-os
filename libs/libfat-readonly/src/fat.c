#include <fs/fat.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

// To make sure our code is memory safe
typedef char p__LINE__[ (sizeof(struct FATContext) >= 11) ? 1 : -1];

int fat_init_context(struct FATContext *ctx, size_t size, struct BlockDevice *device) {
    // Minumum size required to function
    if(size < sizeof(struct FATContext) + device->blockSize * 2)
        return FAT_ERR_MINIMUM_SIZE;

    // As the header of the FAT starts at 11 bytes in, will have the pointer
    // start at 11 bytes before the end of our context, so the header perfectly
    // aligns to the end of it.
    void *ptr = ((void*)ctx) + sizeof(struct FATContext) - 11;
    if (device->read(device, 0, 1, ptr) != 1)
        return FAT_ERR_FAILED_READ;

    // Now have a local pointer to it
    register struct FATBPB *bpb = ptr + 11;

    // Now we know the bytes of the context won't be overwritten we can set it
    // properties
    ctx->size = size;
    ctx->device = device;
    ctx->header = &bpb->header;
    ctx->buffer = ((void*)ctx) + sizeof(struct FATContext)  + sizeof(struct FATBPB);
    ctx->bufferSize = size - sizeof(struct FATContext)  + sizeof(struct FATBPB);

    // They found out 16 bit we'ern't enough so added a new 32 bit one, yeah!
    uint32_t totalNumberOfSectors = bpb->header.smallNumberOfSectors
                    ? bpb->header.smallNumberOfSectors
                    : bpb->header.largeNumberOfSectors;

    // These fields must be 0 for FAT32, so it one is zero we walk the 32bit path
    if (bpb->header.sectorsPerFat == 0 || bpb->header.numberOfRootEntries == 0) {
        // If one is not zero we have an error
        if(bpb->header.sectorsPerFat != 0 || bpb->header.numberOfRootEntries != 0)
            return FAT_ERR_INVALID_FAT32;

        ctx->type = FAT32;

        ctx->extended = &bpb->fat32.extended;
        ctx->bootSector = &bpb->fat32.bootSector;
        if (bpb->fat32.bootSector.extendedBootSignature == 0x29) {
            ctx->signature = &bpb->fat32.signature;
        } else {
            ctx->signature = 0;
        }

        // The sector size of the all the fat copies
        uint32_t fatSize = bpb->fat32.extended.sectorsPerFat * bpb->header.numberOfFatCopies;

        // In FAT32 there we're some improvements done
        ctx->startOfRootDirectory = 0;

        // Now the data starts directly after the fat copies
        ctx->startOfData = bpb->header.reservedSectors + fatSize;

        // Now we can calculate the sector size of the data area and divide it by the sectors per
        // cluster, to get the number of clusters
        ctx->numberOfClusters = (totalNumberOfSectors - ctx->startOfData) / bpb->header.sectorsPerCluster;
    } else {
        ctx->extended = 0;
        ctx->bootSector = &bpb->fat1x.bootSector;

        if (bpb->fat1x.bootSector.extendedBootSignature == 0x29) {
            ctx->signature = &bpb->fat1x.signature;
        } else {
            ctx->signature = 0;
        }

        // The sector size of the all the fat copies
        uint32_t fatSize = bpb->header.sectorsPerFat * bpb->header.numberOfFatCopies;

        // The root directory starts right after the fats
        ctx->startOfRootDirectory = bpb->header.reservedSectors + fatSize;
        
        // The sector size of the root directory (required bytes rounded up to the nearest sector size)
        uint32_t rootSize = ((bpb->header.numberOfRootEntries * 32) + bpb->header.bytesPerSector - 1) / bpb->header.bytesPerSector;

        // The data area starts right after the root directory
        ctx->startOfData = ctx->startOfRootDirectory + rootSize;

        // Now we can calculate the sector size of the data area and divide it by the sectors per
        // cluster, to get the number of clusters
        ctx->numberOfClusters = (totalNumberOfSectors - ctx->startOfData) / bpb->header.sectorsPerCluster;

        // These numbers look odd, until you realize index 0 means empty, the second one defines
        // media type (again). And the last 8 indexes are used to indicate "End Of File" or an
        // error code like bad sector. So in a 12 bit system we can have 4086 addressable
        // clusters... now it's less odd.

        if (ctx->numberOfClusters < 4085) {
            ctx->type = FAT12;
        } else if(ctx->numberOfClusters < 65525) {
            ctx->type = FAT16;
        } else {
            return FAT_ERR_INVALID_FAT12_OR_16;
        }
    }

    // Only when the table size is less then  2/3 of buffer size load table
    size_t tableSize = ctx->header->sectorsPerFat * ctx->header->bytesPerSector;
    if (tableSize < (ctx->bufferSize * 2 / 3)) {
        uint32_t read = device->read(device, bpb->header.reservedSectors, ctx->header->sectorsPerFat, ctx->buffer);

        if (read != ctx->header->sectorsPerFat)
            return FAT_ERR_FAILED_READ_FAT;

        ctx->fat = ctx->buffer;
        ctx->buffer+= tableSize;
        ctx->bufferSize-= tableSize;
    }

    return FAT_SUCCESS;
}

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
int32_t fat_find_file(struct FATContext *ctx, struct FATDirectoryEntry *entries, int32_t size, const char *path) {
    // 1 - Load root directory
    // 2 - if path is empty
    //   2.1 - fill entries up to size
    //   2.2 - read untill end and return count
    // 3 - if path not empty
    //   3.1 - search for entry matching name
    //   3.2 - if directory
    //      3.2.1 - load directory
    //      3.2.2 - goto 2
    //   3.3 - if file
    //      3.3.1 - if next path segment is empty
    //        3.3.1.2 - fill first entry
    //        3.3.1.3 - return 1
    //      3.3.2 - if next path segment is not empty
    //        3.3.2.1 - return 0
    //   3.4 - if not found return 0

    uint8_t segment[11];
    uint32_t clusterIndex = ctx->extended ? ctx->extended->rootCluster : 0;
    int hasPath;
    int32_t count = 0;

    next:

    // Remove any traling slashes
    while (*path == '/' || *path == '\\') 
        path++;

    // If de path is not empty turn first segment into directory
    // format for easy compare
    hasPath = *path ? 1 : 0;
    if(hasPath){
        uint8_t *ptr = segment;

        if (*path == '.') {
            *ptr++ = *path++;
            int i = 1;

            if(*path == '.'){
                *ptr++ = *path++;
                i++;
            }

            for (; i < 11; i++)
                *ptr++ = 0x20;
        } else {
            for (int i = 0; i < 11; i++) {
                switch (*path) {
                    case '/':
                    case '\\':
                    case '\0':
                        *ptr++ = 0x20;
                    break;
                    case '.':
                        if (i < 8) {
                            *ptr++ = 0x20;
                            break;
                        }
                        path++;
                    default:
                        *ptr++ = *path++;
                }
            }
        }

        if (*path == '/' || *path == '\\') {
            path++;
        } else if(*path != '\0') {
            return 0;
        }
    }

    if (clusterIndex == 0) {
        uint32_t sectorCount = ctx->startOfData - ctx->startOfRootDirectory;
        uint32_t entriesCount = ctx->header->numberOfRootEntries;

        uint32_t read = ctx->device->read(ctx->device, ctx->startOfRootDirectory, sectorCount, ctx->buffer);

        if(read != sectorCount)
            return 0;

        struct FATDirectoryEntry *cursor = ctx->buffer;

        for (uint32_t index = 0; index < entriesCount; index++, cursor++) {
            // This search method is for the short name so skip these
            if ((cursor->attributes.value & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME)
                continue;

            // End readed
            if (cursor->name[0] == 0)
                break;

            // Entry is empty
            if (cursor->name[0] == 0xE5)
                continue;
            
            if (hasPath) {
                if (memcmp(segment, cursor->name, 11) == 0) {
                    clusterIndex = cursor->firstClusterLowWord | (cursor->firstClusterHighWord << 16);
                    goto next;
                }
            } else {
                if(count < size)
                    memcpy(entries + count, cursor, sizeof(struct FATDirectoryEntry));
                                    
                count++;
            }
        }
    } else {
        // Read cluster, extract entries, repeat until last entry found or running out of clusters to read
        uint32_t sectorCount = ctx->header->sectorsPerCluster;
        uint32_t entriesCount = sectorCount * ctx->header->bytesPerSector / 32;
        uint16_t sectorIndex = ctx->startOfData + ((clusterIndex - 2) * sectorCount);

        uint32_t read = ctx->device->read(ctx->device, sectorIndex, sectorCount, ctx->buffer);
        if(read != sectorCount)
            return 0;
        
        struct FATDirectoryEntry *cursor = ctx->buffer;

        for (uint32_t index = 0; index < entriesCount; index++, cursor++) {
            // This search method is for the short name so skip these
            if ((cursor->attributes.value & FAT_ATTR_LONG_NAME) == FAT_ATTR_LONG_NAME)
                continue;

            // End readed
            if (cursor->name[0] == 0)
                break;

            // Entry is empty
            if (cursor->name[0] == 0xE5)
                continue;

            if (hasPath) {
                if (memcmp(segment, cursor->name, 11) == 0) {
                    clusterIndex = cursor->firstClusterLowWord | (cursor->firstClusterHighWord << 16);
                    goto next;
                }
            } else {
                if(count < size)
                    memcpy(entries + count, cursor, sizeof(struct FATDirectoryEntry));
                                    
                count++;
            }
        }
        
        // TODO: get next clusterIndex in the chain
    }

    return count;
}