#include <fs/fat.h>
#include <memory.h>

int fat_init_context(struct FATContext *ctx, size_t size, const struct BlockDevice *device) {
    // Minumum size required to function
    if(size < sizeof(struct FATContext) + device->blockSize * 2)
        return FAT_ERR_MINIMUM_SIZE;

    // As the header of the FAT starts at 3 bytes in, will have the pointer
    // start at 3 bytes before the end of our context, so the header perfectly
    // aligns to the end of it.
    void *ptr = ((void*)ctx) + sizeof(struct FATContext) - 3;
    if (device->read(device, 0, 1, ptr) != 1)
        return FAT_ERR_FAILED_READ;

    // Now have a local pointer to it
    register struct FATBPB *bpb = ptr + 3;

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
 * Reads the value in the fat at index
 */ 
uint32_t fat_next_cluster(struct FATContext *ctx, uint32_t index) {
    switch (ctx->type) {
        case FAT12:
            uint16_t *ptr = ctx->fat + (index + index / 2);
            if(index & 1)
                return *ptr >> 4;
            return *ptr & 0xFFF;
        case FAT16:
            return ((uint16_t*)ctx->fat)[index];
        case FAT32:
            return ((uint32_t*)ctx->fat)[index];
    }

    return 0;
}

/**
 * To test if an index value is a EOC mark
 */
static inline int is_eoc(struct FATContext *ctx, uint32_t index) {
    // The first 2 don't exist
    if (index < 2 || index >= ctx->numberOfClusters)
        return 1;

    switch (ctx->type) {
        case FAT12: return index >= FAT12_EOC;
        case FAT16: return index >= FAT16_EOC;
        case FAT32: return index >= FAT32_EOC;
    }

    // In case we didn't know it's the end
    return 1;
}

/**
 * Public method of the static inline version
 */
int fat_is_eoc(struct FATContext *ctx, uint32_t index){
    return is_eoc(ctx, index);
}

/**
 * Reads the contents of the cluster into the buffer
 */
size_t fat_read_cluster(struct FATContext *ctx, uint32_t index, void *dst, size_t size) {
    uint32_t sectorCount = ctx->header->sectorsPerCluster;
    uint32_t sectorIndex = ctx->startOfData + ((index - 2) * sectorCount);

    uint32_t read = ctx->device->read(ctx->device, sectorIndex, sectorCount, ctx->buffer);
    
    if (read != sectorCount)
        return 0;

    if(size > sectorCount * ctx->header->bytesPerSector)
        size = sectorCount * ctx->header->bytesPerSector;

    memory_copy(dst, ctx->buffer, size);
    return size;
}

/**
  * Structure to keep track where we are at reading
  */
struct FATDirectoryReader {
    uint32_t clusterIndex;
    uint32_t sectorCount;
    uint32_t sectorIndex;
    uint32_t entriesCount;
};

/**
 * Reads a single cluster or the whole root directory
 */
static inline int fat_directory_reader_read(struct FATContext *ctx, struct FATDirectoryReader *reader) {
    if (reader->clusterIndex == 0) {
        reader->sectorCount = ctx->startOfData - ctx->startOfRootDirectory;
        reader->entriesCount = ctx->header->numberOfRootEntries;
        reader->sectorIndex = ctx->startOfRootDirectory;
    } else {
        // Read cluster, extract entries, repeat until last entry found or running out of clusters to read
        reader->sectorCount = ctx->header->sectorsPerCluster;
        reader->entriesCount = reader->sectorCount * ctx->header->bytesPerSector / 32;
        reader->sectorIndex = ctx->startOfData + ((reader->clusterIndex - 2) * reader->sectorCount);
    }

    uint32_t read = ctx->device->read(ctx->device, reader->sectorIndex, reader->sectorCount, ctx->buffer);

    return read == reader->sectorCount;
}

/**
 * Check to see if there are any more clusters to read
 */
static inline int fat_directory_reader_next(struct FATContext *ctx, struct FATDirectoryReader *reader){
    if (reader->clusterIndex != 0) {
        reader->clusterIndex = fat_next_cluster(ctx, reader->clusterIndex);
        return !is_eoc(ctx, reader->clusterIndex);
    }

    return 0;
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
    uint8_t segment[11];
    struct FATDirectoryReader reader;
    reader.clusterIndex = ctx->extended ? ctx->extended->rootCluster : 0;
    int hasPath;
    int32_t count = 0;

    next:
    // Remove any traling slashes
    while (*path == '/' || *path == '\\') 
        path++;

    // If de path is not empty turn first segment into directory
    // format for easy compare
    hasPath = *path ? 1 : 0;
    if (hasPath) {
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

    do {
        if(!fat_directory_reader_read(ctx, &reader))
            return 0;
        
        register struct FATDirectoryEntry *cursor = ctx->buffer;

        for (uint32_t index = 0; index < reader.entriesCount; index++, cursor++) {
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
                if (memory_compare(segment, cursor->name, 11) == 0) {
                    if (cursor->attributes.directory) {
                        reader.clusterIndex = cursor->firstClusterLowWord | (cursor->firstClusterHighWord << 16);
                        goto next;
                    } else {
                        // When there more path to traverse we did't find it.
                        if(*path)
                            return 0;

                        if(size > 0)
                            memory_copy(entries, cursor, sizeof(struct FATDirectoryEntry));

                        return 1;
                    }
                }
            } else {
                if(count < size)
                    memory_copy(entries + count, cursor, sizeof(struct FATDirectoryEntry));
                                    
                count++;
            }
        }
    } while (fat_directory_reader_next(ctx, &reader));
    done:

    // We can safely return count when we haven't found the file
    // we were looking for. The counter only gets incremented
    // when the last segment pointed to a directory. Otherwise
    // it stays untouched.
    return count;
}