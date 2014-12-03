#include "blockgroup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "superblock.h"
#include "ext2.h"
#include <time.h>

void *block_addr(int blockaddr) {
    return ext2_map_start + (1024 + c_block_size * (blockaddr - 1));
}

int inode_numblocks(inode *ino) {
    return ino->i_blocks / (c_block_size / 512);
}

int inode_type(inode *i) {
    return ((i->i_mode) & 0xF000);
}

inode *get_inode(int ino) {
    return &(blockgroup_root->inode_table[ino - 1]);
}

void load_blockgroup(blockgroup * bg, int location) {
    // read a new superblock and compare it to the root.
    // if they are not the same, panic and exit

    // TODO change handling  depending on if we are using revision 0 or 1
    // version of the ext2 filesystem (revision 1 has less redundancy)


    //avoid comparing root to self
    if (location != 1024) {
        superblock *new_super_block;
        new_super_block = parse_super(location);

        if (memcmp(superblock_root, new_super_block, sizeof (superblock))) {
            fprintf(stderr, "superblocks not the same, exiting!\n");
            exit(1);
        }
    }

    //grab the block descriptor from the mapping
    bg->desc = (descriptor *) (ext2_map_start + location + c_block_size);

    //point to the bitmaps and table within the mapping
    bg->block_bitmap = block_addr(bg->desc->bg_block_bitmap);
    bg->inode_bitmap = block_addr(bg->desc->bg_inode_bitmap);
    bg->inode_table = block_addr(bg->desc->bg_inode_table);

    // swap the endianess of the bitmaps
    // swap_endian_on_block(bg->block_bitmap, sizeof(char) * c_block_size);
    // swap_endian_on_block(bg->inode_bitmap, sizeof(char) * c_block_size);
}

// gets the pointer to the nth used file block,
// accounting for indirect nodes

void *inode_nth_block_ptr(inode *i, uint n) {
    if (n < 12 && i->i_block[n]) {
        return block_addr(i->i_block[n]);
    } else if (i->i_block[12]) {
        uint *indirect_buf = block_addr(i->i_block[12]);
        return block_addr(indirect_buf[n - 12]);
    } else {
        return NULL;
    }
}




//aggregates the different disk blocks of an inode
//into a single buffer

void *aggregate_file(inode *i) {
    int n;
    void *block;
    char *buf = malloc(sizeof (char) * inode_numblocks(i) * c_block_size);
    int count = 0;
    for (n = 0; n < (12 + (c_block_size / sizeof (uint))); n++) {
        if ((block = inode_nth_block_ptr(i, n))) {
            fflush(stdout);
            memcpy(buf+(count * c_block_size), block, c_block_size);
            count++;
        } else if (n == 12) {
            break;
        }
    }
    return (void *) buf;
}

void dump_buffer(inode *i, void *buffer) {
    int n;
    void *block;
    int count = 0;
    for (n = 0; n < (12 + c_block_size / sizeof (uint)); n++) {
        if ((block = inode_nth_block_ptr(i, n))) {
            printf("dump block %d\n", n);
            memcpy(block+ (count * c_block_size), buffer, c_block_size);
            count ++;
        } else if (n == 12) {
            break;
        }
    }
}


// gets the inode for a directory / file indicated by a given path.

int get_inode_by_path(char *path) {
    int cur = 2;

    path = get_next_in_path(path); // skip the leading / for root


    while (cur != 0 && path != NULL && *path != '\0') {
        
        char *name = pop_first_from_path(path);
        path = get_next_in_path(path);
        
        char *c = name;
        while (*c!='\0'){
            c++;
            if (*c == '/')
                *c = '\0';
        }
        
        printf("name : %.*s, path : %.*s cur : %d\n",
                (int) strlen(name), name,
                (int) strlen(path), path,
                cur);

        
        cur = inode_get_child(get_inode(cur), name);
    }

    return cur;
}

// gets the inode for the child of a given directory

