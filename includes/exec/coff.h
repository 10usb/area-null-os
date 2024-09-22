#ifndef EXEC_COFF_H
#define EXEC_COFF_H

#include <types.h>

struct COFFHeader {
    uint16_t magic;
    uint16_t sectionCount;
    uint32_t created;
    struct {
        uint32_t offset;
        uint32_t count;
    } symbols;
    uint16_t extra;
    uint16_t flags;
} PACKED;

struct COFFVersion {
    uint8_t major;
    uint8_t minor;
} PACKED;

struct COFFExtra {
    uint16_t magic;
    struct COFFVersion version;
    uint32_t textSize;
    uint32_t dataSize;
    uint32_t bssSize;
    uint32_t entryPoint;
    uint32_t textStart;
    uint32_t dataStart;
} PACKED;

union COFFName {
    char text[8];
    struct {
        uint32_t isText;
        uint32_t offset;
    };
} PACKED;

struct COFFSection {
    union COFFName name;
    uint32_t physicalAddress;
    uint32_t virtualAddress;
    uint32_t size;
    uint32_t offset;
    uint32_t relocationOffset;
    uint32_t lineNumbersOffset;
    uint16_t relocationCount;
    uint16_t lineNumbersCount;
    uint32_t flags;
} PACKED;

enum COFFSectionFlags {
    COFF_TEXT = 0x20,
    COFF_DATA = 0x40,
    COFF_BSS = 0x80,
};

struct COFFRelocation {
    uint32_t address;
    uint32_t symbol;
    uint16_t type;
} PACKED;

struct COFFSymbol {
    union COFFName name;
    uint32_t value;
    uint16_t section;
    uint16_t type;
    uint8_t storageClass;
    uint8_t extra;
} PACKED;

#endif