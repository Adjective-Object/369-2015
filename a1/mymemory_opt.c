#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>
#include "malloc_structs.h"


#define WORDSIZE (__WORDSIZE/8)
#define BLOCK_MININTERNAL 512
#define BLOCK_MINSIZE (BLOCK_MININTERNAL + BLOCK_OVERHEAD)

static pthread_mutex_t biglock = PTHREAD_MUTEX_INITIALIZER;

// pointer to the head of the linked list of nodesmemhead *free_head = NULL;
memhead *free_head;
void *sbrk_start = NULL;    // first address of sbrked memory
void *sbrk_end = NULL;      // last address of sbrked memory
int needsinit = 1;

//inits the user malloc
void initusermalloc() {
    //printf("\t\ninitializing user malloc\n");

    sbrk_start = (void *)sbrk(SBRK_STEP+sizeof(unsigned int)); 
    free_head = (memhead *)sbrk_start;
    sbrk_end = (void *)(sbrk(0));

    //printf("\tsbrk starts at %p\n", sbrk_start);
    //printf("\tsbrk ends at %p\n", sbrk_end);

    unsigned int diff = (intptr_t)(sbrk_end) - (intptr_t)(free_head);

    initmemblock(
        free_head, 
        size_etoi(diff));
    free_head->next = free_head;
    free_head->prev = free_head;
    //printf("\tinitialization complete\n");
}

/* drops <size> bytes from the beginning of block described
by node and returns a pointer to the new block.
i.e. 

|         large node          |    
| cropped node |  remainder   |

and cropblock returns the pointer second node
*/
memhead *cropblock(memhead *node, unsigned int isize) {
    /*
    //printf("\tcropping block internal size %d from node (head=%p)\n", 
        isize, node);
    */

    //printf("initial %p %p\n", node, node_after(node));

    int oldsize = node->size;
    initmemblock(node, isize);
    memhead *second = node_after(node);
    //printf("\tsecond %p\n", second);
    initmemblock(second, oldsize - size_itoe(isize));
    second->next = node->next;

    //printf("second -> next %p node->next %p\n", second->next, node->next);
    return second;
}

memhead *handleexistingblock(memhead *prev, memhead *scan, unsigned int isize) {
    //garaunteed that prev != next 
    memhead* next;
    if (0 && scan->size - isize > BLOCK_MINSIZE) {
        // crop to size and point to remainder
        next = cropblock(scan, isize);
    }
    else {
        next = scan->next;
    }

    prev->next = next;
    next->prev = prev;
    free_head = next;

    return scan;
}

memhead *makenewblock(unsigned int isize) {
    unsigned int esize = size_itoe(isize);
    //printf("\tsbrking new (%d)\n", esize);

    //sbrk new space, create memblock
    void *oldend = sbrk_end;
    sbrk(esize+sizeof(unsigned int));
    sbrk_end = (void *)(sbrk(0));

    initmemblock(oldend, isize);

    return (memhead *) oldend;
}

/*finds a block of size > size and the pops it from the list*/
memhead *findblockfitting(unsigned int isize){
    // ** so it can change the value of free_head
    // when prev is null (i.e. when first element works),
    // free_head is pointed to the free_head->next
    memhead *scan;
    memhead *prev;
    if(free_head != NULL){
        scan = free_head->next;
        prev = free_head;
    } else {
        //printf("list is empty, making a new block\n");
        return makenewblock(isize);
    }

    //printf("\thead of list: %p\n", scan);

    while( scan!=free_head &&  
            (scan->size - isize) > BLOCK_MINSIZE){
        prev = scan;
        scan = scan->next;
    }

    //printf("prev: %p, scan %p\n", prev, scan);

    // if at end of list and nothing is big 
    // enough, sbrk something of appropriate size
    if(prev == scan){
        // list is a single cyclical element
        //printf("list is a single element.\n");
        if((scan->size - isize) > BLOCK_MINSIZE){
            // make a new one if is not right size
            //printf("making new block\n");
            return makenewblock(isize);
        }
        else{
            // otherwise just return the head
            //printf("returning root element\n");
            free_head = NULL;
            return scan;
        }
    }

    else if(scan == free_head){
        // no nodes are large enough
        //printf("no nodes are large enough\n");
        return makenewblock(isize);
    }
    
    else {
        // there is an existing node large enough
        //printf("handling node\n");
        return handleexistingblock(prev, scan, isize);
    }
}

