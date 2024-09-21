#include "elf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *sectionTypes[] = {
    "NULL",
    "PROGBITS",
    "SYMTAB",
    "STRTAB",
    "RELA",
    "HASH",
    "DYNAMIC",
    "NOTE",
    "NOBITS",
    "REL",
    "SHLIB",
    "DYNSYM",
};

static char error[100] = { 0 };

const char *elflasterror(){
    return error;
}

const char *elf_get_section_name(struct ELFContext *context, uint32_t offset){
    if (context->header.sectionHeader.stringTableIndex >= context->header.sectionHeader.count)
        return 0;

    if (!context->sectionHeaders)
        return 0;

    uint32_t stringTableIndex = context->header.sectionHeader.stringTableIndex;
    if (offset >= context->sectionHeaders[stringTableIndex].size)
        return 0;

    if (!context->sections || !context->sections[stringTableIndex])
        return 0;

    return context->sections[stringTableIndex] + offset;
}

void elfprint(struct ELFContext *context) {
    printf("identifier          \"%-16.16s\"\n", context->header.identifier);
    printf("type                %-8d\n", context->header.type);
    printf("machine             %-8d\n", context->header.machine);
    printf("version             %-8d\n", context->header.version);
    printf("entryPoint          %-8d\n", context->header.entryPoint);
    printf("programOffset       %-8d\n", context->header.programOffset);
    printf("sectionOffset       %-8d\n", context->header.sectionOffset);
    printf("flags               %-8d\n", context->header.flags);
    printf("headerSize          %-8d\n", context->header.headerSize);
    printf("programHeader:\n");
    printf(" - entrySize        %-8d\n", context->header.programHeader.entrySize);
    printf(" - count            %-8d\n", context->header.programHeader.count);
    printf("sectionHeader:\n");
    printf(" - entrySize        %-8d\n", context->header.sectionHeader.entrySize);
    printf(" - count            %-8d\n", context->header.sectionHeader.count);
    printf(" - stringTableIndex %-8d\n", context->header.sectionHeader.stringTableIndex);
    printf("\n");

    if (!context->sectionHeaders)
        return;
    
    printf("SectionHeaders:\n");
    printf("index name                type     address offset size link info align entrySize flags\n");
    for (int index = 0; index < context->header.sectionHeader.count; index++) {
        struct ELFSectionHeader *section = context->sectionHeaders + index;

        printf("%5X ", index);
        printf("%4X %-14s ", section->name, elf_get_section_name(context, section->name));

        if (section->type < sizeof(sectionTypes)) {
            printf("%-8s ", sectionTypes[section->type]);
        } else{ 
            printf("%-8d ", section->type);
        }

        printf("%7X ", section->address);
        printf("%6X ", section->offset);
        printf("%4X ", section->size);
        printf(section->link ? "%4X " : "     ", section->link);
        printf(section->info ? "%4X " : "     ", section->info);
        printf(section->alignment ? "%5X " : "      ", section->alignment);
        printf(section->entrySize ? "%9X " : "          ", section->entrySize);
        printf("%5X ", section->flags);

        if (section->flags & SHF_WRITE)
            printf(" WRITE");

        if (section->flags & SHF_ALLOC)
            printf(" ALLOC");

        if (section->flags & SHF_EXEC)
            printf(" EXEC");


        printf("\n");
    }
    printf("\n");

    for (int index = 0; index < context->header.sectionHeader.count; index++) {
        struct ELFSectionHeader *section = context->sectionHeaders + index;

        if (section->type != SHT_SYMTAB)
            continue;

        printf("Symbols: %X %X\n", index, section->size / section->entrySize);
        printf("index name                               address   size type bind other  index\n");
        struct ELFSymbolEntry *current = context->sections[index];
        struct ELFSymbolEntry *end = context->sections[index] + section->size;

        uint8_t *strtab = context->sections[section->link];

        for (int index = 0; current < end; index++, current++) {
            printf("%4X ", index);
            printf("%5X ", current->name);
            printf("%-30.30s ", strtab ? strtab + current->name : "");
            printf("%6X ", current->address);
            printf("%6X ", current->size);
            printf("%4X ", current->type);
            printf("%4X ", current->bind);
            printf("%6X ", current->other);
            printf("%6X ", current->index);
            printf("\n");
        }
        printf("\n");
    }

    for (int index = 0; index < context->header.sectionHeader.count; index++) {
        struct ELFSectionHeader *section = context->sectionHeaders + index;

        if (section->type != SHT_REL)
            continue;

        printf("Relocations: %X %X\n", index, section->size / section->entrySize);
        printf("offset symbol type    value\n");
        struct ELFRelocation *current = context->sections[index];
        struct ELFRelocation *end = context->sections[index] + section->size;

        uint8_t *code = context->sections[section->info];

        for (; current < end; current++) {
            uint32_t value = 0;

            if (code){
                value = *(uint32_t*)(code + current->offset);
            }

            printf("%6X ", current->offset);
            printf("%6X ", current->symbol);
            printf("%4X ", current->type);
            printf("%8X ", value);
            printf("\n");
        }
        printf("\n");
    }
}

