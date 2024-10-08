#ifndef FS_FAT_STRUCTURE_H
#define FS_FAT_STRUCTURE_H

#include <io/device.h>
#include <types.h>

struct FATHeader {
    uint8_t oemName[8];
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
    const struct BlockDevice *device;
    enum FATType type;
    struct FATHeader *header;
    struct FATExtendedHeader *extended;
    struct FATBootSector *bootSector;
    struct FATSignature *signature;
    uint32_t startOfRootDirectory;
    uint32_t startOfData;
    uint32_t numberOfClusters;
    void *fat;
    void *buffer;
    size_t bufferSize;
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

union FATAttributes {
    uint8_t value;
    struct {
        uint8_t readOnly : 1;
        uint8_t hidden : 1;
        uint8_t system : 1;
        uint8_t volumeId : 1;
        uint8_t directory : 1;
        uint8_t archive : 1;
    } __attribute__((__packed__));
};

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
    union FATAttributes attributes;
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
    } __attribute__((__packed__)) modified;
    uint16_t firstClusterLowWord;
    uint32_t fileSize;
} __attribute__((__packed__));

#endif