#ifndef MYMALLOC_STRUCTS
#define MYMALLOC_STRUCTS

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>


#define SBRK_STEP 4096
#define BLOCK_OVERHEAD (sizeof(int) + sizeof(memhead))
#define MAGIC 0x45

// blocks of malloced memory are stored as follows:
// next
// size 
// ... 
// malloced memory of size SIZE
// size

typedef struct memhead {
    void *next;
    void *prev;
    unsigned int size;
    unsigned short magic;
    unsigned short isfree;
} memhead;

// converts external size of block to internal size
static unsigned int size_etoi(unsigned int size){
    return size - BLOCK_OVERHEAD;
}
// converts internal size of block to external size 
static unsigned int size_itoe(unsigned int internalsize){
    return internalsize + BLOCK_OVERHEAD;
}

//goes from the internal pointer to the external pointer
static void *ptr_etoi(memhead *node){
    return ((void *)(node)) + sizeof(memhead);
}

//goes fro the external pointer to the internal pointer
static void *ptr_itoe(memhead *internal){
    return ((void *)(internal)) - sizeof(memhead);
}



//points to the integer at the end of the node (the size)
static void *node_tail(memhead *node){
    return (void *)(node) + (node->size) + sizeof(memhead);
}

//points to the integer at the end of the node)
static void *node_after(memhead *node){
    return (void *)(node) + (node->size) + BLOCK_OVERHEAD;
}

//points to the integer at the end of the node)
static void *node_before(memhead *node){
    int *tail = ((int *)(node)) - 1;
    return ((void *)(tail)) - (*tail);
}



// initializes a block of memory
void initmemblock(memhead* head, unsigned int isize){
    /*
    printf("\tdeclaring mem block from %p to %p\n\t\t(size i=%d e=%d)\n", 
        head, tailpointer,
        isize, esize);
    */
    head->size = isize;
    head->magic = MAGIC;
    head->isfree = 1;
    void *tailpointer = node_tail(head);
    *((int *)tailpointer) = size_itoe(isize);
}

void debug_printmemlist(memhead *head) {
    memhead *node = head;
    printf("\n");
    if(node == NULL){
        printf("\tlist empty\n\n");
        return;
    }
    do {
        printf("\t(start %p, end %p, size %d, prev %p, next %p)\n",
            node,
            node_after(node),
            node->size,
            node->prev,
            node->next);
        
        node = node->next;
    } while(node != head);
    printf("\n");
}


void debug_printlocalmemlist(memhead *head, int num) {
    memhead *node = head;
    if(node == NULL){
        printf("\nempty\n\n");
        return;
    }

    int i=0;
    for(i=0; i<num/2; i++){
        printf("%p\n",node);
        if (node->prev == NULL){
             break;
        }
        node = node->prev;
    }

    memhead * init = node;

    for(i=0; i<num; i++){
        printf("\t(start %p,\tend %p,\tsize %d,\tprev %p,\tnext %p)\n",
            node,
            node_after(node),
            node->size,
            node->prev,
            node->next);
        
        node = node->next;

        if(node == init){
            break;
        }
    }
    printf("\n");
}

// checks if internal node is valid
static unsigned int valid_node(memhead *ptr){
    return ptr->magic == MAGIC;
}

#endif