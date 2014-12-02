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
    if (argc != 3) {
        fprintf(stderr,
                "Usage is ext2_traverse <image> <path>\n");
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

    init_ext2lib(img);

    char *next_in_path = get_next_in_path(argv[2]);
    char *last_in_path = get_last_in_path(argv[2]);

    char *first_from_path = pop_first_from_path(argv[2]);
    char *base_path = pop_base_from_path(argv[2]);

    char *path = argv[2];

    pstr(path);

    pstr(next_in_path);
    pstr(last_in_path);
    pstr(first_from_path);
    pstr(base_path);
    printf("\n");

    int dest = get_inode_by_path(argv[2]);
    if (dest) {
        printf("found inode %d\n", dest);
    } else {
        printf("error getting inode.\n");
    }
    teardown_ext2lib();

    return 0;
}
