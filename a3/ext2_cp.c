#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern size_t c_block_size;

void cp_to_dir(FILE *input, char *name, int dest_dir) {
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
                "Usage is ext2_cp <image> <file> <path_to_destination>\n");
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

    FILE *fil = fopen(argv[2], "r");
    if (!fil) {
        fprintf(stderr, "Error opening file \"%.*s\" for reading",
                (int) strlen(argv[2]),
                argv[2]);
        return 1;
    }

    printf("img: %.*s \n", (int) strlen(argv[1]), argv[1]);
    printf("file: %.*s \n", (int) strlen(argv[2]), argv[2]);

    
    if(*(argv[3]) != '/') {
        printf("path must be absolute\n");
        exit(1);
    }
    
    init_ext2lib(img);

    // find the destination directory and name
    // start by assuming path is the path to the parent directory
    // and the file's name it it's name
    char *dir_path = argv[3];
    char *name = get_last_in_path(argv[2]);
    printf("attempting file \n\tpath: \"%.*s\" \n\tname: \"%.*s\"\n",
            (int) strlen(dir_path), dir_path,
            (int) strlen(name), name);
   
    int dest = get_inode_by_path(dir_path);
    
    // if the file was not found or is not a directory, change assumption to
    // path is the path to the new file on dir
    if (dest == 0) { 
        name = get_last_in_path(dir_path);
        dir_path = pop_base_from_path(dir_path);
        printf("directory \"%.*s\" not found",
                (int)strlen(dir_path), dir_path); 
        printf("attempting file \n\tpath: \"%.*s\" \n\tname: \"%.*s\"\n",
            (int) strlen(dir_path), dir_path,
            (int) strlen(name), name);
    }

        
    dest = get_inode_by_path(dir_path);
    
    // exit if the file exists
    char *fullpath = (char *) malloc(strlen(dir_path) + strlen(name) + 1);
    strcpy(fullpath, dir_path);
    strcat(fullpath, "/");
    strcat(fullpath, name);
    printf("\"%.*s\"\n", (int)strlen(fullpath), fullpath);
    if (0 != get_inode_by_path(fullpath) ){
        fprintf(stderr, 
                "file \"%.*s\" already exists", 
                    (int)strlen(fullpath), fullpath);
        return 1;
    }
    
    // exit if the destination directory is invalid
    if(inode_type(get_inode(dest)) != INODE_MODE_DIRECTORY) {
        fprintf(stderr,
                "destination \"%.*s\" is not a directory\n", 
                    (int) strlen(dir_path), dir_path);
        teardown_ext2lib();
        return 1;
    }



    // insert with proper name into the directory
    cp_to_dir(fil, name, dest);

    teardown_ext2lib();

    return 0;
}
