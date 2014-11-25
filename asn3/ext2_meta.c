#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"


int main (int argc, char** argv) {
	if (argc != 2) {
		printf("Usage is ext_meta <disk_img>\n");
		return 1;
	} else{
		FILE *disk_f = fopen(argv[1], "r");
        fseek(disk_f, 1024, SEEK_SET); // seek to the first superblock

		parse_super(disk_f);

		printf("s_inodes_count: %d\n", s_inodes_count);
	}
	return 0;
}

