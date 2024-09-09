#include <fs/fat.h>
#include <memory.h>

// Debugging
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fat_create(struct FATContext *ctx, size_t size, const struct BlockDevice *device, struct FATCreateParams *parameters) {
    // This is a bit of a mandatory
    if (parameters->numberOfSectors == 0)
        return FAT_ERR_MINIMUM_SIZE;

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

    // Not needed 
    if (strnlen(parameters->oemName, 8) == 0)
        memory_copy(parameters->oemName, "MSWIN4.1", 8);

    // Some driver might complain otherwise
    if (parameters->mediaDescriptor == 0)
        parameters->mediaDescriptor = 0xF0;

    // ----------------------------------//
    // TODO: validate parameters
    // ----------------------------------//

    // Make sure context is large enough
    if(size < device->blockSize)
        return FAT_ERR_MINIMUM_SIZE;

    device->read(device, 0, 1, ctx);

    register struct FATBPB *bpb = ((void*)ctx) + 3;
    // Because the first part had the same layout we can just copy
    memory_copy(&bpb->header, parameters, sizeof(struct FATHeader));

    // If it's small enough put it in the 16bit field
    if (parameters->numberOfSectors < (1 << 16)) {
        bpb->header.smallNumberOfSectors = parameters->numberOfSectors;
        bpb->header.largeNumberOfSectors = 0;
    } else {
        bpb->header.smallNumberOfSectors = 0;
        bpb->header.largeNumberOfSectors = parameters->numberOfSectors;
    }

    bpb->fat1x.bootSector.extendedBootSignature = 0x29;

    bpb->fat1x.signature.volumeSerialNumber = 0xDEAD;
    memory_copy(bpb->fat1x.signature.volumeLabel, "NO NAME    " , 11);
    memory_copy(bpb->fat1x.signature.fileSystemType, "FAT12   ", 8);

    device->write(device, 0, 1, ctx);

    int resultCode;
    if ((resultCode = fat_init_context(ctx, size, device)) != FAT_SUCCESS)
        return resultCode;

    if (ctx->fat == 0)
        return FAT_ERR_MINIMUM_SIZE;

    *((uint32_t*)(ctx->fat)) = 0xFF8F00 | parameters->mediaDescriptor;

    device->write(device, ctx->header->reservedSectors, 1, ctx->fat);
    device->write(device, ctx->header->reservedSectors + ctx->header->sectorsPerFat, 1, ctx->fat);

    return FAT_SUCCESS;
}