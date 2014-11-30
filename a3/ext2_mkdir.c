#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdbool.h>
#include "superblock.h"
#include "blockgroup.h"
#include "ext2.h"

void cp_to_dir(char *name, directory *dir, void *buffer, void *amnt) {
	//TODO this
}

int main(int argc, char** argv) {
	if (argc !=3 ){i
		ifprintf(stderr, "Usage is: ext2_cp <image> <file> <path>");
	}
	
	else {
		// load the file and check that it is correct
		FILE	*img  = fopen(argv[1], "r+"),
				*file = fopen(argv[2], "r");
		if (!img)
			fprintf(stderr, "failed to open disk image %.*s",
						strlen(argv[1]), argv[1]);
		if (!file)
			fprintf(stderr, "failed to open file %.*s",
						strlen(argv[2], argv[2]));
		
		init_ext2lib(img);
	
		// 2 possible configurations:
		// destination path can either be a full path
		// or the name of the directory containing it.
		inode *dir;
		bool path_is_dir = (dir = get_inode_for(f, argv[3]))
		bool path_is_name = !path_is_dir && 
				(dir = get_inode_for(f, pop_last_from_path(argv[3])))
		
		if (path_is_dir) {
			cp_to_dir(	get_last_from_path(argv[2]), dir);
		}
		else if (path_is_name) {
			cp_to_dir(	get_last_from_path(argv[3]), dir);
		} else {
			fprintf(stderr, "invalid copy destination");
		}
}

