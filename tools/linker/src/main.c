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

const char *options[] = { "--entry", "--origin", "--format" };

static inline int get_option(const char *arg){
    for (int index = 0; index < 3; index++) {
        if (strcmp(arg, options[index]) == 0)
            return index + 1;
    }

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

    struct ELFContext *context = elfopen(argv[0]);
    if (!context){
        printf("Failed to open '%s'\n\t%s\n", argv[0], elflasterror());
        return 1;
    }

    elfprint(context);
    elfclose(context);
    return 0;
}

/**
 * Change properties of an object
 */
int main_alter(int argc, const char** argv){
    printf("Not yet implemented\n");
    return 1;
}

/**
 * Merge two or more objects into one
 */
int main_merge(int argc, const char** argv){
    printf("Not yet implemented\n");
    const char *entry;
    const char *origin;
    const char *format;
    const char *target;
    const char **sources;
    int sourcesCount;
    
    int index = 1;
    for (; index < argc; index++) {
        switch (get_option(argv[index])) {
            case 1: entry = argv[++index]; break;
            case 2: origin = argv[++index]; break;
            case 3: format = argv[++index]; break;
            default:
                if(strcmp(argv[index], "--") != 0)
                    goto TARGET;
                
                printf("Unknown option '%s'\n", argv[index]);
                return print_help(1);
        }
    }

    TARGET:
    if (argc - index < 2) {
        printf("No target or sources given\n");
        return print_help(1);
    }

    target = argv[index++];
    sources = argv + index;
    sourcesCount = argc - index;

    printf("Target: %s\n", target);
    printf("---\ncount %d\n", sourcesCount);
    for (int i = 0; i < sourcesCount; i++)
        printf("source: %s\n", sources[i]);

    readelf(sources[3]);
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