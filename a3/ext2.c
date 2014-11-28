#include "ext2.h"
#include "superblock.h"
#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void swap_endian_on_field(void *addr, uint32_t size) {
	int i;
	char tmp;
	char *iter = addr;

	for(i=0; i<size/2; i++) {
		tmp = iter[i];
		iter[i] = iter[size-i-1];
		iter[size-1-i] = tmp;
	}
}

// swaps the endianness of a field
void swap_endian_on_block(void *addr, uint32_t size) {
	int i;
	char tmp;
	char *iter = addr;

	for(i=0; i<size/2; i++) {
		tmp = iter[1];
		iter[1] = iter[0];
		iter[0] = tmp;
		
		iter = iter + 2;
	}
}


// intializtion of constants
extern char is_little_endian;
extern descriptor *blockgroup_list;

extern uint c_num_block_groups;
extern uint c_block_size;
extern uint c_bg_size;
extern bool c_one_bg;

void print_hex(void *bin, size_t size){
	short *c = bin;
	int i;

	for (i=0; i<size/2; i++) {
		if (i%8 == 0) {
			printf("\n");
		}
		printf("%04hx ", (unsigned short) (*c) );
		c++;
	}
	printf("\n");
}

void init_ext2lib(FILE *f) { 
	//init endianness
	unsigned int x = 1;
	// cast to a char ptr and get the first char.
	// if is little endian, will be
	// [0001] 0000 0000 0000
	// not 0000 0000 0000 0001
	is_little_endian = (((short *)&x)[0]);

	//load the root block;
	fseek(f, 1024, SEEK_SET); // seek to the beginning of the first superblock
	superblock_root = parse_super(f);

	//print_hex(superblock_root, sizeof(superblock));

	c_block_size = 1024 << (superblock_root->s_log_block_size);
	c_bg_size = c_block_size * superblock_root->s_blocks_per_group;

	//load each blockgroup into the list of blockgroups)

	c_num_block_groups = superblock_root->s_blocks_count /
		superblock_root->s_blocks_per_group;
	c_one_bg = (c_num_block_groups == 0);

	//pfield(superblock_root,s_blocks_count);
	//pfield(superblock_root,s_blocks_per_group);

	//load the blockgroups in to memory
	blockgroup_list = malloc(sizeof(descriptor) * 
			((c_one_bg) ? 1 : (c_num_block_groups)) );
	fseek(f,1024 + SUPERBLOCK_SIZE, SEEK_SET);
	
	int i=0;
	do {
		load_blockgroup(blockgroup_list + i, f, 1024+i*c_bg_size);
		i++;
	} while (i<c_num_block_groups);
}


