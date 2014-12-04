#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

extern superblock *superblock_root;
extern blockgroup *blockgroup_root;

extern bool c_one_bg;
extern size_t c_block_size;
extern uint c_num_block_groups;

void inspect_inode(int ino);

int indentation = 0;
#define pind() {\
        int __ind_iter=0;\
        for(__ind_iter=0; __ind_iter<indentation; __ind_iter++){\
                printf("\t");\
        }}

void inspect_directory(inode *i) {
    directory_node *d = aggregate_file(i);
    directory_node *first_d = d;

    int traversed = 0;
    while (d->d_inode_num != 0) {
        pind();
        printf("%d:\t", d->d_inode_num);
        printf("%.*s (%d)", (int) strlen(d->name), d->name, traversed);
        printf("\n");
        
        pind();
        pfield(d, d_rec_len);
        pind();
        pfields(d, name_len);

        traversed += d->d_rec_len;
        d = next_node(d);

    }

    
    d = first_d;
    while (d->d_inode_num != 0) {
        
        if (d->name[0] != '.') {

            inode *sub_inode = get_inode(d->d_inode_num);
            ushort ftype = ((sub_inode->i_mode) & 0xF000);
            if (ftype == INODE_MODE_DIRECTORY) {
                pind();
                printf("going into subdirectory %.*s {\n", d->name_len, d->name);
                indentation ++;
                inspect_inode(d->d_inode_num);
                indentation --;
                pind();
                printf("}\n");
            } else if (ftype == INODE_MODE_FILE) {
                pind();
                printf("inspecting file %.*s {\n", d->name_len, d->name);
                indentation ++;
                inspect_inode(d->d_inode_num);
                indentation --;
                pind();
                printf("}\n");
            }

        }

        d = next_node(d);
    }
}

void inspect_inode(int ino) {
    //load inode into memory
    pind();
    printf("number=%d, inode_size=%d\n",
            ino, superblock_root -> s_inode_size);

    inode *cur_inode = get_inode(ino);

    //print_hex(block_addr(blockgroup_list->desc->bg_inode_table), c_block_size);
    //print_hex(cur_inode, sizeof(inode));

    /*
    printf("%p\n", blockgroup_list->desc);
    printf("block=%p. inode 1=%p, 2=%p, 3=%p\n", 
                    blockgroup_list->inode_table, 
                    get_inode(1), get_inode(2), get_inode(3));

    printf("table=%p, ino=%p, diff=%ld\n", 
                    blockgroup_list->inode_table, 
                    cur_inode,
                    cur_inode - blockgroup_list->inode_table);
     */

    // print what type it is
    pind();
    printf("inode no: %d", ino);
    pfields(cur_inode, i_mode);
    pfield(cur_inode, i_flags);
    printf("\tblocks: ");

    int x;
    for (x = 0; x < 12; x++) {
        printf("%d ", cur_inode->i_block[x]);
    }
    printf("[%d, %d, %d]\n",
            cur_inode->i_block[12],
            cur_inode->i_block[13],
            cur_inode->i_block[14]);


    ushort ftype = ((cur_inode->i_mode) & 0xF000);
    switch (ftype) {
        case INODE_MODE_DIRECTORY:
            inspect_directory(cur_inode);
            break;
        case INODE_MODE_FILE:
            pind();
            printf("\tinode %d is a file\n", ino);
            char *buf = aggregate_file(cur_inode);
            indentation++;
            pind();
            printf("%.*s\n", cur_inode->i_blocks * 512, buf);
            indentation--;
            break;
        default:
            pind();
            printf("no method to handle file mode type %x, (inode %d)\n",
                    ftype, ino);
    }
}

void inspect_bitmap(char *bitmap, uint count) {
    int i;

    printf("in use: ");
    for (i=1; i<=count; i++) {
        if (!is_bitmap_free(i, bitmap)) {
            printf("%d, ", i);
        }
    }
    printf("\n");
}

void print_num(int ino) {
    printf("%d, ", ino);
}

void inspect_blockgroup(blockgroup *bg) {
    descriptor *d = bg->desc;

    pfield(d, bg_block_bitmap);
    pfield(d, bg_inode_bitmap);
    pfield(d, bg_inode_table);
    printf("\n");

    pfields(d, bg_free_blocks_cont);
    pfields(d, bg_free_inodes_cont);
    pfields(d, bg_used_dirs_cont);

    // check out the inode bitmap
    
    printf("\n\n");
    print_hex(bg->inode_bitmap, superblock_root->s_inodes_count/8);
    printf("inodes ");
    inspect_bitmap(bg->inode_bitmap, superblock_root->s_inodes_count);

    printf("\n\n");
    // and the data bitmap
    print_hex(bg->inode_bitmap, superblock_root->s_blocks_count/8);
    printf("data blocks ");
    inspect_bitmap(bg->block_bitmap, superblock_root->s_blocks_count);

    printf("\n");
    // c
    // inspect the root inode
    inspect_inode(2);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage is ext2_inspect <disk_img>\n");
        return 1;
    } else {
        FILE *f = fopen(argv[1], "r+");
        init_ext2lib(f);
        inspect_blockgroup(blockgroup_root);
        teardown_ext2lib(f);
    }
    return 0;
}

