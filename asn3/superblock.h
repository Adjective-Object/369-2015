#ifndef EXT2_369
#define EXT2_369

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t uint;
typedef uint16_t ushort;
typedef unsigned __int128 uvlong;

/*******************************/
/* SUPERBLOCK FIELDS, IN ORDER */
/*******************************/

//general

uint   s_inodes_count;       // inodes in system (used and free)
uint   s_blocks_count;       // blocks in system (used and free)
uint   s_r_blocks_count;     // blocks reserved for superuser
uint   s_free_blocks_count;  // free blocks, including reserved
uint   s_free_inodes_count;  // free inodes
uint   s_first_data_block;   // the word of the id containing the superblock
                              // (note that this is bytes * 4)

uint   s_log_block_size;     // log of the block size (1024 << size)
uint   s_log_frag_size;      // fragmenet size (?? what is this TODO)

uint   s_blocks_per_group;   // blocks per group (for boundaries)
uint   s_frags_per_group;    // fragmenets per group (for block bitmap size)

uint   s_mtime;              // last mount time (Unix Timestamp)
uint   s_wtime;              // last write time (Unix Timestamp)

ushort s_mnt_count;          // number of mounts since last verification
ushort s_max_mnt_count;      // number of mounts between verification

ushort s_magic;              // EX2 verifiction (if not 0xEF53, PANIC)
ushort s_state;				 // 1: mounted
                             // 2: unmounted

ushort s_errors;             // Error level and associated response
                             // 1: continue mount
                             // 2: remount as read only
                             // 3: cause a kernel panic

ushort s_minor_rev_level;    // revision level within the major revision
                             // TODO I assume it is safe to ignore this

uint   s_lastcheck;          // last file system check (Unix Timestamp)
uint   s_checkinterval;      // maximum time between checks allowed  (Unix Time Interval)

uint   s_creator_os;         // Operating system that created the partition
                             // 0: linux
                             // 1: gnu hurd (?)
                             // 2: masix
                             // 3: freebsd
                             // 4: lites

uint   s_rev_level;          // 0: old ext2 (static sized inodes)
                             // 1: new (variable inode sizes, extended attrs)

ushort s_def_resuif;         // default user id for reserved blocks
ushort s_def_resgid;         // default group id for reserved blocks

// EXT2 DYANMIC REV Specific
uint   s_first_ino;          // first inode for standard files  (11 in old)
ushort s_inode_size;           // size of inode					(128 in old)
ushort s_block_group_nr;     //	the block group hosting this superblock

// http://www.nongnu.org/ext2-doc/ext2.html#S-FEATURE-COMPAT
uint   s_feature_compat; 
uint   s_feature_incompat;
uint   s_feature_ro_compat;

uvlong s_uuid;               // uuid of the filesystem
uvlong s_volume_name;        // name of the filesystem, ISO-Latin-1, \0 term'd

//uvlong s_last_mounted
    // we don't care about this field at all, so we'll skip it
    // just remember that it is 64 byes wide, so we need to fseek past it

uint   s_algo_bitmap;        // compression algorithms used

// Performance hits (prealoc blocks)
    // noone cares, just skip 4 bytes (one int)

// Journaling Support
uvlong s_journal_uuid;       // uuid of the journal superblock (?? TODO)
uint   s_journal_inum;       // inode number of the journal file
uint   s_journal_dev;        // device number of the journal device
uint   s_last_orphan;        // first inode in a list of inodes to be deleted

// Directory Indexing with Hashes
	// don't need to support this, skip 20 bytes

// these might just be ext3, so who cares for now
// uint   s_default_mount_options;
// uint   s_first_meta_bg;

void parse_super(FILE *f);

#endif

