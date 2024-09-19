#ifndef EXEC_PE_H
#define EXEC_PE_H

#include <exec/mz.h>
#include <exec/coff.h>

struct PEVersion {
    uint16_t major;
    uint16_t minor;
} PACKED;

struct PEExtra {
    // COFF
    uint16_t magic;
    struct COFFVersion version;
    uint32_t textSize;
    uint32_t dataSize;
    uint32_t bssSize;
    uint32_t entryPoint;
    uint32_t textStart;
    uint32_t dataStart;
    // PE Extension
	uint32_t imageBase;
	uint32_t sectionAlignment;
	uint32_t fileAlignment;
    struct PEVersion osVersion;
    struct PEVersion imageVersion;
    struct PEVersion subSystemVersion;
	uint32_t win32VersionValue;
	uint32_t sizeOfImage;
	uint32_t sizeOfHeaders;
	uint32_t checkSum;
	uint16_t subsystem;
	uint16_t dllFlags;
	uint32_t sizeOfStackReserve;
	uint32_t sizeOfStackCommit;
	uint32_t sizeOfHeapReserve;
	uint32_t sizeOfHeapCommit;
	uint32_t loaderFlags;
	uint32_t numberOfRvaAndSizes;
} PACKED;

struct PEDirectoryEntry {
    uint32_t virtualAddress;
    uint32_t size;
} PACKED;

struct PESection {
    union COFFName name;
    uint32_t virtualSize; // in COFF this is the physicalAddress
    uint32_t virtualAddress;
    uint32_t size;
    uint32_t offset;
    uint32_t relocationOffset;
    uint32_t lineNumbersOffset;
    uint16_t relocationCount;
    uint16_t lineNumbersCount;
    uint32_t flags;
} PACKED;

#endif