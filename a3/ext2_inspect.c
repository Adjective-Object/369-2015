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


void inspect_directory(FILE *f, inode *i) {
	printf("inspecting directory %d\n", i->i_uid); 
	
	int x;
	printf("used disk blocks: ");
	for (x=0; x<15; x++) {
		printf("%d ", i->i_block[x] );
	} printf("\n");
	
	int index = 0;
	char *dirbuf = malloc(sizeof(char) * inode_numblocks(i) * c_block_size);
	while (inode_seek_nth_block(f, i, index )){
		fread(dirbuf + (c_block_size * index), 
				sizeof(char), c_block_size, f);
		index++;
	}
	printf("directory contents size: %d\n", c_block_size * index);
	print_hex(dirbuf, sizeof(char) * inode_numblocks(i) * c_block_size);
}

void inspect_inode(FILE *f, int inode_table_offset, int ino) {
	//load inode into memory
	printf("offset=%d, number=%d, inode_size=%d\n",
			inode_table_offset, ino, superblock_root -> s_inode_size);
	
	fseek(f, inode_table_offset + (ino-1) * superblock_root->s_inode_size,
			 SEEK_SET);
	inode *cur_inode = load_inode(f);

	print_hex(cur_inode, sizeof(inode));

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

	printf("\n");
	// check out the inode bitmap
	fseek(f, (block_addr(bg->bg_inode_bitmap)), SEEK_SET);
	inspect_bitmap(f);

	printf("\n");
	// inspect the root inode
	inspect_inode(f, block_addr(bg->bg_inode_table), 2);
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

