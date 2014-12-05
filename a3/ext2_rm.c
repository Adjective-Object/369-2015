#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"
#include <strings.h>

extern size_t c_block_size;

// delete a link to an inode.
// if there are no links left,
// release it from the inode table
void rm_inode(uint inode_no) {
    inode *ino = get_inode(inode_no);
    ino->i_links_count --;
    if (ino->i_links_count == 0) {
        release_inode(inode_no);
    }
}

void remove_entry(int parent_no, char *name) {
    inode *parent = get_inode(parent_no);
    directory_node *head  = 
        (directory_node *)aggregate_file(parent);
    directory_node *node = head, *prev = NULL;
   
    int total_count = fsize_blocks(parent->i_blocks);
    int traversed_size = 0;
    while(traversed_size < total_count * (int) c_block_size){
        
        printf("%d ", traversed_size);
        printf("%.*s %.*s\n", 
                (int) strlen(name), name,
                (int) node->name_len, node->name);

        if ((int) strlen(name) == node->name_len && 
                !memcmp(node->name, name, strlen(name))) {
           
            printf("removing entry..\n");
            
            if (prev == NULL) {
                prev = node;
                node = (directory_node *) 
                    (((char *) node) + d_node + node->name_len);
            }


            // point this node over the d inode
            prev->d_rec_len = prev->d_rec_len+node->d_rec_len;
            rm_inode(node->d_inode_num);
            node->d_inode_num = 0;

            dump_buffer(parent, head);
            free(head);
            return;
        }

        // advance everything
        traversed_size += node->d_rec_len;
        prev = node;
        node = next_node(node);
    }

    //fail to find file
    fprintf(stderr, "failed to find file \"%.*s\" in the directory\n",
            (int) strlen(name), name);
    teardown_ext2lib();
    exit(1);
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        fprintf(stderr,
                "Usage is ext2_rm <image> <target_path>\n");
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

    
    if(*(argv[2]) != '/') {
        printf("paths must be absolute\n");
        exit(1);
    }
    
    init_ext2lib(img);

    // find the destination directory and name
    int target_no = get_inode_by_path(argv[2]);
    int target_parent =
        get_inode_by_path(
                pop_base_from_path(
                    argv[2] ));
    char *target_name = get_last_in_path(argv[2]);
    
    // ensure the file exists first
    if (target_no == 0) {
        fprintf(stderr, "file \"%.*s\" not found\n",
                    (int) strlen(argv[2]),
                    argv[2]);
        teardown_ext2lib();
        return 1;
    }

    // check that the target is a file
    if (inode_type(get_inode(target_no)) != INODE_MODE_FILE){
        fprintf(stderr, "target file is not a normal file\n");
        teardown_ext2lib();
        return 1;
    }

    remove_entry(target_parent, target_name);
    teardown_ext2lib();

    return 0;
}
