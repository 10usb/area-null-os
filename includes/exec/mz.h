#ifndef EXEC_MZ_H
#define EXEC_MZ_H

#include <types.h>

struct MZHeader {
    uint16_t signature;         // MZ
    uint16_t bytes;             // Bytes in last the page
    uint16_t pages;             // Number of 512 bytes pages large
    uint16_t relocationCount;
    uint16_t headerSize;        // in paragraphs
    uint16_t minimumAllocation; // in paragraphs
    uint16_t maximumAllocation; // in paragraphs
    uint16_t initialSS;
    uint16_t initialSP;
    uint16_t checksum;
    uint16_t initialIP;
    uint16_t initialCS;
    uint16_t relocationOffset;
    uint16_t overlay;
} PACKED;

struct MZRelocation {
    uint16_t offset;
    uint16_t segment;
} PACKED;

struct MZExtension {
    uint8_t reserved1[8];
    uint16_t oemIdentifier;
    uint16_t oemInfo;
    uint8_t reserved2[20];
    uint32_t offset;
} PACKED;

#endif