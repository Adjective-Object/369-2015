#ifndef EXT2_BLOCKGROUP_369
#define EXT2_BLOXKGROUP_369

#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"
#include "ext2.h"

#define pfields(strut, field_name) {\
	printf("\t" #field_name ": %hu\n",\
			strut->field_name );\
}

#define pfield(strut, field_name) {\
	printf("\t" #field_name ": %u\n",\
			strut->field_name );\
}

struct descriptor {
	uint   bg_block_bitmap;         // blockID of the data bitmap
	uint   bg_inode_bitmap;         // blockID of the inode bitmap
	uint   bg_inode_table;          // blockID of the inode table

	ushort bg_free_blocks_cont;     // # of free data blocks
	ushort bg_free_inodes_cont;     // # of free inodes
	ushort bg_used_dirs_cont;       // # inodes used for directories
} __attribute__((packed));

typedef struct descriptor descriptor;

// list of block group descriptors, initialized by init_ext2lib;
descriptor *blockgroup_list;

// loads a block group starting from the current location of the file.
// checks the superblock loaded against superblock_root, the fist superblock
// seen. If they are differnet, panic and fail
descriptor *load_blockgroup(FILE *f, int location);

#endif
