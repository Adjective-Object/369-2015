#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;

int main(int argc, char ** argv) {
	if (argc != 3){
		fprintf(stderr, 
				"Usage is ext2_traverse <image> <path>\n");
		return 1;
	}

	//load files, check for opening errors
	FILE	*img = fopen(argv[1], "r+");
	if (!img) {
		fprintf( stderr, "Error opening image file \"%.*s\"",
				(int) strlen(argv[1]),
				argv[1]);
		return 1;
	}

	init_ext2lib(img);

    char *next_in_path = get_next_in_path(argv[2]);
    char *last_in_path = get_last_in_path(argv[2]);

    char *first_in_path = get_next_in_path(argv[2]);
    char *shorter_path = pop_last_from_path(argv[2]);

    char *path = argv[2];

    pstr(path);

    pstr(next_in_path);
    pstr(last_in_path);
    pstr(first_in_path);
    pstr(shorter_path);
    
	inode *dest = get_inode_for(argv[2]);
    printf("found inode %d\n", dest->i_uid);

	teardown_ext2lib();

	return 0;
}
