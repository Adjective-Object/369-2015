#ifndef EXT2_369
#define EXT2_369

#include <stdint.h>
#include <stdio.h>

//helper
void print_hex(void *bin, size_t size);


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
void swap_endian_on_block(void* c, uint size);
void swap_endian_on_field(void *c, uint size);

// function to init the ext2lib
void init_ext2lib(FILE *f);

#endif
