#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/posix.h>
#include <fs/fat.h>

/**
 * Print the help info
 *
 * @param[in]  value  The value
 *
 * @return     Value given to this function
 */
static int print_help(int value) {
    printf("Usage:\n");
    printf(" fat help\n");
    printf(" fat info <file>\n");
    printf(" fat create <file> <sectors> [options]\n");
    printf("  -s N    Number of bytes per sector\n");
    printf("  -T N    Number of sectors per track\n");
    printf("  -H N    Number of heads\n");
    printf("  -r N    Number of sectors for the reservedsectors (min 1)\n");
    printf("  -F N    Number of FAT copies\n");
    printf("  -f N    Number of sectors per FAT\n");
    printf("  -e N    Maximum number of root entries\n");
    printf("  -h N    Number of hidden sectors preceding the filesystem\n");
    printf("  -c N    Number of sectors per cluster\n");
    printf(" fat list <file> [path]\n");
    printf(" fat load <file> <path> <destination>\n");
    printf(" fat store <file> <path> <source>\n");
    printf(" fat remove <file> <path>\n");
    return value;
}

/**
 * 
 */
static void print_info(struct FATContext *ctx) {
	printf("--- Disk Info --- (ambiguous)\n");
	printf("Bytes per sector           %8d\n", ctx->header->bytesPerSector);
	printf("Sectors per track          %8d\n", ctx->header->sectorsPerTrack);
	printf("Number of heads            %8d\n", ctx->header->numberOfHeads);
    printf("Media descriptor           %8X\n", ctx->header->mediaDescriptor);
    printf("\n");

	printf("--- Partition Info ---\n");
    printf("First sector               %8d\n", ctx->header->hiddenSectors);
    printf("Number of sectors          %8d\n", ctx->header->smallNumberOfSectors ? ctx->header->smallNumberOfSectors : ctx->header->largeNumberOfSectors);
    printf("\n");

    printf("--- Header ---\n");
    printf("OEM Name                 \"%8.8s\"\n", ctx->header->oemName);
	printf("Reserved sectors           %8d\n", ctx->header->reservedSectors);
    printf("Sectors per FAT            %8d\n", ctx->header->sectorsPerFat);
    printf("Fat copies                 %8d\n", ctx->header->numberOfFatCopies);
    printf("Sectors per cluster        %8d\n", ctx->header->sectorsPerCluster);
	printf("Root entries               %8d\n", ctx->header->numberOfRootEntries);
	printf("\n");

	if(ctx->extended){
		printf("--- Extended Header (32bit) ---\n");
		printf("Sectors per FAT             %8d\n", ctx->extended->sectorsPerFat);
		printf("Flags                       %8d\n", ctx->extended->flags);
		printf("Version                     %d.%d\n", ctx->extended->major, ctx->extended->minor);
		printf("Root cluster                %8d\n", ctx->extended->rootCluster);
		printf("fileSystemInfoSector        %8d\n", ctx->extended->fileSystemInfoSector);
		printf("Backup BootSector           %8d\n", ctx->extended->backupBootSector);
		printf("\n");
	}

	printf("--- Boot Sector ---\n");
	printf("Drive number (ambiguous)   %8d\n", ctx->bootSector->driveNumber);
	printf("Extended boot signature    %8d\n", ctx->bootSector->extendedBootSignature);
	printf("\n");

    
    printf("--- Signature ---\n");
    if(ctx->signature) {
        printf("Volume serial              %8d\n", ctx->signature->volumeSerialNumber);
        printf("Volume Label                  \"%11.11s\"\n", ctx->signature->volumeLabel);
        printf("FileSystem type               \"%8.8s\"\n", ctx->signature->fileSystemType);
    }else{
        printf("Not present\n");
    }
    printf("\n");

	printf("--- Caclculated ---\n");
    printf("Start root directory       %8d\n", ctx->startOfRootDirectory);
    printf("First data sector          %8d\n", ctx->startOfData);
    printf("Number of clusters         %8d\n", ctx->numberOfClusters);
    printf("Buffer size                %8ld\n", ctx->bufferSize);
}

