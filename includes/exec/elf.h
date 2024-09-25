#ifndef EXEC_ELF_H
#define EXEC_ELF_H

#include <types.h>

enum ELFType {
    ET_NONE = 0,        // No file type
    ET_REL = 1,         // Relocatable file
    ET_EXEC = 2,        // Executable file
    ET_DYN = 3,         // Shared object file
    ET_CORE = 4,        // Core file
};

struct ELFHeader {
    uint8_t	identifier[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entryPoint;
    uint32_t programOffset;
    uint32_t sectionOffset;
    uint32_t flags;
    uint16_t headerSize; // Size of header
    struct {
        uint16_t entrySize; // Size of an entry in the program header
        uint16_t count; // Number of entries in the program header
    } PACKED programHeader;
    struct {
        uint16_t entrySize; // Size of an entry in the section header
        uint16_t count; // Number of entries in the section header
        uint16_t stringTableIndex; // Index of the section containing a string table used for names of the sections
    } PACKED sectionHeader;
} PACKED;

enum {
    PHF_EXEC    = 1,
    PHF_WRITE   = 2,
    PHF_READ    = 4,
};

struct ELFProgramHeader {
    uint32_t type;
    uint32_t offset;
    uint32_t virtualAddress;
    uint32_t physicalAddress;
    uint32_t sizeInFile;
    uint32_t sizeInMemory;
    uint32_t flags;
    uint32_t alignment;
} PACKED;

struct ELFSectionHeader {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t address;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t alignment;
    uint32_t entrySize;
} PACKED;

enum {
    SHT_NULL        = 0,
    SHT_PROGBITS    = 1,
    SHT_SYMTAB      = 2,
    SHT_STRTAB      = 3,
    SHT_RELA        = 4,
    SHT_HASH        = 5,
    SHT_DYNAMIC     = 6,
    SHT_NOTE        = 7,
    SHT_NOBITS      = 8,
    SHT_REL         = 9,
    SHT_SHLIB       = 10,
    SHT_DYNSYM      = 11
};

enum {
    SHF_WRITE       = 1,
    SHF_ALLOC       = 2,
    SHF_EXEC        = 4,
};

enum {
    ST_NOTYPE = 0,
    ST_OBJECT = 1,
    ST_FUNC = 2,
    ST_SECTION = 3,
    ST_FILE = 4,
};
struct ELFSymbol {
    uint32_t name;
    uint32_t address;
    uint32_t size;
    union {
        uint8_t info;
        struct {
            uint8_t type : 4;
            uint8_t bind : 4;
        };
    } PACKED;
    uint8_t other;
    uint16_t index;
} PACKED;

struct ELFRelocation {
    uint32_t offset;
    union {
        uint32_t info;
        struct {
            uint8_t type;
            uint32_t symbol : 24;
        } PACKED;
    };
} PACKED;

struct ELFRelocationAddend {
    uint32_t offset;
    union {
        uint32_t info;
        struct {
            uint8_t type;
            uint32_t symbol : 24;
        } PACKED;
    };
    uint32_t addend;
} PACKED;

struct ELFContext {
    struct ELFHeader header;
    struct ELFProgramHeader *programHeaders;
    struct ELFSectionHeader *sectionHeaders;
    void **sections;
};

#endif