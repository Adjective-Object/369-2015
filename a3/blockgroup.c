#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "superblock.h"
#include "ext2.h"
#include <time.h>


void load_blockgroup(blockgroup *bg, FILE *f, int location){
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
	fread(&(bg->desc), sizeof(descriptor), 1, f);
	
	//allocate and read the bitmaps
	bg->block_bitmap = malloc(sizeof(char) * c_block_size);
	bg->inode_bitmap = malloc(sizeof(char) * c_block_size);
	bg->inode_table  = malloc(sizeof(char) * c_block_size);

	fseek(f, bg->desc.bg_block_bitmap  * c_block_size, SEEK_SET); 
	fread(bg->block_bitmap, sizeof(char), c_block_size, f);

	fseek(f, bg->desc.bg_inode_bitmap * c_block_size, SEEK_SET); 
	fread(bg->inode_bitmap, sizeof(char), c_block_size, f);
	
	fseek(f, bg->desc.bg_inode_table   * c_block_size, SEEK_SET); 
	fread(bg->inode_table, sizeof(char), c_block_size, f);

	// swap the endianess of the bitmaps
	swap_endian_on_block(bg->block_bitmap, sizeof(char) * c_block_size);
	swap_endian_on_block(bg->inode_bitmap, sizeof(char) * c_block_size);
}

int inode_numblocks(inode *ino){
	return ino->i_blocks / (c_block_size / 512);
}

// seeks f to the nth root-level block on the disk
int inode_seek_nth_block(FILE *f, inode *i, int n) {
	// direct link
	if (i->i_block[n]) {
		int addr = block_addr(i->i_block[n]);
		fseek(f, addr, SEEK_SET);	
		return addr;
	} else {
		return 0;
	}
}

inode *load_inode(FILE *f, int ino){
	// fseek to location
    // we only ever have one group block so always seek
    // to the bg_inode_bitmap of the root bg.
    fseek(f, 
            block_addr(blockgroup_list->desc.bg_inode_table) + 
            (ino-1) * superblock_root->s_inode_size,
			SEEK_SET);

    //copy that inode into memory
	inode *new_inode = malloc(
			superblock_root->s_inode_size * sizeof(char));
	fread(new_inode, sizeof(char), superblock_root->s_inode_size, f);
   
    return new_inode;

}

int inode_type(inode *i) {
	return ((i->i_mode) & 0xF000);
}


//aggregates the different disk blocks of an inode
//into a single buffer
void *aggregate_file(FILE *f, inode *i){ 
	int index = 0, x, block;
	char *dirbuf = malloc(sizeof(char) * inode_numblocks(i) * c_block_size);
	for(x=0; x<12; x++) {
        if ((block=inode_seek_nth_block(f, i, x ))) {
		    printf("seeked to x=%d, block=%d\n",x, block);
			fread(dirbuf + (c_block_size * index), 
		    		sizeof(char), c_block_size, f);
            index++;
        }
    }
	// the first indirect block
	if (i->i_block[12]) {
		//read the indirect block
		inode_seek_nth_block(f,i,12);
		uint *indirbuf = malloc(sizeof(char) * c_block_size);
		fread(indirbuf, sizeof(char), c_block_size, f);
		
		// scan the indirect block for nodes, appending them to the
		// block buffer as you go.
		for (x=0; x<c_block_size/4; x++) {
			if(indirbuf[x]) {
				fseek(f, block_addr(indirbuf[x]), SEEK_SET);
				fread(	dirbuf + ( (12 + x) * c_block_size), 
						sizeof(char), c_block_size, f);
			}
		}

	}
    return (void *)dirbuf;

}


// disk traversal helpers (with inodes

// gets the inode for a directory / file indicated by a given path.
inode *get_inode_for(FILE *f, char *path){
	inode *cur = load_inode(f, 2); 
	
	path = get_next_in_path(path); // skip the leading / for root
	while (path != NULL && *path != '\0') {
		char *name = pop_first_from_path(path);
		inode *nxt = inode_get_child(f, cur, name);
		free(cur);
		cur = nxt;

		path = get_next_in_path(path);
	}

	return cur;
}

// gets the inode for the child of a given directory
inode *inode_get_child(FILE *f, inode *current, char *child_name) {
	directory_node *dnode = aggregate_file(f, current);
	directory_node *dbuffer = dnode;

	if (inode_type(current) != INODE_MODE_DIRECTORY) {
		free(dbuffer);
		return NULL;
	}
	
	while(dnode->d_inode_num != 0) {
		if (memcmp(dnode->name, child_name, 
					sizeof(char) *strlen(child_name))) {
			inode *child = load_inode(f, dnode->d_inode_num);
			free(dbuffer);
			return child;
		}
		dnode = next_node(dnode);
	}
	free(dbuffer);
	return NULL;
}


int fsize_blocks(int fsize){
	return  fsize / c_block_size + (fsize % c_block_size != 0);
}

