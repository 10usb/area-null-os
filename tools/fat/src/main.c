#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/posix.h>
#include <fs/fat.h>

/**
 * Print the help info
 *
 * @param[in]  value  The value
 *
 * @return     Value given to this function
 */
static inline int main_help(int value){
	printf("Usage:\n");
	printf(" fat help\n");
	printf(" fat info <file>\n");
	printf(" fat create <file> <sectors> [options]\n");
	printf("  -sN    Number of bytes per sector\n");
	printf("  -TN    Number of sectors per track\n");
	printf("  -HN    Number of heads\n");
	printf("  -rN    Number of sectors for the reservedsectors (min 1)\n");
	printf("  -FN    Number of FAT copies\n");
	printf("  -fN    Number of sectors per FAT\n");
	printf("  -eN    Maximum number of root entries\n");
	printf("  -hN    Number of hidden sectors preceding the filesystem\n");
	printf("  -cN    Number of sectors per cluster\n");
	printf(" fat list <file> [path]\n");
	printf(" fat store <file> <path> <source>\n");
	printf(" fat load <file> <path> <destination>\n");
	printf(" fat remove <file> <path>\n");
	return value;
}

/**
 * Main entry for program
 *
 * @param[in]  argc  The argc
 * @param      argv  The argv
 *
 * @return     program exit code
 */
int main(int argc, char** argv){
	if (argc < 2 )
        return main_help(1);

	if (strcmp(argv[1], "help") == 0) {
 		return main_help(0);
	}

	printf("Unknown command '%s'\n", argv[1]);
	return main_help(1);
}