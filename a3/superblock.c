#include "superblock.h"
#include "ext2.h"
#include <stdio.h>
#include <string.h>

extern char is_little_endian;

void check_and_fix_endian(superblock *s) {
	if (is_little_endian) {
		printf("this system is little endian, do not need to flip byte order\n");
	} else{
		printf("this system is big endian, performing swap on superblock\n");
		swap_endian_on_block(s, sizeof(superblock));
		//fix_endian(s);
	}
}

void verify(superblock *s) {
	if (s->s_magic != 0xEF53) {
		// TODO graceful fail for reading multiple super blocks
		// with conflicts
		fprintf(stderr, "superblock failed magic test, panicing and exit\n");
		exit(1);
	}
}


void check_support(superblock *s) {
	char *feature = NULL;

	if (1) {
		// TODO check the values of s_feature_compat, s_feature_incompat
		// and s_feature_ro_compat to panic and crash
	}

	if (feature != NULL) {
		fprintf(stderr, "unsupported feature %.*s",
				(int) strlen(feature),
				feature);
		exit(1);
	}
}


superblock *parse_super(int offset) {
	superblock *super = (superblock *) (ext2_map_start + offset);
	
	// correct the endianness of the items in fields
	// check_and_fix_endian(super);

	//check if it is usable
	check_support(super);

	// set fields not set in revision 0 superblock
	/*
	if (super->s_rev_level == 0) {
		super->s_first_ino = 11;
		super->s_inode_size = 128;
	} */

	return super;
}

