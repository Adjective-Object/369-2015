#include <stdio.h>
#include <stdlib.h>
#include "avl.h"
#include <stdbool.h>

/* The avl tree is a simple way to get a fast look up table, and is
 * used to implement the page table.
 */
extern struct avl_table *avl_tree;
extern struct libavl_allocator avl_allocator_default;

typedef unsigned long addr_t;

struct page {
	addr_t vaddr; // key
	char type;    // Instruction or data
	int pframe;   // Page frame number. -1 if not in physical memory
	

	//last time accessed in terms of #s of operations
	//(used for FIFO)
	uint insert_time;

	//last time accessed in terms of #s of operations
	//(used for LRU)
	uint access_time;

	//if the block has been used (for LRU_Clock)
	bool referenced;
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

void rand_init();
void lru_init();
void clock_init();
void fifo_init();
void opt_init();

int rand_evict(struct page *p);
int lru_evict(struct page *p);
int clock_evict(struct page *p);
int fifo_evict(struct page *p);
int opt_evict(struct page *p);
