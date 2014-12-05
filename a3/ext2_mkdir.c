#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;



void mk_dir(int parent_no, char *child_name) { 
    int child_no = make_directory_inode(); 
    inode *new_inode = get_inode(child_no);

    make_hardlink("..", new_inode, parent_no);
    make_hardlink(child_name, get_inode(parent_no), child_no);
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        fprintf(stderr,
                "Usage is ext2_mkdir <image> <path_to_directory>\n");
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
    
    char *path = pop_base_from_path(argv[2]);
    char *dirname = get_last_in_path(argv[2]);

    printf("img: %.*s \n", (int) strlen(argv[1]), argv[1]);
    printf("path: %.*s \n", (int) strlen(path), path);
    printf("dirname: %.*s\n", (int) strlen(dirname), dirname);

    init_ext2lib(img);

    // check the file does not already exist
    if( get_inode_by_path(argv[2]) != 0 ){
        fprintf(stderr, "file \"%.*s\" already exists\n",
                    (int) strlen(argv[2]),
                    argv[2]);
        teardown_ext2lib();
        return 1;
    }
    // find the destination directory and name
    int path_no = get_inode_by_path(path);
    if(path_no == 0){
        fprintf(stderr, "parent directory \"%.*s\" does not exist\n",
                    (int) strlen(path),
                    path);
        teardown_ext2lib();
        return 1;
    }
    
    //check that the parent of the file is a directory
    if (inode_type(get_inode(path_no)) != INODE_MODE_DIRECTORY) {
        fprintf(stderr, "cannot create directory as child of a file\n");
        teardown_ext2lib();
        return 1;
    }

    mk_dir(path_no, dirname);
    teardown_ext2lib();

    return 0;
}
