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
	//advance the usage time pointer on p	
	map_next(p->vaddr);	

	int max_nextuse = 0;
	int curmax = 0;
	int frame;
	for(frame=0; frame < memsize; frame++){	
		int frameaddr = coremap[frame].vaddr;
		int nuse = map_peek(frameaddr);
		if (nuse > max_nextuse) {
			max_nextuse = nuse;
			curmax = frame;
		}
	}

	#if DEBUG
	printf("evicting slot %d (%x)  for %x\n", 
		curmax, 
		coremap[curmax].vaddr,
		p->vaddr);	
	#endif

	return curmax;
}

void opt_advanceptr(struct page *p){
	//step forward the "accessed" pointer on the hashmap
	#if DEBUG
	printf("advancing pointer for %x\n", p->vaddr);
	#endif
	map_next(p->vaddr);
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

	// initialize the global map
	map_init();

	// read file line by line and build the hashmap 
	while(fgets(buf, MAXLINE, file)) {
		if(buf[0] != '=') {
			sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
			// only pass the page in (drop the last 12 digs)
			vaddr = vaddr & (~0xfff);
			map_insert(vaddr, line_no);
			line_no++;
		} else {
			continue;
		}
	}

	// reset the "trace" on the map (so the linked lists
	// in each slot can be consumed as if they were streams)
	map_rsttrace();
	if(debug) {	
		map_print();
	}
}