#define SYSTEM_MALLOC 0
/* mymalloc: allocates memory on the heap of the requested size. The block
             of memory returned should always be padded so that it begins
             and ends on a word boundary.
     unsigned int size: the number of bytes to allocate.
     retval: a pointer to the block of memory allocated or NULL if the 
             memory could not be allocated. 
             (NOTE: the system also sets errno, but we are not the system, 
                    so you are not required to do so.)
*/
void *mymalloc(unsigned int size) {
    #if SYSTEM_MALLOC
    return malloc(size);
    #endif


    //return NULL on malloc 0
    if (size == 0){ return NULL; }

    //resize to multiple of WORDSIZE
    if(size%WORDSIZE != 0){ size = (size/WORDSIZE+1)*WORDSIZE; }

    //lock
    pthread_mutex_lock(&biglock);

    //printf("mymalloc %d\n", size);
    //printf("header %p\n", free_head);
    
    //initialize the list
    //if the list needs initialization
    if (needsinit){
        initusermalloc();
        needsinit = 0;
    }

    memhead *mptr = findblockfitting(size);
    mptr->isfree = 0;
    
    //printf("%p\t(%d)\tfor size %d\n", mptr, mptr->size, size);
    //printf("\tfound block (%p) fitting: %d\n", mptr, size);

    //printf("\tusermalloc %d complete\n\n", size);
    //debug_printmemlist(free_head);
    //printf("malloc done \n");

    pthread_mutex_unlock(&biglock);

    return ptr_etoi(mptr);    
}

unsigned int check_backward_coalesce(memhead* in) {

    memhead *prev = node_before(in);
    if((void *)prev >= sbrk_start && prev->isfree){
        /*
        printf("coalesce back\n");
        printf("(this %p) (this.size %d) (prec %p) (prec.size %d)\n",
            in, in->size, prev, prev->size
        );*/

        //debug_printlocalmemlist(prev,10);

        prev->size = prev->size + size_itoe(in->size);
        int *tail = node_tail(prev);
        *tail = size_itoe(prev->size);

        //debug_printlocalmemlist(prev,10);

        return 1;
    }
    return 0;
}

unsigned int forward_coalesce(memhead *in, memhead *after){
    //printf("coalesce forward in %p, after %p\n", in, after);

    //debug_printmemlist(free_head);

    memhead *nxt = after->next;
    in->next = nxt;
    nxt->prev = in;

    in->size = in->size + size_itoe(after->size);

    memhead *oldprev = (after->prev);

    oldprev->next = in;
    in->prev = oldprev;

    int *tail = node_tail(in);
    *tail = size_itoe(in->size);

    if(free_head == after){
        // printf("reassigning free_head\n");
        free_head = in;
    }

    //debug_printlocalmemlist(in,10);
    // printf("coalesce done\n");

    return 1;
}

unsigned int check_forward_coalesce(memhead* in) {
    memhead *after = node_after(in);
    if (0 && (void *)after < sbrk_end && after->isfree){

        forward_coalesce(in, after);
    }

    return 0;
}

void insert_node(memhead *insert){
    if(free_head == NULL) {
        free_head = insert;
        insert->next = insert;
        insert->prev = insert;
    } else {
        memhead *nxt = free_head->next;
        
        insert->prev=free_head;
        insert->next = nxt;

        free_head->next = insert;
        nxt->prev = insert;

    }
}

/* myfree: unallocates memory that has been allocated with mymalloc.
     void *ptr: pointer to the first byte of a block of memory allocated by 
                mymalloc.
     retval: 0 if the memory was successfully freed and 1 otherwise.
             (NOTE: the system version of free returns no error.)
*/
unsigned int myfree(void *ptr) {
    #if SYSTEM_MALLOC
    free(ptr);
    return 0;
    #endif

    pthread_mutex_lock(&biglock);
    memhead *ext = ptr_itoe(ptr);
    //printf("\nmyfree %p (%p)\n", ptr, ext);
    //printf("header %p\n", free_head);

    //check if in valid range
    if ((sbrk_start > ptr) || (sbrk_end < ptr)) {
        pthread_mutex_unlock(&biglock);
        return 1;
    }

    //check magic number
    if (!valid_node(ext)) {
        pthread_mutex_unlock(&biglock);
        return 1;
    }

    ext->isfree = 1;
    
    //coalesce
    //printf("check coalesce\n");
    if(check_backward_coalesce(ext) || check_forward_coalesce(ext)) {
        //printf("coalesce done\n");

        //printf("free done\n");

        pthread_mutex_unlock(&biglock);
        return 0;
    }
    //printf("no coalesce\n");

    insert_node(ext);

    //debug_printmemlist(free_head);

    //printf("free done \n");
    pthread_mutex_unlock(&biglock);

    return 0;
}