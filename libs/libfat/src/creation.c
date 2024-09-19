#include <fs/fat.h>
#include <memory.h>

// Debugging
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

static inline void fix_label(uint8_t *name, size_t size){
    size_t index = 0;
    while (index < size && *name) {
        index++;
        name++;
    }

    while (index++ < size)
        *name++ = 0x20;
}

static inline size_t label_length(uint8_t *name, size_t size){
    size_t index = 0;
    size_t length = 0;

    while (index++ < size) {
        if (*name++ != 0x20)
            length = index;
    }

    return length;
}

static inline void set_next_cluster(struct FATContext *ctx, uint32_t index, uint32_t next) {
    switch (ctx->type) {
        case FAT12:
            uint16_t *ptr = ctx->fat + (index + index / 2);

            if (index & 1) {
                *ptr = (*ptr & 0x000F) | ((next & 0xFFF) << 4);
            } else {
                *ptr = (*ptr & 0xF000) | (next & 0xFFF);
            }
        break;
        case FAT16:
            ((uint16_t*)ctx->fat)[index] = next;
        break;
        case FAT32:
            ((uint32_t*)ctx->fat)[index] = next;
        break;
    }
}

static inline void sanitize_parameters(const struct BlockDevice *device, struct FATCreateParams *parameters){
    // Valid values are 512, 1024, 2048, 4096. But I only care
    // that it's not zero
    if (parameters->bytesPerSector == 0)
        // header->bytesPerSector = 512;
        parameters->bytesPerSector = device->blockSize;

    // A minumum of 1 is required as this header resides in it
    if(parameters->reservedSectors == 0)
        parameters->reservedSectors = 1;

    // We could check to a minimum. But if some wants to set this
    // to 1, I won't judge.
    if (parameters->numberOfRootEntries == 0)
        parameters->numberOfRootEntries = 32;

    // The minumum is one and 2 is recomended
    if (parameters->numberOfFatCopies == 0)
        parameters->numberOfFatCopies = 2;

    // This could be done more intelligent, so larger mediums get
    // larger cluster sizes. But maybe the user can specify :)
    if (parameters->sectorsPerCluster == 0)
        parameters->sectorsPerCluster = 2;

    if (parameters->sectorsPerFat == 0) {
        // This is a rough estimate, but at least it will fit
        uint32_t numberOfClusters = parameters->numberOfSectors / parameters->sectorsPerCluster;
        parameters->sectorsPerFat = ((numberOfClusters * 4) + parameters->bytesPerSector - 1) / parameters->bytesPerSector;
    }

    // Make sure it's padded with spaces
    fix_label(parameters->oemName, 8);
    
    // Not needed
    if (label_length(parameters->oemName, 8) == 0)
        memory_copy(parameters->oemName, "MSWIN4.1", 8);

    // Make sure it's padded with spaces
    fix_label(parameters->volumeLabel, 11);

    // Some driver might complain otherwise
    if (parameters->mediaDescriptor == 0)
        parameters->mediaDescriptor = 0xF0;
}

