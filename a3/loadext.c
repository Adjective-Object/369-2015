#include <stdio.h>
#include <stdlib.h>
#include "ext2.h"

extern int 


int main (int argc, char** argv) {
	if (argc != 2) {
		printf("Usage is ext_meta <disk_img>\n");
		return 1;
	} else{
		FILE *disk_f = fopen(argv[1], "r");
        fseek(disk_f, 1024, SEEK_SET); // seek to the first superblock
	
		parse_super(disk_fp); // load the static shared vars
	}
	return 0;
}

