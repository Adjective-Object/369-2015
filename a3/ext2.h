#ifndef EXT2_369
#define EXT2_369

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//macros

#define pfields(strut, field_name) {\
	printf("\t" #field_name ": %hu\n",\
			strut->field_name );\
}

#define pfield(strut, field_name) {\
	printf("\t" #field_name ": %u\n",\
			strut->field_name );\
}

#define efix(field) \
	(swap_endian_on_field(\
		(void *)&(s->field),\
		sizeof( ((superblock *) NULL)->field) )) ;

// from William Whyte on StackOverflow
// http://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
#define BYTE2BIN_PAT "%d%d%d%d%d%d%d%d"
#define BYTE2BIN(byte)  \
	  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 


// aliases for readability I guess
typedef uint32_t uint;
typedef uint16_t ushort;
typedef unsigned __int128 uvlong;


// general purpose helpers
void print_hex(void *bin, size_t size);
uint file_peek(FILE *f);

// path writing
// in path is a pointer to a substring. from path is a new string;
char *get_last_in_path(char *path);
char *pop_last_from_path(char *path);
char *get_next_in_path(char *path);
char *pop_first_from_path(char *path);

// disk interface helpers
FILE f_global;

// global endianness tracker
char is_little_endian;
void swap_endian_on_block(void* c, uint size);
void swap_endian_on_field(void *c, uint size);

// function to init the ext2lib
void init_ext2lib(FILE *f);
// function to write changes to data back onto disk
void update_image(FILE *f);

#endif
