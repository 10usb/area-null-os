#include "elf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *elfTypes[] = {
    "NONE",
    "REL",
    "EXEC",
    "DYN",
    "CORE",
};

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

const char *programTypes[] = {
    "NULL",
    "LOAD",
    "DYNAMIC",
    "INTERP",
    "NOTE",
    "SHLIB",
    "PHDR"
};

const char *symbolTypes[] = {
    "NOTYPE",
    "OBJECT",
    "FUNC",
    "SECTION",
    "FILE",
};

const char *symbolBinds[] = {
    "LOCAL",
    "GLOBAL",
    "WEAK",
};

static char error[100] = { 0 };

/**
 * Returns the buffer 
 */
const char *elf_last_error(){
    return error;
}

/**
 * @brief 
 * @param context 
 * @param offset 
 * @return 
 */
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

/**
 * Print all value
 */
void elf_print(struct ELFContext *context) {
    printf("identifier          \"%-16.16s\"\n", context->header.identifier);

    if (context->header.type < sizeof(elfTypes)) {
        printf("type                %-8s\n", elfTypes[context->header.type]);
    } else{ 
        printf("type                %-8d\n", context->header.type);
    }

    printf("machine             %-8d\n", context->header.machine);
    printf("version             %-8d\n", context->header.version);
    printf("entryPoint          %-8X\n", context->header.entryPoint);
    printf("programOffset       %-8X\n", context->header.programOffset);
    printf("sectionOffset       %-8X\n", context->header.sectionOffset);
    printf("flags               %-8d\n", context->header.flags);
    printf("headerSize          %-8X\n", context->header.headerSize);
    printf("programHeader:\n");
    printf(" - entrySize        %-8d\n", context->header.programHeader.entrySize);
    printf(" - count            %-8d\n", context->header.programHeader.count);
    printf("sectionHeader:\n");
    printf(" - entrySize        %-8d\n", context->header.sectionHeader.entrySize);
    printf(" - count            %-8d\n", context->header.sectionHeader.count);
    printf(" - stringTableIndex %-8d\n", context->header.sectionHeader.stringTableIndex);
    printf("\n");

    if (context->programHeaders) {

        printf("ProgramHeaders:\n");
        printf("index type     offset     vaddr    paddr filesize  memsize flags align\n");
        for (int index = 0; index < context->header.sectionHeader.count; index++) {
            struct ELFProgramHeader *program = context->programHeaders + index;

            printf("%5X ", index);

            if (program->type < sizeof(programTypes)) {
                printf("%-6s ", programTypes[program->type]);
            } else{ 
                printf("%-6d ", program->type);
            }

            printf("%8X ", program->offset);
            printf("%8X ", program->virtualAddress);
            printf("%8X ", program->physicalAddress);
            printf("%9X ", program->sizeInFile);
            printf("%8X ", program->sizeInMemory);
            printf("%5X ", program->flags);
            printf("%5X ", program->alignment);

            if (program->flags & PHF_READ)
                printf(" READ");

            if (program->flags & PHF_WRITE)
                printf(" WRITE");

            if (program->flags & PHF_EXEC)
                printf(" EXEC");

            printf("\n");
        }
        printf("\n");
    }


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
        printf("index name                               address   size type    bind    other  index\n");
        struct ELFSymbolEntry *current = context->sections[index];
        struct ELFSymbolEntry *end = context->sections[index] + section->size;

        char *strtab = context->sections[section->link];

        for (int index = 0; current < end; index++, current++) {
            printf("%4X ", index);
            printf("%5X ", current->name);
            printf("%-30.30s ", strtab ? strtab + current->name : "");
            printf("%6X ", current->address);
            printf("%6X ", current->size);
            if (current->type < sizeof(symbolTypes)){
                printf("%-7s ", symbolTypes[current->type]);
            } else {
                printf("%-7X ", current->type);
            }

            if (current->bind < sizeof(symbolBinds)){
                printf("%-6s ", symbolBinds[current->bind]);
            } else {
                printf("%-6X ", current->bind);
            }

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

        struct ELFSectionHeader *text = context->sectionHeaders + section->info;
        uint8_t *code = context->sections[section->info];

        for (; current < end; current++) {
            uint32_t value = 0;

            if (code) {
                if (context->programHeaders) {
                    value = *(uint32_t*)(code + (current->offset - text->address));
                } else {
                    value = *(uint32_t*)(code + current->offset);
                }
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

/**
 * 
 */
struct ELFContext *elf_open(const char *path) {
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

    fclose(handle);
    return context;

    error:
    fclose(handle);
    elf_close(context);
    return 0;
}

/**
 * 
 */
void elf_close(struct ELFContext *context) {
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

/**
 * 
 */
int elf_save(struct ELFContext *context, const char *path) {
    FILE *handle = fopen(path, "w+");
    if(!handle){
        sprintf(error, "Failed open file for writing");
        return 0;
    }

    if(fwrite(&context->header, sizeof(struct ELFHeader), 1, handle) != 1) {
        sprintf(error, "Failed to write header");
        goto error;
    }

    if (context->sectionHeaders) {
        if (fseek(handle, context->header.sectionOffset, SEEK_SET) != 0) {
            sprintf(error, "Failed to seek to section headers");
            goto error;
        }

        if (fwrite(context->sectionHeaders, context->header.sectionHeader.entrySize, context->header.sectionHeader.count, handle) != context->header.sectionHeader.count) {
            sprintf(error, "Failed to write section headers (%d %d %d)",
                context->header.sectionOffset,
                context->header.sectionHeader.entrySize,
                context->header.sectionHeader.count
            );
            goto error;
        }


        for (uint32_t index = 0; index < context->header.sectionHeader.count; index++) {
            struct ELFSectionHeader *sectionHeader = context->sectionHeaders + index;

            if (fseek(handle, sectionHeader->offset, SEEK_SET) != 0) {
                sprintf(error, "Failed to seek to section");
                goto error;
            }

            if (fwrite(context->sections[index], 1, sectionHeader->size, handle) != sectionHeader->size) {
                sprintf(error, "Failed to write section");
                goto error;
            }
        }
    }

    fclose(handle);
    return 1;
    error:
    fclose(handle);
    return 0;
}

static inline void print_section(struct ELFContext *context, struct ELFSectionHeader *section) {
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

static inline int elf_section_apply_address(struct ELFContext *context, uint32_t sectionIndex, uint32_t origin) {
    struct ELFSectionHeader *section = context->sectionHeaders + sectionIndex;
    printf("%5X ", sectionIndex);
    print_section(context, section);
    printf("------------------------------------------------------------------------------------------------------\n");

    // - for each symbol table
    //   - for each symbol in symbol table for this section
    //     - find relocation table linked to this symbol table
    //       - for each relocation for symbol in relocation table
    //         - calculate new address
    //         - update value at offset of relocation entry
    for (uint32_t symtabIndex = 0; symtabIndex < context->header.sectionHeader.count; symtabIndex++) {
        struct ELFSectionHeader *symtab = context->sectionHeaders + symtabIndex;

        if (symtab->type != SHT_SYMTAB)
            continue;

        char *strtab = context->sections[symtab->link];

        uint32_t symbolCount = symtab->size / symtab->entrySize;

        struct ELFSymbolEntry *symbols = context->sections[symtabIndex];

        for (uint32_t symbolIndex = 0; symbolIndex < symbolCount; symbolIndex++) {
            struct ELFSymbolEntry *symbol = symbols + symbolIndex;
            if(symbol->index != sectionIndex)
                continue;

            printf("%4X ", symbolIndex);
            printf("%5X ", symbol->name);
            printf("%-30.30s ", strtab ? strtab + symbol->name : "");
            printf("%6X ", symbol->address);
            printf("%6X ", symbol->size);
            printf("%4X ", symbol->type);
            printf("%4X ", symbol->bind);
            printf("%6X ", symbol->other);
            printf("%6X ", symbol->index);
            printf("\n");

            //--
            for (uint32_t reltabIndex = 0; reltabIndex < context->header.sectionHeader.count; reltabIndex++) {
                struct ELFSectionHeader *reltab = context->sectionHeaders + reltabIndex;

                if (reltab->type != SHT_REL)
                    continue;

                if (reltab->link != symtabIndex)
                    continue;
                
                printf("--- ");
                print_section(context, reltab);

                uint8_t *code = context->sections[reltab->info];
                struct ELFRelocation *relocations = context->sections[reltabIndex];

                uint32_t relCount = reltab->size / reltab->entrySize;
                for (uint32_t relIndex = 0; relIndex < relCount; relIndex++) {
                    struct ELFRelocation *relocation = relocations + relIndex;
                    if(relocation->symbol != symbolIndex)
                        continue;
                    
                    uint32_t value = 0;

                    if (code){
                        value = *(uint32_t*)(code + relocation->offset);
                    }
                    printf("--- ");
                    printf("%6X ", relocation->offset);
                    //printf("%6X ", relocation->symbol);
                    printf("%4X ", relocation->type);
                    printf("%8X ", value);

                    switch (relocation->type) {
                        case 1:
                            value+= origin; 
                        break;
                        case 2:
                            value = origin + symbol->address; 
                        break;
                    }
                    printf("%8X ", value);
                    printf("\n");

                    // YOLO
                    (*(uint32_t*)(code + relocation->offset)) = value;
                }
            }
            printf("\n");
        }
    }

    return 1;
}

/**
 * 
 */
int elf_relocate(struct ELFContext *context, uint32_t origin, uint32_t alignment){
    for (uint32_t index = 0; index < context->header.sectionHeader.count; index++) {
        struct ELFSectionHeader *section = context->sectionHeaders + index;
        if (!(section->flags & SHF_ALLOC))
            continue;

        if(!section->size)
            continue;

        uint32_t align = alignment;
        if(section->alignment > align)
            align = section->alignment;
        
        // Align origin to alignmenrs
        origin+= (align - ((origin - 1) % align)) - 1;

        if(!elf_section_apply_address(context, index, origin))
            return 0;

        // Now we can update
        section->address = origin;

        // Move origin to the end of current section, so next one will start after this one
        origin+= section->size;


        printf("\n");
    }

    return 1;
}
