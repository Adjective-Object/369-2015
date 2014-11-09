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

struct page **pages;
int initialinsertcounter = 0;

void opt_insert(struct page *p){
	pages[initialinsertcounter] = p;
	//printf("initial insert %d", initialinsertcounter);
	initialinsertcounter ++;
	
	// advance the usage time on pointer p (for initial use)
	map_next(p->vaddr);
}

int opt_evict(struct page *p) {
	// advance the usage time pointer on p(for some intermittent use)
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

	if(debug)
	printf("evicting slot %d (%x)  for %x\n", 
		curmax, 
		coremap[curmax].vaddr,
		p->vaddr);	

	//TODO set the evicted page's addr to -1;
	pages[curmax]->pframe = -1;
	pages[curmax] = p;

	return curmax;
}

void opt_advanceptr(struct page *p){
	// step forward the "accessed" pointer on the hashmap
	// yea dog
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
	int line = 0;

	pages = malloc(sizeof(struct page *) * memsize);

	char buf[MAXLINE];
	FILE *file = fopen(tracefile, "r");
	
	char type;
	int length;
	addr_t vaddr;

	// initialize the global map
	map_init();

	// read file line by line and build the hashmap 
	while(fgets(buf, MAXLINE, file)) {
		line++;
		
		if(buf[0] != '=') {
			sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
			// only pass the page in (drop the last 12 bits)
			map_insert((vaddr & ~ 0xfff), line);
		}
	}

	// reset the "trace" on the map (so the linked lists
	// in each slot can be consumed as if they were streams)
	map_rsttrace();
	if(debug) {	
		map_print();
	}
}


