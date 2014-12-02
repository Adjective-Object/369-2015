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

void inspect_directory(inode *i) {
    directory_node *d = aggregate_file(i);
    directory_node *first_d = d;

    while (d->d_inode_num != 0) {
        printf("%d:\t", d->d_inode_num);
        printf("%.*s", (int) strlen(d->name), d->name);
        d = next_node(d);
        printf("\n");
    }

    d = first_d;
    while (d->d_inode_num != 0) {
        if (d->name[0] != '.') {

            inode *sub_inode = get_inode(d->d_inode_num);
            ushort ftype = ((sub_inode->i_mode) & 0xF000);
            if (ftype == INODE_MODE_DIRECTORY) {
                printf("going into subdirectory %.*s\n", d->name_len, d->name);
                inspect_inode(d->d_inode_num);
            } else if (ftype == INODE_MODE_FILE) {
                printf("inspecting file %.*s\n", d->name_len, d->name);
                inspect_inode(d->d_inode_num);
            }

        }

        d = next_node(d);
    }
}

void inspect_inode(int ino) {
    //load inode into memory
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
    pfields(cur_inode, i_uid);
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
            printf("\tinode %d is a file\n",
                    cur_inode->i_uid);
            char *buf = aggregate_file(cur_inode);
            printf("%.*s\n", cur_inode->i_blocks * 512, buf);

            break;
        default:
            printf("no method to handle file mode type %x, (inode %d = %d)\n",
                    ftype, cur_inode->i_uid, ino);
    }

    printf("done inspecting inode %d\n", ino);
}

void inspect_bitmap(char *bitmap) {
    int i;

    printf("in use: ");
    for (i = 0; i < c_block_size * 8; i++) {
        if (!is_bitmap_free(i, bitmap)) {
            printf("%d, ", i);
            /*
            printf("\n i=%d, n=%d\t, value=%02hx (" 
                            BYTE2BIN_PAT ", " BYTE2BIN_PAT ", %d)",
                            i,
                            (i*8 + x), 
                            (bitmap[i]),
                            BYTE2BIN(bitmap[i]),
                            BYTE2BIN(mask),
                            mask);*/
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

    printf("\n");
    // check out the inode bitmap
    printf("inodes ");
    inspect_bitmap(bg->inode_bitmap);
    // and the data bitmap
    printf("data blocks ");
    inspect_bitmap(bg->block_bitmap);

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

