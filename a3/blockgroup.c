#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "superblock.h"
#include "ext2.h"


void load_blockgroup(descriptor *dest, FILE *f, int location){
	// read a new superblock and compare it to the root.
	// if they are not the same, panic and exit

	printf("loading blockgroup at i=%d\n", location);

	// TODO change handling  depending on if we are using revision 0 or 1
	// version of the ext2 filesystem (revision 1 has less redundancy)
	
	superblock *new_super_block;	

	fseek(f, location, SEEK_SET);
	new_super_block = parse_super(f);

	if (memcmp(superblock_root, new_super_block, sizeof(superblock))) {
		fprintf(stderr, "superblocks not the same, exiting!\n");
		exit(1);
	}

	free(new_super_block);

	// seek past the superblock and read the block descriptor
	fseek(f, location + c_block_size, SEEK_SET);
	fread(dest, sizeof(descriptor), 1, f);
}
