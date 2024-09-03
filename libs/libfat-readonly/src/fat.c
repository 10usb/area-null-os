#include <fs/fat.h>
#include <memory.h>

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
    device->read(device, 0, 1, ptr);

    // Now have a local pointer to it
    register struct FATBPB *bpb = ptr + 11;

    // Now we know the bytes of the context won't be overwritten we can set it
    // properties
    ctx->size = size;
    ctx->device = device;
    ctx->header = &bpb->header;

    // These fields must be 0 for FAT32, so it one is zero we walk the 32bit path
    if (bpb->header.sectorsPerFat == 0 || bpb->header.numberOfRootEntries == 0) {
        // If one is not zero we have an error
        if(bpb->header.sectorsPerFat != 0 || bpb->header.numberOfRootEntries != 0)
            return FAT_ERR_INVALID_FAT32;

        ctx->extended = &bpb->fat32.extended;
        ctx->bootSector = &bpb->fat32.bootSector;
        ctx->signature = &bpb->fat32.signature;


        // The sector size of the all the fat copies
        uint32_t fatSize = bpb->header.sectorsPerFat * bpb->header.numberOfFatCopies;

        // In FAT32 there we're some improvements done
        ctx->startOfRootDirectory = 0;

        // Now the data starts directly after the fat copies
        ctx->startOfData = bpb->header.reservedSectors + fatSize;
    } else {
        ctx->extended = 0;
        ctx->bootSector = &bpb->fat1x.bootSector;
        ctx->signature = &bpb->fat1x.signature;

        // The sector size of the all the fat copies
        uint32_t fatSize = bpb->header.sectorsPerFat * bpb->header.numberOfFatCopies;

        // The root directory starts right after the fats
        ctx->startOfRootDirectory = bpb->header.reservedSectors + fatSize;
        
        // The sector size of the root directory (required bytes rounded up to the nearest sector size)
        uint32_t rootSize = ((bpb->header.numberOfRootEntries * 32) + bpb->header.bytesPerSector - 1) / bpb->header.bytesPerSector;

        // The data area starts right after the root directory
        ctx->startOfData = ctx->startOfRootDirectory + rootSize;

        // They found out 16 bit we'ern't enough so added a new 32 bit one, yeah!
        uint32_t totalNumberOfSectors = bpb->header.smallNumberOfSectors
                        ? bpb->header.smallNumberOfSectors
                        : bpb->header.largeNumberOfSectors;

        // Now we can calculate the sector size of the data area and divide it by the sectors per
        // cluster, to get the number of clusters
        uint32_t numberOfClusters = (totalNumberOfSectors - ctx->startOfData) / bpb->header.sectorsPerCluster;

        // These numbers look odd, until you realize index 0 means empty, the second one defines
        // media type (again). And the last 8 indexes are used to indicate "End Of File" or an
        // error code like bad sector. So in a 12 bit system we can have 4086 addressable
        // clusters... now it's less odd.

        if (numberOfClusters < 4085) {
            ctx->type = FAT12;
        } else if(numberOfClusters < 65525) {
            ctx->type = FAT16;
        } else {
            return FAT_ERR_INVALID_FAT12_OR_16;
        }
    }

    // TODO allocate the rest of the size to the buffer, and if there is room to spare load the
    //      FAT into memory for quick access.

    return 0;
}

size_t fat_find_file(struct FATContext *ctx, struct FATDirectoryEntry *entries, size_t size, char *path) {
    return 0;
}