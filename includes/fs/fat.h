#ifndef FS_FAT_H
#define FS_FAT_H

#include <io/device.h>
#include <io/stream.h>

#include <types.h>

struct FATHeader {
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numberOfFatCopies;
    uint16_t numberOfRootEntries;
    uint16_t smallNumberOfSectors;
    uint8_t mediaDescriptor;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t numberOfHeads;
    uint32_t hiddenSectors;
    uint32_t largeNumberOfSectors;
} __attribute__((__packed__));

struct FATExtendedHeader {
    uint32_t sectorsPerFat;
    uint16_t flags;
    union {
        uint16_t version;
        struct {
            uint8_t minor;
            uint8_t major;
        };
    };
    uint32_t rootCluster;
    uint16_t fileSystemInfoSector;
    uint16_t backupBootSector;
    uint8_t reserved[12];
} __attribute__((__packed__));

struct FATBootSector {
    uint8_t driveNumber;
    uint8_t reserved;
    uint8_t extendedBootSignature;
} __attribute__((__packed__));

struct FATSignature {
    uint32_t volumeSerialNumber;
    uint8_t volumeLabel[11];
    uint8_t fileSystemType[8];
} __attribute__((__packed__));

struct FATBPB {
    struct FATHeader header;
    union {
        struct {
            struct FATBootSector bootSector;
            struct FATSignature signature;
        } __attribute__((__packed__)) fat1x;
        struct {
            struct FATExtendedHeader extended;
            struct FATBootSector bootSector;
            struct FATSignature signature;
        } __attribute__((__packed__)) fat32;
    } __attribute__((__packed__));
} __attribute__((__packed__));

enum FATType {
    FAT12 = 014,
    FAT16 = 020,
    FAT32 = 040,
};

struct FATContext {
    size_t size;
    struct BlockDevice *device;
    enum FATType type;
    struct FATHeader *header;
    struct FATExtendedHeader *extended;
    struct FATBootSector *bootSector;
    struct FATSignature *signature;
    uint32_t startOfRootDirectory;
    uint32_t startOfData;
    void *fat;
    void *buffer;
};

struct FATTime {
    uint16_t seconds : 5;
    uint16_t minutes : 6;
    uint16_t hours : 5;
} __attribute__((__packed__));

struct FATDate {
    uint16_t day : 5;
    uint16_t month : 4;
    uint16_t year : 7;
} __attribute__((__packed__));

#define FAT_ATTR_READ_ONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_VOLUME_ID	0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE	0x20
#define FAT_ATTR_LONG_NAME	(FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

struct FATDirectoryEntry {
    union {
        uint8_t name[11];
        struct {
            uint8_t shortName[8];
            uint8_t extension[3];
        } __attribute__((__packed__));
    };
    
    uint8_t attributes;
    uint8_t reserved;
    struct {
        uint8_t miliseconds;
        struct FATTime time;
        struct FATDate date;
    } __attribute__((__packed__)) created;
    struct FATDate lastAccessed;
    uint16_t firstClusterHighWord;
    struct {
        struct FATTime time;
        struct FATDate date;
    } __attribute__((__packed__)) write;
    uint16_t firstClusterLowWord;
    uint32_t fileSize;
} __attribute__((__packed__));

#define FAT_SUCCESS		             0
#define FAT_ERR_MINIMUM_SIZE		-1
#define FAT_ERR_INVALID_FAT32 		-2
#define FAT_ERR_INVALID_FAT12_OR_16	-1

/**
 * Reads the headers from the device and prepares the context for use.
 * 
 * @param ctx 
 * @param size 
 * @param device 
 * @return Larger then 0 for success
 */
int fat_init_context(struct FATContext *ctx, size_t size, struct BlockDevice *device);

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
size_t fat_find_file(struct FATContext *ctx, struct FATDirectoryEntry *entries, size_t size, char *path);

#endif