/**
 * Show info about the image
 * 
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
static inline int main_create(int argc, char** argv) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return print_help(1);
    }
    
    struct FATCreateParams parameters;
    memset(&parameters, 0, sizeof(struct FATHeader));

    const char *file = argv[0];

    uint32_t i;

    if (!sscanf(argv[1], "%i", &i)) {
        printf("Failed to parse <sectors>\n");
        return print_help(1);
    }
    parameters.numberOfSectors = i;

    for(int index = 2; index < argc; index++){
        if(argv[index][0] != '-') {
            printf("Unknown argument '%s'\n", argv[index]);
            return print_help(1);
        }

        switch (argv[index][1]) {
            case 's':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <bytesPerSector>\n");
                    return print_help(1);
                }
                parameters.bytesPerSector = i;
            break;
            case 'T':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <sectorsPerTrack>\n");
                    return print_help(1);
                }
                parameters.sectorsPerTrack = i;
            break;
            case 'H':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <numberOfHeads>\n");
                    return print_help(1);
                }
                parameters.numberOfHeads = i;
            break;
            case 'r':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <reservedSectors>\n");
                    return print_help(1);
                }
                parameters.reservedSectors = i;
            break;
            case 'F':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <numberOfFatCopies>\n");
                    return print_help(1);
                }
                parameters.numberOfFatCopies = i;
            break;
            case 'f':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <sectorsPerFat>\n");
                    return print_help(1);
                }
                parameters.sectorsPerFat = i;
            break;
            case 'e':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <numberOfRootEntries>\n");
                    return print_help(1);
                }
                parameters.numberOfRootEntries = i;
            break;
            case 'h':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <hiddenSectors>\n");
                    return print_help(1);
                }
                parameters.hiddenSectors = i;
            break;
            case 'c':
                index++;
                if (!sscanf(argv[index], "%d", &i)) {
                    printf("Failed to parse <sectorsPerCluster>\n");
                    return print_help(1);
                }
                parameters.sectorsPerCluster = i;
            break;
            default:
                printf("Unknown argument '%s'\n", argv[index]);
                return print_help(1);
        }
    }

    struct BlockDevice *device = malloc(posix_stream_device_size());
    if(!posix_create_stream_device(device, argv[0], 512, parameters.numberOfSectors)){
        printf("Failed to open file command '%s'\n", argv[0]);
        free(device);
        return 1;
    }

    struct FATContext *ctx =  malloc(0x100000);
    int resultCode;
    if((resultCode = fat_create(ctx, 0x100000, device, &parameters)) != FAT_SUCCESS){
        printf("Failed to create filesystem %d\n", resultCode);
        device->action(device, BLOCK_DEVICE_CLOSE);
        free(device);
        free(ctx);
        return 1;
    }

    print_info(ctx);

    return 0;
}

/**
 * Show info about the image
 * 
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
static inline int main_info(int argc, char** argv) {
    if(argc < 1){
        printf("Not enough arguments\n");
        return print_help(1);
    }

    struct BlockDevice *device = malloc(posix_stream_device_size());
    if(!posix_get_stream_device(device, argv[0], 512)){
        printf("Failed to open file command '%s'\n", argv[0]);
        free(device);
        return 1;
    }

    struct FATContext *ctx =  malloc(0x100000);
    int resultCode;
    if((resultCode = fat_init_context(ctx, 0x100000, device)) != FAT_SUCCESS){
        printf("Failed to load filesystem %d\n", resultCode);
        device->action(device, BLOCK_DEVICE_CLOSE);
        free(device);
        free(ctx);
        return 1;
    }

    print_info(ctx);

    device->action(device, BLOCK_DEVICE_CLOSE);
    free(device);
    free(ctx);
    return 0;
}

/**
 * Show info about the image
 * 
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
static inline int main_list(int argc, char** argv) {
    if(argc < 1){
        printf("Not enough arguments\n");
        return print_help(1);
    }

    struct BlockDevice *device = malloc(posix_stream_device_size());
    if(!posix_get_stream_device(device, argv[0], 512)){
        printf("Failed to open file command '%s'\n", argv[0]);
        free(device);
        return 1;
    }

    struct FATContext *ctx =  malloc(0x100000);
    if(fat_init_context(ctx, 0x100000, device) != FAT_SUCCESS){
        printf("Failed to load filesystem\n");
        goto error;
    }

    const char *path = "";
    if(argc >= 2)
        path = argv[1];

    struct FATDirectoryEntry directory;
    int32_t count = fat_find_file(ctx, &directory, 1, path);
    if (count <= 0) {
        printf("Directory not found\n");
        goto error;
    }

    printf("Found %d entries\n", count);

    struct FATDirectoryEntry *entries = malloc(sizeof(struct FATDirectoryEntry) * count);

    if (fat_find_file(ctx, entries, count, path) != count){
        printf("Failed to read entries\n");
        free(entries);
        goto error;
    }

    printf("\n");

    struct FATDirectoryEntry *cursor = entries;
    for(int32_t index = 0; index < count; index++, cursor++) {
        printf("%.8s %.3s %c%c%c%c%c %8d %8d - %02d-%02d-%04d %02d:%02d:%02d - %02d-%02d-%04d %02d:%02d:%02d\n",
            cursor->shortName,
            cursor->extension,
            cursor->attributes.directory ? 'D' : '-',
            cursor->attributes.readOnly ? 'R' : 'W',
            cursor->attributes.hidden ? 'H' : '-',
            cursor->attributes.system ? 'S' : '-',
            cursor->attributes.archive ? 'A' : '-',
            cursor->fileSize,
            cursor->firstClusterLowWord | (cursor->firstClusterHighWord << 16),
            cursor->created.date.day,
            cursor->created.date.month,
            cursor->created.date.year + 1980,
            cursor->created.time.hours,
            cursor->created.time.minutes,
            cursor->created.time.seconds,
            cursor->modified.date.day,
            cursor->modified.date.month,
            cursor->modified.date.year + 1980,
            cursor->modified.time.hours,
            cursor->modified.time.minutes,
            cursor->modified.time.seconds
        );
    }
    free(entries);

    device->action(device, BLOCK_DEVICE_CLOSE);
    free(device);
    free(ctx);
    return 0;
    
    error:
    device->action(device, BLOCK_DEVICE_CLOSE);
    free(device);
    free(ctx);
    return 1;
}

/**
 * Show info about the image
 * 
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
static inline int main_load(int argc, char** argv) {
    if(argc < 3){
        printf("Not enough arguments\n");
        return print_help(1);
    }

    struct BlockDevice *device = malloc(posix_stream_device_size());
    if(!posix_get_stream_device(device, argv[0], 512)){
        printf("Failed to open file command '%s'\n", argv[0]);
        free(device);
        return 1;
    }

    struct FATContext *ctx =  malloc(0x100000);
    if(fat_init_context(ctx, 0x100000, device) != FAT_SUCCESS){
        printf("Failed to load filesystem\n");
        goto error;
    }

    struct FATDirectoryEntry entry;
    int32_t count = fat_find_file(ctx, &entry, 1, argv[1]);
    
    if (count <= 0) {
        printf("File not found\n");
        goto error;
    }

    if(entry.attributes.directory) {
        printf("Path is a directory\n");
        goto error;
    }

    FILE *f = fopen(argv[2], "w");
    if (!f) {
        printf("Failed to open file\n");
        goto error;
    }

    size_t bufferSize = ctx->header->sectorsPerCluster * ctx->header->bytesPerSector;
    void *buffer = malloc(bufferSize);
    uint32_t clusterIndex = entry.firstClusterLowWord | (entry.firstClusterHighWord << 16);
    uint32_t remainSize = entry.fileSize;
    uint32_t writeSize = bufferSize;

    do {
        size_t read = fat_read_cluster(ctx, clusterIndex, buffer, bufferSize);
        if (read != bufferSize) {
            printf("Failed to read cluster (%d != %d)\n", (uint32_t)read, (uint32_t)bufferSize);
            free(buffer);
            fclose(f);
            goto error;
        }

        if(remainSize < writeSize)
            writeSize = remainSize;

        size_t written = fwrite(buffer, 1, writeSize, f);
        if (written != writeSize) {
            printf("Failed to write to file (%d != %d)\n", (uint32_t)written, (uint32_t)writeSize);
            free(buffer);
            fclose(f);
            goto error;
        }
        remainSize-= writeSize;

        clusterIndex = fat_next_cluster(ctx, clusterIndex);
    } while (!fat_is_eoc(ctx, clusterIndex));


    fclose(f);

    device->action(device, BLOCK_DEVICE_CLOSE);
    free(device);
    free(ctx);
    return 0;
    
    error:
    device->action(device, BLOCK_DEVICE_CLOSE);
    free(device);
    free(ctx);
    return 1;
}

/**
 * Main entry for program
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
int main(int argc, char** argv){
    if (argc < 2){
        printf("No arguments given\n");
        return print_help(1);
    }

    if (strcmp(argv[1], "help") == 0)
         return print_help(0);

    if (strcmp(argv[1], "create") == 0)
         return main_create(argc - 2, argv + 2);

    if (strcmp(argv[1], "info") == 0)
         return main_info(argc - 2, argv + 2);

    if (strcmp(argv[1], "list") == 0)
         return main_list(argc - 2, argv + 2);

    if (strcmp(argv[1], "load") == 0)
         return main_load(argc - 2, argv + 2);

    printf("Unknown command '%s'\n", argv[1]);
    return print_help(1);
}