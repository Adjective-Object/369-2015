#include "superblock.h"
#include "ext2.h"
#include <stdio.h>
#include <string.h>

extern char is_little_endian;

void fix_endian(superblock *s) {
    efix(s_inodes_count);
    efix(s_blocks_count);
    efix(s_r_blocks_count);
    efix(s_free_blocks_count);
    efix(s_free_inodes_count);
    efix(s_first_data_block);

    efix(s_log_block_size);
    efix(s_log_frag_size);

    efix(s_blocks_per_group);
    efix(s_frags_per_group);

    efix(s_mtime);
    efix(s_wtime);

    efix(s_mnt_count);
    efix(s_max_mnt_count);

    efix(s_magic);
    efix(s_state);

    efix(s_errors);

    efix(s_minor_rev_level);

    efix(s_lastcheck);
    efix(s_checkinterval);

    efix(s_creator_os);
    efix(s_rev_level);

    efix(s_def_resuif);
    efix(s_def_resgid);

    efix(s_first_ino);
    efix(s_inode_size);
    efix(s_block_group_nr);

    efix(s_feature_compat);
    efix(s_feature_incompat);
    efix(s_feature_ro_compat);

	efix(s_uuid);
	efix(s_volume_name);

    efix(s_algo_bitmap);

	efix(s_journal_uuid);
    efix(s_journal_inum);
    efix(s_journal_dev);
    efix(s_last_orphan);
}

void check_and_fix_endian(superblock *s) {
	if (is_little_endian) {
		printf("this system is little endian, do not need to flip byte order\n");
	} else{
		printf("this system is big endian, performing swap on superblock\n");
		fix_endian(s);
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


superblock *parse_super(FILE *f) {
	// read contents of junk
	superblock *super = malloc(sizeof(superblock)); // malloc and load
	fread((void *) super, sizeof(superblock), 1, f);

	// correct the endianness of the items in fields
	check_and_fix_endian(super);

	// check verification tricks

	//check if itis usable
	check_support(super);

	// set fields not set in revision 0 superblock
	if (super->s_rev_level == 0) {
		super->s_first_ino = 11;
		super->s_inode_size = 128;
	}

	return super;
}

