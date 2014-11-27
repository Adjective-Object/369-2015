#ifndef EXT2_369
#define EXT2_369

#include <stdint.h>
#include <stdio.h>

#define efix(field) \
	(swap_endian_on_field(\
		(void *)&(s->field),\
		sizeof( ((superblock *) NULL)->field) )) ;

// aliases for readability I guess
typedef uint32_t uint;
typedef uint16_t ushort;
typedef unsigned __int128 uvlong;

// global endianness tracker
char is_little_endian;
void fix_endian_on_block(void* c, uint32_t size);
void swap_endian_on_field;

// function to init the ext2lib
void init_ex2lib(FILE *f);

#endif