// init inode metadata
void init_inode_meta(inode *i){
	uint ctime = (uint) time(NULL);
	
	i->i_atime = ctime;
	i->i_ctime = ctime;
	i->i_mtime = ctime;
	i->i_dtime = 0;

	i->i_links_count = 0;
	memset(&(i->i_block), 0, sizeof(((inode *) NULL) -> i_block) );
}


// checks if a given directory block is free
bool is_bitmap_free(int i, char *bitmap) {
	unsigned char data = bitmap[i/8];
	unsigned char mask = 128 >> ((i % 8));
	return mask & data;
}

void set_bitmap_used(int i, char *bitmap, bool val) {
	unsigned char mask = 128 >> ((i % 8));
	bitmap[i/8] = (bitmap[i/8] & ~mask) | ((val) ? 0 : mask ) ;
}

// find an inode from blockgroup_list and mark it as used
// returns a pointer to the inode
uint allocate_inode() {
	//TODO make this work for multiple blocks instead of just the first
	char *ibm = blockgroup_list->inode_bitmap;
	int i;
	for (i=0; i<c_block_size; i++){
		if(is_bitmap_free(i,ibm)) {
			set_bitmap_used(i, ibm, true);
			return i;
		}
	}
	return 0;
}

// find a data block in blockgroup_list and mark it as used
// returns the number of thedatablock
uint allocate_data_block() {
	//TODO make this work for multiple blocks instead of just the first
	char *ibm = blockgroup_list->block_bitmap;
	int i;
	for (i=0; i<c_block_size; i++){
		if(is_bitmap_free(i,ibm)) {
			set_bitmap_used(i, ibm, true);
			return i;
		}
	}
	return 0;
}

// inode creation
int make_inode(int fsize) {
	int fblocks = fsize_blocks(fsize);

	// find empty inode in datablock list and allocate it
	uint new_ino = allocate_inode();
	inode *new_node = get_inode(new_ino);
	if(new_node == NULL) {
		fprintf(stderr, "could not allocate new inode, no space left\n");
		exit(1);
	}

	// init the generic inode metadata
	// and the size
	init_inode_meta(new_node);
	new_node->i_size = fsize;
	new_node->i_blocks = (c_block_size/512) * fblocks;

	// find and allocate enough data blocks for the file
	int i;
	for(i=0; i<fblocks; i++) {
		uint block = allocate_data_block();
		if(block == 0) {
			fprintf(stderr, "could not allocate new data block, no space left\n");
			exit(1);
		}
		new_node->i_block[i] = block;
	}
	// set the remaining blocks to 0
	memset( &(new_node->i_block[i+1]), 0, 15-i);

	return new_ino;
}

inode *get_inode(int ino) {
	return &(blockgroup_list->inode_table[ino]);
}

int make_file_inode(int fsize) {
	int ino = make_inode(fsize);
	inode *i = get_inode(ino);
	i->i_mode = INODE_MODE_FILE;
	return ino;
}


directory_node *next_node(directory_node *d){
    return (directory_node *) ( ((char *)d) + d->d_rec_len);
}

void make_hardlink(FILE *f, char *name, inode *dir, uint file_ino) {

	//aggregate the file, get the pointer to the padding 
	directory_node *dir_head = aggregate_file(f, dir);
	directory_node *padding = dir_head;
	while(padding->d_inode_num != 0)
		padding = next_node(padding);


	ushort required_size = 8 + (int)(d_node + strlen(name));
	// if there is not enough room in the file, allocate a new buffer for it
	if (padding->d_rec_len - 8 < required_size) {
		// malloc an appropriately sized buffer
		size_t size_buffer = sizeof(char)
							* c_block_size 
							* (dir->i_blocks*(512/c_block_size));
		directory_node *new_buffer = malloc(size_buffer + c_block_size * sizeof(char));
		
		// copy over to the appropriate buffer
		memcpy(new_buffer, dir_head, size_buffer);	
		free(dir_head);

		// reassign file_head and new_node
		dir_head = new_buffer;
		int padding_diff = (intptr_t) padding - (intptr_t) dir_head;
		padding = dir_head + padding_diff;
	
		//add a new block to the inode
		int new_block = allocate_data_block();
		inode_add_block(new_block);
	}

	
	// move the node at the head of the padding forward
	// and put the new directory node in its place
	directory_node *newpadding = (directory_node *)(((char *)padding) + required_size);
	memcpy(newpadding, padding, required_size);
	newpadding->d_rec_len = newpadding->d_rec_len - required_size;

	// get the right file type for the directory entry
	inode *file_inode = get_inode(file_ino);
	ushort ftype;
	switch (inode_type(file_inode)) {
		case INODE_MODE_FILE:
			ftype = 1;
			break;
		case INODE_MODE_DIRECTORY:
			ftype = 2;
			break;
		default:
			ftype = 0;
	}
	// assign the values for the new directory entry
	padding->d_inode_num = file_ino;
	padding->d_rec_len = required_size;
	padding->name_len = (ushort) strlen(name);
	padding->filt_type = ftype;

	// dump the modified directory file back to the disk
	dump_file_to_disk(f, dir, dir_head);
	free(dir_head);
}





int block_addr(int blockaddr){
	return 1024 + c_block_size * (blockaddr - 1);
}
