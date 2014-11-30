#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern uint c_block_size;

void cp_to_dir(FILE *input, FILE *img,  char *name, inode *dest){
	//get the size of the file
	fseek(input, 0, SEEK_END);
	int i, fsize = ftell(input);

	int fsize_blocks = fsize / c_block_size + (fsize % c_block_size != 0);
	char *buffer = malloc(sizeof(char) *c_block_size);

	// find a new inode and allocate to it the blocks for a new file
	inode *new_inode = make_file_inode(fsize);

	// read the file in one block at a time and copy it in
	fseek(input, 0, SEEK_SET);
	for(i=0; i<fsize_blocks; i++){
		fread(buffer, sizeof(char), c_block_size, input);
		
		// seek to the proper data block and write it
		// TODO fix nth block to handle files that use indirect nodes
		inode_seek_nth_block(img, new_inode, i);
		fwrite(buffer, sizeof(char), c_block_size, img);
	}

	make_hardlink(img, name, dest, new_inode);

	// update the image to changes in the inode table, bitmaps, superblock
	update_image(img);
}

int main(int argc, char ** argv) {
	if (argc != 3){
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

	// find the destination directory and name
	inode *dest = get_inode_for(img ,argv[3]);
	char *name = get_last_in_path(argv[3]);
	
	if (dest == NULL || inode_type(dest) != INODE_MODE_DIRECTORY) {
		dest = get_inode_for(img, pop_last_from_path(argv[3]) );
		name = get_last_in_path(argv[2]);
	}

	// insert with proper name into the directory
	cp_to_dir(fil, img, name, dest);
	
	return 0;
}
