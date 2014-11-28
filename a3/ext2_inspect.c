#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern superblock *superblock_root;
extern descriptor *blockgroup_list;

extern bool c_one_bg;
extern uint c_block_size;
extern uint c_num_block_groups;


void inspect_bitmap(FILE *f) {
    int i, x, mask;
	char *bitmap = malloc(sizeof(char) * c_block_size);
	fread(bitmap, sizeof(char), c_block_size, f);

	for (i=0 ; i<c_block_size; i++) {
		mask=128;
        for (x=0; x<8; x++) {
            if (!(bitmap[i] & mask)) {printf("%d, ", i*8 + x);}
            mask = mask / 2;
		}
	}
}

void inspect_block(descriptor *bgl, FILE *f, int index) {	
	descriptor * bg = bgl+index;
	printf("bgl=%p, bg=%p\n", bgl, bg);

	printf("inspecting block %d\n", index);


	pfield(bg, bg_block_bitmap);
	pfield(bg, bg_inode_bitmap);
	pfield(bg, bg_inode_table);
	printf("\n");
	pfields(bg, bg_free_blocks_cont);
	pfields(bg, bg_free_inodes_cont);
	pfields(bg, bg_used_dirs_cont);

	fseek(f, (bg->bg_inode_bitmap * c_block_size), SEEK_SET);
    printf("inodes ");
    inspect_bitmap(f);
    printf("\b\b in use \n");
}

void inspect_blocks(descriptor *bgl, FILE *f) {
	int i;
	if (!c_one_bg) {
		printf("number of block groups: %d\n", c_num_block_groups);
		for(i=0; i<c_num_block_groups; i++) {
			inspect_block(bgl, f, i);
		}
	} else {
		printf("only one block\n");
		inspect_block(bgl, f, 0);
	}
}	

int main (int argc, char** argv) {
	if (argc != 2) {
		printf("Usage is ext2_inspect <disk_img>\n");
		return 1;
	} else {
		FILE *f = fopen(argv[1], "r");
		init_ext2lib(f);
		inspect_blocks(blockgroup_list, f);
	}
	return 0;
}