int inode_get_child(inode *current, char *child_name) {
    directory_node *dnode = aggregate_file(current);
    directory_node *dbuffer = dnode;

    if (inode_type(current) != INODE_MODE_DIRECTORY) {
        free(dbuffer);
        return NULL;
    }

    while (dnode->d_inode_num != 0) {
        
        if (strlen(child_name) == strlen(dnode->name) &&
                 !(memcmp( dnode->name, child_name, strlen(child_name))) ) {
            int child = dnode->d_inode_num;
            free(dbuffer);
            return child;
        }        
        dnode = next_node(dnode);
    }
    free(dbuffer);
    return 0;
}

int fsize_blocks(int fsize) {
    return fsize / c_block_size + (fsize % c_block_size != 0);
}

// init inode metadata

void init_inode_meta(inode *i) {
    uint ctime = (uint) time(NULL);

    i->i_atime = ctime;
    i->i_ctime = ctime;
    i->i_mtime = ctime;
    i->i_dtime = 0;

    i->i_links_count = 0;
}


// checks if a given directory block is free

bool is_bitmap_free(int i, char *bitmap) {
    i--;
    unsigned char data = bitmap[i / 8];
    unsigned char mask = 1 << ((i % 8));
    return !(mask & data);
}

void set_bitmap_used(int i, char *bitmap, bool val) {
    i--;
    unsigned char mask = 1 << ((i % 8));
    bitmap[i / 8] = (bitmap[i / 8] & (~mask)) | ((true == val) ? mask : 0);
}

void release_inode(uint i){
    set_bitmap_used(i, blockgroup_root->inode_bitmap, false);
}

void release_data_block(uint i){
    set_bitmap_used(i, blockgroup_root->block_bitmap, false);
}

// find an inode from blockgroup_list and mark it as used
// returns a pointer to the inode

uint allocate_inode() {
    if (superblock_root->s_free_inodes_count == 0) {
        return 0;
    }

    char *ibm = blockgroup_root->inode_bitmap;
    int i;
    // lost+found is at 11 and is not marked in the bitmap for some reason
    for (i = 12; i < c_block_size * 8; i++) {
        if (is_bitmap_free(i, ibm)) {
            set_bitmap_used(i, ibm, true);
            return i;
        }
    }

    superblock_root->s_free_inodes_count--;
    return 0;
}

// find a data block in blockgroup_list and mark it as used
// returns the number of thedatablock

uint allocate_data_block() {
    if (superblock_root->s_free_blocks_count == 0) {
        return 0;
    }

    char *dbm = blockgroup_root->block_bitmap;
    int i;
    //start scanning at 16 because of reasons//alloc
    //
    for (i = 16; i < c_block_size * 8; i++) {
        if (is_bitmap_free(i, dbm)) {
            set_bitmap_used(i, dbm, true);
            return i;
        }
    }

    superblock_root->s_free_blocks_count--;
    return 0;
}

// inode creation

int make_inode(int fsize) {
    int fblocks = fsize_blocks(fsize);

    // find empty inode in datablock list and allocate it
    uint new_ino = allocate_inode();
    if (new_ino == 0) {
        fprintf(stderr, "could not allocate new inode, no space left\n");
        teardown_ext2lib();
        exit(1);
        
    }
    
    inode *new_node = get_inode(new_ino);

    // init the generic inode metadata
    // and the size
    init_inode_meta(new_node);
    new_node->i_size = fsize;
    new_node->i_blocks = (c_block_size / 512) * fblocks;
    
    // find and allocate enough data blocks for the file
    int i;
    for (i = 0; i < fblocks; i++) {
        //TODO if it requires indirect blocks
        uint block = allocate_data_block();
        printf("allocated data block %d\n", block);
        if (block == 0) {
                fprintf(stderr, "could not allocate new data block, no space left\n");
                release_inode(new_ino);
                teardown_ext2lib();
                exit(1);
        }
        new_node->i_block[i] = block;
    }
    
    printf("memsetting end of file\n");
    // set the remaining blocks to 0
    memset(&(new_node->i_block[i + 1]), 0, 15 - i);

    return new_ino;
}

int make_directory_inode() {
    int ino = make_inode(c_block_size / 512);
    if (ino == 0) return 0;
    inode *i = get_inode(ino);
    i->i_mode = INODE_MODE_DIRECTORY;


    // update the directory count in the superblock;
    return ino;
}

