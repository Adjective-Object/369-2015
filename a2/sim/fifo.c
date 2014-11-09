#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

extern int memsize;
extern int debug;
extern struct frame *coremap;

/* Handles the first insert of a page into LRU (i.e. inserts
 * as a result of cold misses)
 */

// track the head and the tail of the linked list
// nodes in the linked list are inserted at the front, so the
// oldest node will always be at the back of the linked list.
extern struct page *ll_head;
extern struct page *ll_tail;

void fifo_insert(struct page *p){

	//printf("inserting\n");
	
	// tell this node it comes before head       	
	p->next = ll_head;
	p->prev = NULL;
	if (ll_head != NULL)
		ll_head->prev = p;

	// insert it at the head of the linked list
	ll_head = p;

	// and set it as the tail too in a 1 element linked list
	if (ll_tail == NULL) 
		ll_tail = ll_head;
	
}

/* Page to evict is the tail of the list
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int fifo_evict(struct page *p) {

	// grab the tail and tell it it's not in memory
	int lru_frame = ll_tail->pframe;	
	ll_tail->pframe = -1;
	// printf("evicting from frame %d\n", ll_tail->pframe);

	struct page  *newtail = ll_tail->prev;
	// previous will always be non-null
	// because evict is only ever called on a
	// full list.
	newtail->next = NULL;

	// insert the node at the head of the list
	p->prev = NULL;
	p->next = ll_head;

	// edge case for 1 mem bin
	if (ll_head != p)
		ll_head->prev = p;
	ll_head = p;

	// and set the old tail
	ll_tail = newtail;
	
       	return lru_frame;
}

// initialize fifo
void fifo_init() {
	//printf("fifo init\n");

	ll_head = NULL;
	ll_tail = NULL;
}
