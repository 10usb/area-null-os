#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "elf.h"

/**
 * Print the help info
 *
 * @param[in]  value  The value
 *
 * @return     Value given to this function
 */
static int print_help(int value) {
    printf("Usage:\n");
    printf(" linker help\n");
    printf(" linker inspect <source>\n");
    printf(" linker alter [options] <source> [target]\n");
    printf(" linker merge [options] <target> <sources> ...\n");
    printf("\n");
    printf("Options\n");
    printf(" --entry <name>         Entry point\n");
    printf(" --origin <offset>      Location in memory the file is loaded\n");
    printf(" --format <format>      Output format (binary, elf, coff..)\n");
    return value;
}


const char *options_string[] = {
    "--entry",
    "--origin",
    "--format",
    0 // null terminated
};

enum {
    FORMAT_ELF = 1,
    FORMAT_BINARY = 2,
    FORMAT_ORC = 3,
};

const char *formats_string[] = {
    "elf",
    "binary",
    "orc",
    0 // null terminated
};

static inline int get_index(const char **list, const char *arg){
    for (int index = 0; list[index]; index++) {
        if (strcmp(arg, list[index]) == 0)
            return index + 1;
    }

    return 0;
}

struct Options {
    const char *entry;
    uint32_t origin;
    uint32_t format;
};

static inline int get_options(int *index, int argc, const char** argv, struct Options *options) {
    memset(options, 0, sizeof(struct Options));
    options->format = FORMAT_ELF;
    options->origin = 0xFFFFFFFF;
    
    uint32_t i;
    for (; *index < argc; (*index)++) {
        switch (get_index(options_string, argv[*index])) {
            case 1: options->entry = argv[++(*index)]; break;
            case 2:
                if (!sscanf(argv[++(*index)], "%x", &i)) {
                    printf("Failed to parse <origin> %s\n", argv[*index]);
                    return print_help(1);
                }
                options->origin = i;
            break;
            case 3:
                i = get_index(formats_string, argv[++(*index)]);
                if (i == 0) {
                    printf("Unknown format '%s'\n", argv[*index]);
                    return print_help(1);
                }
                options->format = i;
            break;
            default:
                if(strcmp(argv[*index], "--") != 0)
                    goto end;
                
                printf("Unknown option '%s'\n", argv[*index]);
                return print_help(1);
        }
    }

    end:
    return 0;
}

/**
 * Print info about an object
 */
int main_inspect(int argc, const char** argv){
    if (argc < 1){
        printf("Not enough arguments given\n");
        return print_help(1);
    }

    struct ELFContext *context = elf_open(argv[0]);
    if (!context){
        printf("Failed to open '%s'\n\t%s\n", argv[0], elf_last_error());
        return 1;
    }

    elf_print(context);
    elf_close(context);
    return 0;
}

/**
 * Change properties of an object
 */
int main_alter(int argc, const char** argv){
    struct Options options;
    int index = 0, result;
    const char *source, *target;

    if((result = get_options(&index, argc, argv, &options)) != 0)
        return result;

    if (argc - index < 1) {
        printf("No source given (%d,%d)\n", index, argc);
        return print_help(1);
    }

    source = target = argv[index++];

    if (argc - index > 0)
        target = argv[index++];

    printf("entry: %s\n", options.entry);
    printf("origin: %X\n", options.origin);
    printf("format: %d\n", options.format);
    printf("source: %s\n", source);
    printf("target: %s\n", target);

    struct ELFContext *context = elf_open(source);
    if (!context){
        printf("Failed to open '%s'\n\t%s\n", source, elf_last_error());
        return 1;
    }

    if (options.origin != 0xFFFFFFFF) {
        elf_relocate(context, options.origin, 0x1000);
    }

    //elf_print(context);

    if(!elf_save(context, target)){
        printf("Failed to save '%s'\n\t%s\n", target, elf_last_error());
    }
    elf_close(context);
    return 0;
}

/**
 * Merge two or more objects into one
 */
int main_merge(int argc, const char** argv){
    printf("Not yet implemented\n");
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
int main(int argc, const char** argv){
    if (argc < 2){
        printf("No arguments given\n");
        return print_help(1);
    }

    if (strcmp(argv[1], "help") == 0)
         return print_help(0);

    if (strcmp(argv[1], "inspect") == 0)
         return main_inspect(argc - 2, argv + 2);

    if (strcmp(argv[1], "alter") == 0)
         return main_alter(argc - 2, argv + 2);

    if (strcmp(argv[1], "merge") == 0)
         return main_merge(argc - 2, argv + 2);

    printf("Unknown command '%s'\n", argv[1]);
    return print_help(1);
}