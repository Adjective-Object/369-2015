#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern superblock *superblock_root;
extern descriptor *blockgroup_list;

extern bool c_one_bg;
extern uint c_block_size;
extern uint c_num_block_groups;


void inspect_directory(FILE *f, inode *i) {
	int x;
	printf("used blocks: ");
    for (x=0; x<15; x++) {
		printf("%d ", i->i_block[x] );
	} printf("\n");
	
    directory_node *d = aggregate_file(f, i);
    directory_node *first_d = d;

    while (d->d_inode_num != 0){
        printf("%d:\t", d->name_len);
        printf(d->name);
        d = next_node(d);
        printf("\n");
    }

    d = first_d; 
    while (d->d_inode_num != 0) {
	    if (d->name[0] != '.'){
            printf("going into subdirectory %.*s\n", d->name_len, d->name);
	        
            inode *sub_inode = load_inode(f, d->d_inode_num);
            inspect_directory(1, sub_inode);
        }

        d = next_node(d);
    }
}

void inspect_inode(FILE *f, int ino) {
	//load inode into memory
	printf("number=%d, inode_size=%d\n",
			ino, superblock_root -> s_inode_size);
	
	inode *cur_inode = load_inode(f,ino);

	// print what type it is
	pfields(cur_inode, i_uid);
	pfields(cur_inode, i_mode);
	pfield(cur_inode, i_flags);

	ushort ftype = ((cur_inode->i_mode) & 0xF000);
	switch (ftype) {
		case INODE_MODE_DIRECTORY:
			inspect_directory(f, cur_inode);
			break;
		case INODE_MODE_FILE:
			printf("\tinode %d is a file\n",
					cur_inode->i_uid);
			break;
		default:
			printf("no method to handle file mode type %x\n, (inode %d)", 
					ftype, cur_inode->i_uid);
	}

}

void inspect_bitmap(FILE *f) {
    int i, x;
    
	char *bitmap = malloc(sizeof(char) * c_block_size);
	unsigned char  mask;
	fread(bitmap, sizeof(char), c_block_size, f);

	swap_endian_on_block(bitmap, sizeof(char) * c_block_size);

	printf("inodes in use: ");
	for (i=0 ; i<c_block_size; i++) {
		mask=128;
        for (x=0; x<8; x++) {
            if (!(bitmap[i] & mask)) {
				printf("%d, ", i*8 + x);
				/* debug print for matching mask properly.
				printf("\n%d\t(" BYTE2BIN_PAT ", " BYTE2BIN_PAT ", %d)",
						(i*8 + x), 
						BYTE2BIN(bitmap[i]),
						BYTE2BIN(mask),
						mask);
				*/
			}
            mask = mask / 2;
		}
	}
	printf("\b\b\n");
}

void print_num(int ino){
	printf("%d, ", ino);
}

void inspect_blockgroup(descriptor *bgl, FILE *f, int index) {	
	descriptor * bg = bgl+index;
	printf("bgl=%p, bg=%p\n", bgl, bg);

	printf("inspecting block group %d\n", index);

	pfield(bg, bg_block_bitmap);
	pfield(bg, bg_inode_bitmap);
	pfield(bg, bg_inode_table);
	printf("\n");

	pfields(bg, bg_free_blocks_cont);
	pfields(bg, bg_free_inodes_cont);
	pfields(bg, bg_used_dirs_cont);

	printf("\n");
	// check out the inode bitmap
	fseek(f, (block_addr(bg->bg_inode_bitmap)), SEEK_SET);
	inspect_bitmap(f);

	printf("\n");
    // c
	// inspect the root inode
	inspect_inode(f, 2);
}

void inspect_blocks(descriptor *bgl, FILE *f) {
	int i;
	if (!c_one_bg) {
		printf("number of block groups: %d\n", c_num_block_groups);
		for(i=0; i<c_num_block_groups; i++) {
			inspect_blockgroup(bgl, f, i);
		}
	} else {
		printf("only one block group\n");
		inspect_blockgroup(bgl, f, 0);
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

