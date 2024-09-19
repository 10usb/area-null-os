#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Print the help info
 *
 * @param[in]  value  The value
 *
 * @return     Value given to this function
 */
static int print_help(int value) {
    printf("Usage:\n");
    printf(" linker [options] <target> <sources> ...\n");
    printf("  --entry <name>        Entry point\n");
    printf("  --origin <offset>     Location in memory the file is loaded\n");
    printf("  --format <format>     Output format (binary, elf, coff..)\n");
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
 * Main entry for program
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
int main(int argc, const char** argv){
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
        return print_help(0);

    const char *entry;
    const char *origin;
    const char *format;
    const char *target;
    const char **sources;
    
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
    // TODO expand sources with glob search

    printf("BUILDING target\n");
    return 0;
}