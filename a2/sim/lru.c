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
extern struct page *ll_head;
extern struct page *ll_tail;

// because fifo is implemented as a linked list, lru
// only needs to promote items to the top of the linked
// list on access

void lru_access(struct page *p){
	// printf("promoting %d to head\n", p->pframe);

	//connect the nodes on either side of p
	struct page *prev = p->prev;
	struct page *next = p->next;

	if (prev != NULL)
		prev->next = next;

	if (next != NULL)
		next->prev = prev;

	if (p == ll_tail)
		ll_tail = prev;

	if (p == ll_head)
		ll_head = next;

	//and put it at the head
	p->next = ll_head;
	p->prev = NULL;	
	ll_head = p;
}