struct ELFContext *elfopen(const char *path) {
    FILE *handle = fopen(path, "r");
    if(!handle)
        return 0;

    struct ELFContext *context = malloc(sizeof(struct ELFContext));
    memset(context, 0, sizeof(struct ELFContext));

    // Read header
    if (fread(&context->header, sizeof(struct ELFHeader), 1, handle) != 1) {
        sprintf(error, "Failed to read header");
        goto error;
    }

    if (context->header.programHeader.count) {
        // Read program headers
        if (fseek(handle, context->header.programOffset, SEEK_SET) != 0) {
            sprintf(error, "Failed to seek to program headers");
            goto error;
        }

        context->programHeaders = malloc(context->header.sectionHeader.entrySize * context->header.sectionHeader.count);
        if (fread(context->programHeaders, context->header.programHeader.entrySize, context->header.programHeader.count, handle) != context->header.programHeader.count) {
            sprintf(error, "Failed to read program headers");
            goto error;
        }
    }

    if (context->header.sectionHeader.count > 0) {
        // Read section headers
        if (fseek(handle, context->header.sectionOffset, SEEK_SET) != 0) {
            sprintf(error, "Failed to seek to section headers");
            goto error;
        }

        context->sectionHeaders = malloc(context->header.sectionHeader.entrySize * context->header.sectionHeader.count);
        if (fread(context->sectionHeaders, context->header.sectionHeader.entrySize, context->header.sectionHeader.count, handle) != context->header.sectionHeader.count) {
            sprintf(error, "Failed to read section headers (%d %d %d)",
                context->header.sectionOffset,
                context->header.sectionHeader.entrySize,
                context->header.sectionHeader.count
            );
            goto error;
        }

        context->sections = malloc(context->header.sectionHeader.count * sizeof(void*));
        memset(context->sections, 0, sizeof(void *));

        for (uint32_t index = 0; index < context->header.sectionHeader.count; index++) {
            struct ELFSectionHeader *sectionHeader = context->sectionHeaders + index;
            context->sections[index] = malloc(sectionHeader->size);

            if (fseek(handle, sectionHeader->offset, SEEK_SET) != 0) {
                sprintf(error, "Failed to seek to section");
                goto error;
            }

            if (fread(context->sections[index], 1, sectionHeader->size, handle) != sectionHeader->size) {
                sprintf(error, "Failed to read section");
                goto error;
            }
        }
    }

    return context;

    error:
    fclose(handle);
    elfclose(context);
    return 0;
}

void elfclose(struct ELFContext *context) {
    if (!context)
        return;

    if (context->programHeaders)
        free(context->programHeaders);

    if (context->sectionHeaders)
        free(context->sectionHeaders);

    if (context->sections) {
        for (uint32_t index = 0; index < context->header.sectionHeader.count; index++)  {
            if (context->sections[index])
                free(context->sections[index]);
        }

        free(context->sections);
    }
    
    free(context);
}
