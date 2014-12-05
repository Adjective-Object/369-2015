#include "ext2.h"
#include "superblock.h"
#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <ctype.h>

void swap_endian_on_field(void *addr, uint32_t size) {
    int i;
    char tmp;
    char *iter = addr;

    for (i = 0; i < size / 2; i++) {
        tmp = iter[i];
        iter[i] = iter[size - i - 1];
        iter[size - 1 - i] = tmp;
    }
}

// swaps the endianness of a field

void swap_endian_on_block(void *addr, uint32_t size) {
    int i;
    char tmp;
    char *iter = addr;

    for (i = 0; i < size / 2; i++) {
        tmp = iter[1];
        iter[1] = iter[0];
        iter[0] = tmp;

        iter = iter + 2;
    }
}


// intializtion of constants
extern char is_little_endian;
extern blockgroup *blockgroup_root;

extern uint c_num_block_groups;
extern size_t c_block_size;
extern uint c_bg_size;
extern bool c_one_bg;

void print_chars(void *bin, size_t size) {
    char *c = bin;
    int i;
    for (i = 0; i < size; i++) {
        if (isgraph(*c)) {
            printf("%c", *c);
        } else {
            printf(" ");
        }
        c++;
    }
}

void print_hex(void *bin, size_t size) {
    short *c = bin;
    int i;

    for (i = 0; i < size / 2; i++) {
        if (i % 8 == 0) {
            if (i != 0) {
                printf("\t");
                print_chars(c - 8, sizeof (char) * 16);
            }
            printf("\n");
        }
        printf("%04hx ", (unsigned short) (*c));
        c++;
    }
    printf("\n");
}

extern char *ext2_map_start;

void init_ext2lib(FILE *f) {
    //init endianness
    unsigned int x = 1;
    // cast to a char ptr and get the first char.
    // if is little endian, will be
    // [0001] 0000 0000 0000
    // not 0000 0000 0000 0001
    is_little_endian = (((short *) &x)[0]);

    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);

    //map memory;
    ext2_map_start = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
    if (ext2_map_start == MAP_FAILED) {
        perror("Error mmapping the image file\n");
        exit(1);
    } else {
        printf("mmap created, address %p\n", ext2_map_start);
    }

    //load the root block;
    superblock_root = parse_super(1024);

    //print_hex(superblock_root, sizeof(superblock));

    c_block_size = 1024 << (superblock_root->s_log_block_size);
    c_bg_size = c_block_size * superblock_root->s_blocks_per_group;

    //load each blockgroup into the list of blockgroups)

    //print_hex(superblock_root, sizeof(superblock));

    c_num_block_groups = superblock_root->s_blocks_count /
            superblock_root->s_blocks_per_group;
    c_one_bg = (c_num_block_groups == 0);

    //pfield(superblock_root,s_blocks_count);
    //pfield(superblock_root,s_blocks_per_group);

    //load the blockgroups in to memory

    blockgroup_root = malloc(sizeof (blockgroup));

    load_blockgroup(blockgroup_root, 1024);
    //swap_endian_on_block(blockgroup_root->inode_bitmap, c_block_size);
    //swap_endian_on_block(blockgroup_root->block_bitmap, c_block_size);

}

void teardown_ext2lib() {

    //swap_endian_on_block(blockgroup_root->inode_bitmap, c_block_size);
    //swap_endian_on_block(blockgroup_root->block_bitmap, c_block_size);

}



size_t d_node = 8 * sizeof (char);

uint file_peek(FILE *f) {
    int p;
    fread(&p, sizeof (int), 1, f);

    fseek(f, -1, SEEK_CUR);
    return p;
}










// path helpers
// we don't care about windows style directory entries

char *get_last_in_path(char *path) {
    char *lastslash = path;
    while (*path != '\0') {
        if (*path == '/')
            lastslash = path;
        path++;
    }
    return lastslash + 1;
}

char *pop_base_from_path(char *path) {
    char *last = get_last_in_path(path);
    intptr_t len = (intptr_t) (last) - (intptr_t) (path);
    
    char *newstr = malloc(sizeof (char) * len + 1);
    memcpy(newstr, path, len);
    newstr[len] = '\0';
    return newstr;
}

char *get_next_in_path(char *path) {
    while (*path != '\0') {
        if (*path == '/') {
            return path + 1;
        }
        path++;
    }
    return path;
}

char *pop_first_from_path(char *path) {
    char *next = get_next_in_path(path);

    intptr_t len = (intptr_t) next - (intptr_t) path;
    
    char *newstr = malloc(sizeof (char) * (len + 1));
    memcpy(newstr, path, sizeof (char) * (len));
    memcpy(newstr + len, "\0", sizeof (char));

    return newstr;
}



