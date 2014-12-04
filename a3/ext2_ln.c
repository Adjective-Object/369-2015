#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;

int main(int argc, char ** argv) {
    if (argc != 4) {
        fprintf(stderr,
                "Usage is ext2_cp <image> <target_path> <directory_path> \n");
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
    int dir = get_inode_by_path(argv[3]);
    int target = get_inode_by_path(argv[2]);
    char *target_name = get_last_in_path(argv[2]);

    if(target == 0){
        fprintf(stderr, "target does not exist\n");
        teardown_ext2lib();
        exit(1);
    }
   
    if(dir == 0){
        fprintf(stderr, "directory does not exist\n");
        teardown_ext2lib();
        exit(1);
    }

    if ( inode_type(get_inode(target)) == INODE_MODE_DIRECTORY) {
        fprintf(stderr, "target inode must not be a directory\n");
        teardown_ext2lib();
        return 1;
    }


    make_hardlink(target_name, get_inode(dir), target);
    teardown_ext2lib();

    return 0;
}
