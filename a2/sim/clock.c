#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pagetable.h"


extern int memsize;
extern int debug;
extern struct frame *coremap;

struct page **slots;
int clockhand = 0;
int insertedcount = 0;

/* Page to evict is chosen using the accurate clock algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

void clock_insert(struct page *p){
	p->referenced=true;
	slots[insertedcount] = p;
	insertedcount++;
}

int clock_evict(struct page *p) {
	while (slots[clockhand]->referenced){
		slots[clockhand]->referenced = false;
		clockhand = (clockhand + 1) % memsize;
	}
	
	int toret = slots[clockhand]->pframe;
	slots[clockhand]->pframe = -1;
	slots[clockhand] = p;
	p->referenced = true;
	return toret;
}

void clock_access(struct page *p) {
	p->referenced = true;
}

void clock_init(struct page *p) {
	slots = malloc(sizeof(struct page *) * memsize);
}
