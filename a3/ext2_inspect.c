#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern superblock *superblock_root;
extern descriptor *blockgroup_list;

extern uint c_block_size;
extern uint c_num_block_groups;


void inspect_bitmap(FILE *f, char*formatstring){
	int i, mask, mcnt;
	char buf;
	for (i=0 ;i< c_block_size; i++) {
		fread(&buf, sizeof(char), 1, f);
		mcnt = 0;
		for (mask=1; mask<=128; mask=mask*2) {
			if (buf && mask) {
				printf(formatstring, i*8 + mcnt);
			}
			mcnt++;
		}
	}
}

void inspect_block(descriptor *bgl, FILE *f, int index) {	
	descriptor * bg = bgl+index;
	
	pfield(bg, bg_block_bitmap);
	pfield(bg, bg_inode_bitmap);
	pfield(bg, bg_inode_table);
	printf("\n");
	pfields(bg, bg_free_blocks_cont);
	pfields(bg, bg_free_inodes_cont);
	pfields(bg, bg_used_dirs_cont);

	fseek(f, bg->bg_inode_bitmap, SEEK_SET);
	inspect_bitmap(f, "inode %s in use\n");

	fseek(f, bg->bg_block_bitmap, SEEK_SET);
}

void inspect_blocks(descriptor *bgl, FILE *f) {
	int i;
	printf("number of block groups: %d\n", c_num_block_groups);
	for(i=0; i<c_num_block_groups; i++) {
		inspect_block(bgl, f, i);
	}
}	

int main (int argc, char** argv) {
	if (argc != 2) {
		printf("Usage is ext_meta <disk_img>\n");
		return 1;
	} else {
		FILE *f = fopen(argv[1], "r");
		init_ext2lib(f);

		// inspect used blocks and stuff
		inspect_blocks(blockgroup_list, f);
	}
	return 0;
}