int fat_create(struct FATContext *ctx, size_t size, const struct BlockDevice *device, struct FATCreateParams *parameters) {
    // This is a bit of a mandatory
    if (parameters->numberOfSectors == 0)
        return FAT_ERR_MINIMUM_SIZE;

    sanitize_parameters(device, parameters);


    // The sector size of the all the fat copies
    uint32_t fatSize = parameters->sectorsPerFat * parameters->numberOfFatCopies;
    
    // The sector size of the root directory (required bytes rounded up to the nearest sector size)
    uint32_t rootSize = ((parameters->numberOfRootEntries * 32) + parameters->bytesPerSector - 1) / parameters->bytesPerSector;

    // The data area starts right after the root directory
    uint32_t startOfData = parameters->reservedSectors + fatSize + rootSize;

    // Now we can calculate the sector size of the data area and divide it by the sectors per
    // cluster, to get the number of clusters
    uint32_t numberOfClusters = (parameters->numberOfSectors - startOfData) / parameters->sectorsPerCluster;

    // These numbers look odd, until you realize index 0 means empty, the second one defines
    // media type (again). And the last 8 indexes are used to indicate "End Of File" or an
    // error code like bad sector. So in a 12 bit system we can have 4086 addressable
    // clusters... now it's less odd.
    enum FATType type;
    if (numberOfClusters < 4085) {
        type = FAT12;
    } else if(numberOfClusters < 65525) {
        type = FAT16;
    } else {
        type = FAT32;
        // FAT32 doesn't have a fixed root directory
        startOfData-= rootSize;
        numberOfClusters = (parameters->numberOfSectors - startOfData) / parameters->sectorsPerCluster;
    }


    // ----------------------------------//
    // TODO: validate parameters
    // ----------------------------------//

    // Make sure context is large enough
    if (size < device->blockSize)
        return FAT_ERR_MINIMUM_SIZE;

    // Read current bootSector to preserve possible code
    device->read(device, 0, 1, ctx);

    // Header start at 3
    register struct FATBPB *bpb = ((void*)ctx) + 3;

    // Because the first part has the same layout we can just copy
    memory_copy(&bpb->header, parameters, sizeof(struct FATHeader));

    // If it's small enough put it in the 16bit field
    if (parameters->numberOfSectors < (1 << 16)) {
        bpb->header.smallNumberOfSectors = parameters->numberOfSectors;
        bpb->header.largeNumberOfSectors = 0;
    } else {
        bpb->header.smallNumberOfSectors = 0;
        bpb->header.largeNumberOfSectors = parameters->numberOfSectors;
    }

    if(type != FAT32){
        bpb->fat1x.bootSector.driveNumber = parameters->driveNumber;

        // According to specification a signature is not required, and
        // mounting the image without it is not a problem. But we do
        // include it otherwise tools like fsck.fat will complain.
        bpb->fat1x.bootSector.extendedBootSignature = 0x29;

        bpb->fat1x.signature.volumeSerialNumber = parameters->volumeSerialNumber;
        if (label_length(parameters->volumeLabel, 11) == 0){
            memory_copy(bpb->fat1x.signature.volumeLabel, "NO NAME    " , 11);
        } else {
            memory_copy(bpb->fat1x.signature.volumeLabel, parameters->volumeLabel, 11);
        }
        if(type == FAT12){
            memory_copy(bpb->fat1x.signature.fileSystemType, "FAT12   ", 8);
        } else {
            memory_copy(bpb->fat1x.signature.fileSystemType, "FAT16   ", 8);
        }
    } else {
        // NOT YET SUPPORTED
        return FAT_ERROR;
    }

    // Write it back to the device
    device->write(device, 0, 1, ctx);

    // Init the normal loading to get a fully functional context
    int resultCode;
    if ((resultCode = fat_init_context(ctx, size, device)) != FAT_SUCCESS)
        return resultCode;

    if (ctx->fat == 0)
        return FAT_ERROR;

    // The upper bit/bytes need to be the same as an EOC mark. We
    // can safely use the FAT32_EOC because trimmed down to 16 or
    // 12 bit it will become there EOC mark values.
    set_next_cluster(ctx, 0, 0x0FFFFF00 | parameters->mediaDescriptor);
    set_next_cluster(ctx, 1, FAT32_EOC);

    device->write(device, ctx->header->reservedSectors, 1, ctx->fat);
    device->write(device, ctx->header->reservedSectors + ctx->header->sectorsPerFat, 1, ctx->fat);

    return FAT_SUCCESS;
}

int fat_set_reserved(struct FATContext *ctx, uint32_t startIndex, uint32_t endIndex, const void *source, size_t size) {
    if(endIndex < startIndex)
        return FAT_ERROR;

    if(endIndex >= ctx->header->reservedSectors)
        return FAT_ERROR;

    uint32_t sectorSize = endIndex - startIndex + 1;

    size_t bytedNeeded = sectorSize * ctx->header->bytesPerSector;

    if (size > bytedNeeded)
        return FAT_ERROR;

    if(bytedNeeded > ctx->bufferSize)
        return FAT_ERROR;

    //printf("Sectors available %d\n", sectorSize);
    //printf("Sectors needed %d\n", (int)((size + ctx->header->bytesPerSector - 1) / ctx->header->bytesPerSector));

    memory_set(ctx->buffer, 0, bytedNeeded);
    memory_copy(ctx->buffer, source, size);
    if(startIndex == 0)
        memory_copy(ctx->buffer + 3, ctx->header, sizeof(struct FATBPB));

    ctx->device->write(ctx->device, startIndex, sectorSize, ctx->buffer);
    
    return FAT_SUCCESS;
}