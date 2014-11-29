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
			fseek(f, addr, SEEK_SET);	
			return addr;
		} else {
			return 0;
		}
	}
	else {
		// indirect link
		if (i->i_block[n]) {
		    fprintf(stderr, "can't cope with indirect blocks\n");
	    	exit(1);
        } else{
            return 0;
        }
	}
}

inode *load_inode(FILE *f, int ino){
	// fseek to location
    // we only ever have one group block so always seek
    // to the bg_inode_bitmap of the root bg.
    fseek(f, 
            block_addr(blockgroup_list->bg_inode_table) + 
            (ino-1) * superblock_root->s_inode_size,
			SEEK_SET);

    //copy that indoe into memory
	inode *new_inode = malloc(
			superblock_root->s_inode_size * sizeof(char));
	fread(new_inode, sizeof(char), superblock_root->s_inode_size, f);
   
    return new_inode;

}



//aggregates the different disk blocks of an inode
//into a single buffer
void *aggregate_file(FILE *f, inode *i){ 
	int index = 0, x;
	char *dirbuf = malloc(sizeof(char) * inode_numblocks(i) * c_block_size);
	for(x=0; x<15; x++) {
        if (inode_seek_nth_block(f, i, x )){
		    fread(dirbuf + (c_block_size * index), 
		    		sizeof(char), c_block_size, f);
            index++;
        }
    }
    return (void *)dirbuf;

}

directory_node *next_node(directory_node *d){
    return (directory_node *) ( ((char *)d) + d->d_rec_len);
}






int block_addr(int blockaddr){
	return 1024 + c_block_size * (blockaddr - 1);
}