int make_file_inode(int fsize) {
    int ino = make_inode(fsize);
    if (ino == 0) return 0;
    inode *i = get_inode(ino);
    i->i_mode = INODE_MODE_FILE;
    return ino;
}

directory_node *next_node(directory_node *d) {
    return (directory_node *) ((char *) d + (d->d_rec_len));
}

void inode_add_block(inode *i, uint new_block) {
    int x;
    for (x = 0; x < 12; x++) {
        if (i->i_block[x] == 0) {
            i->i_block[x] = new_block;
            return;
        }
    }
    
    //TODO indirect nodes in dirs.
    fprintf(stderr, "(blockgroup.c: inode_add_block) indirect nodes not implemented\n");
    teardown_ext2lib();
    exit(1);
}

void make_hardlink(char *name, inode *dir, uint file_ino) {
    //aggregate the file, get the pointer to the tail 
    directory_node *dir_head = aggregate_file(dir);
    directory_node *tail = dir_head;

    //print_hex(dir_head, c_block_size);
    int bsize = fsize_blocks(dir->i_blocks) * c_block_size;
    int bufcount = bsize;

    printf("scanning to end of directory");
    fflush(stdout);
    // scan until the tail of the linked list
    // TODO traversals that account for deleted entries
    directory_node *next = tail;
    while (bufcount > 0) {
        tail = next;
        printf(".");
        printf("\n %d, %d", tail->d_inode_num, bufcount);
        next = next_node(tail);
        bufcount = bsize - (((intptr_t) next) - ((intptr_t) dir_head));
    }

    // update the the size of the tail pointer and step
    // forward to the blank space
    ushort termlen = (ushort) d_node + (char) tail->name_len + 1;
    tail->d_rec_len = termlen;
    tail = (directory_node *) (((char*) tail) + tail->d_rec_len);

    printf("\ntailno=%d, bufcount=%d", tail->d_inode_num, bufcount);
    printf("\ntail is at %lu (%p - %p)\n",
            (intptr_t) tail - (intptr_t) dir_head,
            tail,
            dir_head);

    ushort required_size = (int) (d_node + strlen(name) + 1);
    // if there is not enough room in the file, allocate a new buffer for it
    if (bufcount - d_node < required_size) {
        printf("not enough size, expanding directory buffer\n");
        // malloc an appropriately sized buffer
        size_t size_buffer = sizeof (char)
                * c_block_size
                * (dir->i_blocks * (512 / c_block_size));
        directory_node *new_buffer = malloc(size_buffer + c_block_size * sizeof (char));

        // copy over to the appropriate buffer
        memcpy(new_buffer, dir_head, size_buffer);
        free(dir_head);

        // reassign file_head and new_node
        dir_head = new_buffer;
        int tail_diff = (intptr_t) tail - (intptr_t) dir_head;
        tail = dir_head + tail_diff;

        //add a new block to the inode
        int new_block = allocate_data_block();
        inode_add_block(dir, new_block);
    }

    // get the right file type for the directory entry
    inode *file_inode = get_inode(file_ino);
    ushort ftype;
    switch (inode_type(file_inode)) {
        case INODE_MODE_FILE:
            printf("adding a new file\n");
            ftype = 1;
            break;
        case INODE_MODE_DIRECTORY:
            printf("adding a new directory\n");
            ftype = 2;
            break;
        default:
            fprintf(stderr, "unknown file type..\n");
            ftype = 0;
    }

    // assign the values for the new directory entry
    printf("assigning directory values\n");
    tail->d_inode_num = file_ino;
    tail->name_len = (ushort) strlen(name) + 1;
    tail->d_rec_len =
            bsize - ((intptr_t) tail - (intptr_t) dir_head);
    tail->filt_type = ftype;
    memcpy(&(tail->name), name, sizeof (char) * strlen(name) + 1);

    // increment the link count on the linked node;
    file_inode->i_links_count++;
    file_inode->i_atime = time(NULL);

    // dump the modified directory file back to the disk
    printf("dumping directory back to disk\n");
    //print_hex(dir_head, c_block_size);
    dump_buffer(dir, dir_head);
}



