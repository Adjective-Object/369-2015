#include <stdio.h>
#include <stdlib.h>
#include "avl.h"
#include <stdbool.h>

#ifndef SIM_HEADER
#define SIM_HEADER

/* The avl tree is a simple way to get a fast look up table, and is
 * used to implement the page table.
 */
extern struct avl_table *avl_tree;
extern struct libavl_allocator avl_allocator_default;

// linked list tracking  used in lru and fifo
struct page *ll_head;
struct page *ll_tail;

typedef unsigned long addr_t;

struct page {
	addr_t vaddr; // key
	char type;    // Instruction or data
	int pframe;   // Page frame number. -1 if not in physical memory
	
	// if the block has been used (for clock)
	bool referenced;
	
	// next and previous counters, for putting blocks
	// in a linked list (for fifo, lru)
	struct page *next;
	struct page *prev;
};


int page_cmp(const void *a, const void *b, void *p); 
void init_pagetable();

struct page *pagetable_insert(addr_t vaddr, char type);
struct page *find_page(addr_t vaddr);
void print_pagetable(void);

struct frame {
	char in_use;   //
	char type;     //Instruction (I) or Data (D)
	addr_t vaddr;
};

// called before the trace is analyzed
void rand_init();
void clock_init();
void opt_init();
void fifo_init();

void opt_advanceptr();

//called when a page is inserted into memory (on cold miss)
void rand_insert();
void lru_insert();
void clock_insert();
void fifo_insert();
void opt_insert();

// called when a page is in memory and is accessed (on hit)
void rand_access();
void lru_access();
void clock_access();

// called when there is a capacity miss and somethig needs
// to be evicted from the page table
int rand_evict(struct page *p);
int lru_evict(struct page *p);
int clock_evict(struct page *p);
int fifo_evict(struct page *p);
int opt_evict(struct page *p);
#endif
