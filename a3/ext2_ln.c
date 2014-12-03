#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;

void ln_to_dir(FILE *input, char *name, int dest_dir) {
    printf("cp %.*s to inode %d\n", (int) strlen(name), name, dest_dir);

    //get the size of the file
    fseek(input, 0, SEEK_END);
    int i, fsize = ftell(input);

    int fsize_blocks = fsize / c_block_size + (fsize % c_block_size != 0);
    char *buffer = malloc(sizeof (char) *c_block_size);

    // find a new inode and allocate to it the blocks for a new file
    int new_inode = make_file_inode(fsize);
    if(new_inode == 0){
        fprintf(stderr, "could not allocate new inode, no space left\n");
    } else{
        printf("new file in inode %d\n", new_inode);
    }

    // read the file in one block at a time and copy it into the
    // appropriate data blocks
    fseek(input, 0, SEEK_SET);
    for (i = 0; i < fsize_blocks; i++) {
        fread(buffer, sizeof (char), c_block_size, input);

        void *block = inode_nth_block_ptr(get_inode(new_inode), i);
        memcpy(block, buffer, c_block_size);
    }

    printf("creatidng hardlink %d->%d\n", dest_dir, new_inode);
    make_hardlink(name, get_inode(dest_dir), new_inode);
}

int main(int argc, char ** argv) {
    if (argc != 4) {
        fprintf(stderr,
                "Usage is ext2_cp <image> <target> <directory>\n");
        return 1;
    }

    //load files, check for opening errors
    FILE *img = fopen(argv[1], "r+");
    if (!img) {
        fprintf(stderr, "Error opening image file \"%.*s\"",
                (int) strlen(argv[1]),
                argv[1]);
        return 1;
    }

    printf("img: %.*s \n", (int) strlen(argv[1]), argv[1]);
    printf("target: %.*s \n", (int) strlen(argv[2]), argv[2]);
    printf("directory: %.*s \n", (int) strlen(argv[3]), argv[3]);

    
    if(*(argv[2]) != '/' || *(argv[3]) != '/') {
        printf("paths must be absolute\n");
        exit(1);
    }
    
    init_ext2lib(img);

    // find the destination directory and name
    int dest = get_inode_by_path(argv[3]);
    char *name = get_last_in_path(argv[2]);

    // move destination
    if (dest == 0 || inode_type(get_inode(dest)) != INODE_MODE_DIRECTORY) {
        dest = get_inode_by_path((argv[3]));
        name = get_last_in_path(argv[3]);
    } else if (inode_type(get_inode(dest)) != INODE_MODE_DIRECTORY){
        fprintf(stderr, "file with path \"%.*s\" already exists.\n",
                (int) strlen(argv[3]),
                argv[3]);
        fprintf(stderr, "remove it before cping something to that path\n");
        teardown_ext2lib();pop_base_from_path
        exit(1);

    }

    // insert with proper name into the directory
    cp_to_dir(fil, name, dest);

    teardown_ext2lib();

    return 0;
}
