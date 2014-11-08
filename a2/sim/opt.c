#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#ifndef MAXLINE
#define MAXLINE 256
#endif

extern char* tracefile;

extern int memsize;
extern int debug;
extern struct frame *coremap;

/* Page to evict is chosen using the accurate optimal algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int opt_evict(struct page *p) {
	
	return 0;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	// read the in line by line, and generate a list of access times
	// for each pointer accessed in the file
	int line_no = 0;

	char buf[MAXLINE];
	FILE *file = fopen(tracefile, "r");
	
	char type;
	int length;
	addr_t vaddr;


	printf("initializing map\n");
	map_init();
	printf("done init\n");

	// read file line by line and build the hashmap 
	while(fgets(buf, MAXLINE, file)) {
		if(buf[0] != '=') {
			sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
			// only pass the page in (drop the last 12 digs)
			vaddr = vaddr & (~0xfff);
			printf("inserting %x\n as %d", vaddr, line_no);
			map_insert(vaddr, line_no);
			line_no++;
		} else {
			continue;
		}
	}
	map_print();

}


