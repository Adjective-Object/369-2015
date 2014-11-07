#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;
extern int debug;
extern struct frame *coremap;

// list of ints of page numbers
struct frame *fifo_buffer;
// points to the next thing to be evicted
int fifo_count;

/* Page to evict is chosen using the fifo algorithm
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int fifo_evict(struct page *p) {

	int pagenumber = *(fifo_buffer + fifo_count);
	fifo_count = (fifo_count + 1) % memsize;
	
	return curframe->vaddr;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	fifo_buffer = malloc(sizeof(struct frame) * memsize);
	fifo_count = 0;
}
