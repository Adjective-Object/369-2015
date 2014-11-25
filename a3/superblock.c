#include "superblock.h"


#define read_sp(var) if ( fread(&var, sizeof(var), 1, f) == 0 ) {\
	perror("fail on read variable " #var " from super block");\
	exit(1);\
}

#define skip(amt) fseek(f, amt, SEEK_CUR);

void parse_super(FILE *f) {
	//assumes the current fp is pointing to the head of a super block

	// read the first block of values in, order matters
	// (see superblock.h for meanings)
	// it's times like these I wish C had recursive vararg macros
	read_sp(s_inodes_count);
	read_sp(s_blocks_count);
	read_sp(s_r_blocks_count);
	read_sp(s_free_blocks_count);
	read_sp(s_free_inodes_count);
	read_sp(s_first_data_block);
	
	read_sp(s_log_block_size);
	read_sp(s_log_frag_size);
	
	read_sp(s_blocks_per_group);
	read_sp(s_frags_per_group);
	
	read_sp(s_mtime);
	read_sp(s_wtime);

	read_sp(s_mnt_count);
	read_sp(s_max_mnt_count);

	read_sp(s_magic);
	read_sp(s_state);

	read_sp(s_errors);
	
	read_sp(s_minor_rev_level);
	
	read_sp(s_lastcheck);
	read_sp(s_checkinterval);
	read_sp(s_rev_level);

	read_sp(s_def_resuif);
	read_sp(s_def_resgid);
	
	// EX2 Dynamic Rev Specific
	read_sp(s_first_ino);
	read_sp(s_inode_size);
	read_sp(s_block_group_nr);
	
	read_sp(s_feature_compat);
	read_sp(s_feature_incompat);
	read_sp(s_feature_ro_compat);
	read_sp(s_uuid);
	read_sp(s_volume_name);
	
	skip(64); // skip 64 bit timestamp of last mount

	read_sp(s_algo_bitmap);

	skip(4); // performance hints I dont care about

	read_sp(s_journal_uuid);
	read_sp(s_journal_inum);
	read_sp(s_journal_dev);
	read_sp(s_last_orphan);
}

