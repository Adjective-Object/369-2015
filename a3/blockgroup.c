#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "superblock.h"
#include "ext2.h"


descriptor *load_blockgroup(FILE *f, uint location){
	// read a new superblock and compare it to the root.
	// if they are not the same, panic and exit

	// TODO change handling  depending on if we are using revision 0 or 1
	// version of the ext2 filesystem (revision 1 has less redundancy)
	
	superblock *new_super_block = malloc(sizeof(descriptor));	

	fseek(f, location, SEEK_SET);
	fread(new_super_block, sizeof(superblock), 1, f);

	if (memcmp(superblock_root, new_super_block, sizeof(superblock))) {
		fprintf(stderr, "superblocks not the same, exiting!");
		exit(1);
	}

	free(new_super_block);


	// seek past the superblock and read the block descriptor
	descriptor *desc = malloc(sizeof(descriptor));
	fseek(f, location + SUPERBLOCK_SIZE, SEEK_SET);
	fread(desc, sizeof(descriptor), 1, f);

	return desc;
}
