#include <stdio.h>
#include <stdlib.h>
#include "superblock.h"
#include "ext2.h"

#define pfield(strut, field_name) {\
	printf("\t" #field_name ": %u\n",\
			strut->field_name );\
}

extern size_t c_block_size;

int main (int argc, char** argv) {
	if (argc != 2) {
		printf("Usage is ext_meta <disk_img>\n");
		return 1;
	} else {
		
		FILE *f = fopen(argv[1], "r+");
		init_ext2lib(f);
		superblock *sp = superblock_root;

		printf("parsed superblock, revision level %d:\n", sp->s_rev_level);
		pfield(sp, s_inodes_count);
		pfield(sp, s_blocks_count);
		pfield(sp, s_free_blocks_count);
		pfield(sp, s_free_inodes_count);
		pfield(sp, s_first_data_block);
		printf("\n");
		pfield(sp, s_log_block_size);
		printf("\tactual block size: %d\n", (uint)c_block_size);
		printf("\n");
		pfield(sp, s_first_ino);
		pfield(sp, s_inode_size);

		teardown_ext2lib();
	}
	return 0;
}

