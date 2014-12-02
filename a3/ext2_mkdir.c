#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;

void mk_dir(FILE *input,  char *name, inode *dest_dir){
	printf("cp %.*s to inode %d\n", (int)strlen(name), name, dest_dir->i_uid);
	
	//get the size of the file
	fseek(input, 0, SEEK_END);
	int i, fsize = ftell(input);

	int fsize_blocks = fsize / c_block_size + (fsize % c_block_size != 0);
	char *buffer = malloc(sizeof(char) *c_block_size);

	// find a new inode and allocate to it the blocks for a new file
	int new_inode = make_file_inode(fsize);

	// read the file in one block at a time and copy it into the
	// appropriate data blocks
	fseek(input, 0, SEEK_SET);
	for(i=0; i<fsize_blocks; i++){	
		fread(buffer, sizeof(char), c_block_size, input);

		void *block = inode_nth_block_ptr(get_inode(new_inode), i);
		memcpy(block, buffer, c_block_size);
	}

	printf("file copied, creating hardlink\n");
	make_hardlink(name, dest_dir, new_inode);
}

int main(int argc, char ** argv) {
	if (argc != 4){
		fprintf(stderr, 
				"Usage is ext2_cp <image> <file> <path_to_destination>\n");
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

	FILE  *fil = fopen(argv[2], "r");
	if (!fil) {
		fprintf( stderr, "Error opening file \"%.*s\" for reading",
				(int) strlen(argv[2]),
				argv[2]);
		return 1;
	}
	
	printf("img: %.*s \n", (int)strlen(argv[1]), argv[1]);
	printf("file: %.*s \n", (int)strlen(argv[2]), argv[2]);

	init_ext2lib(img);

	// find the destination directory and name
	inode *dest = get_inode_for(argv[3]);
	char *name = get_last_in_path(argv[2]);
	
	if (dest == NULL || inode_type(dest) != INODE_MODE_DIRECTORY) {
		dest = get_inode_for(pop_last_from_path(argv[3]) );
		name = get_last_in_path(argv[3]);
	}

	// insert with proper name into the directory
	cp_to_dir(fil,  name, dest);

	teardown_ext2lib();

	return 0;
}
