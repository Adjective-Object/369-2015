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

int inode_numblocks(inode *ino){
	return ino->i_blocks / (c_block_size / 512);
}

// returns 0 if there are no more blocks, else returns address of thhe
// block on the disk
int inode_seek_nth_block(FILE *f, inode *i, int n) {
	if (n<12) {
		// direct link
		if (i->i_block[n]) {
			int addr = block_addr(i->i_block[n]);
			printf("address: %d\n", addr);
			fseek(f, addr, SEEK_SET);	
			return addr;
		} else {
			return 0;
		}
	}
	else {
		// indirect block
		fprintf(stderr, "can't cope with indirect blocks\n");
		exit(1);
	}
}

inode *load_inode(FILE *f){
	//copy that indoe into memory
	inode *new_inode = malloc(
			superblock_root->s_inode_size * sizeof(char));
	fread(new_inode, sizeof(char), superblock_root->s_inode_size, f);
	return new_inode;
}





directory *load_dir(void *f){
	void *head = f;
	directory *dir = NULL, pdir = NULL;

	do {
		dir = malloc(sizeof(directory));
		memcpy(dir, head, malloc(sizeof(directory) - 2* sizeof(char*))
		dir->name = malloc(sizeof(char) * name_len);
		dir->next = NULL;
		
		if (pdir != NULL)
			pdir->next = dir;
		pdir = dir;

	} while (*((uint *) dir) != 0);
}







int block_addr(int blockaddr){
	return 1024 + c_block_size * (blockaddr - 1);
}
