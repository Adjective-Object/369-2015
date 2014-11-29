#ifndef EXT2_BLOCKGROUP_369
#define EXT2_BLOXKGROUP_369

#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"
#include "ext2.h"


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
void load_blockgroup(descriptor *dest, FILE *f, int location);



#define INODE_MODE_DIRECTORY 0x4000
#define INODE_MODE_FILE 0x8000

struct inode {
	ushort i_mode; // see http://www.nongnu.org/ext2-doc/ext2.html#I-MODE
	ushort i_uid;  // user id
	
	uint   i_size; // size of file
	uint   i_atime; // unix timestamp, access
	uint   i_ctime; //                 creation
	uint   i_mtime; //                 modification
	uint   i_dtime; //                 deletion
	
	ushort i_gud; // group id
	ushort i_links_count; // number of hard links to inode
	
	uint   i_blocks; // number of 512 byte blocks used on this file
	uint   i_flags; // http://www.nongnu.org/ext2-doc/ext2.html#I-FLAGS
	uint   i_osd1; // ignore

	char   i_block[60]; // the array of blocks used
	
	uint   i_generation;
	uint   i_file_generation;
	uint   i_file_acl;
	uint   i_dir_acl;
	uint   i_faddr;

	char   i_osd2[12];

} __attribute__((packed));
typedef struct inode inode;

// inode helpers
inode *load_inode(FILE *f, int inode);
int inode_numblocks(inode *ino);
int inode_seek_nth_block(FILE *f, inode *i, int n);
void *aggregate_file(FILE *f, inode *i);




// Directory Fun
struct directory_node {
	uint   d_inode_num;
	ushort d_rec_len;
	char   name_len;
	char   filt_type;
	char   name[255];
} __attribute__((packed));
typedef struct directory_node directory_node;

// Directory Helpers
directory_node *next_node(directory_node *d);


size_t d_node;


// etc
int block_addr(int blocknumber);

#endif
