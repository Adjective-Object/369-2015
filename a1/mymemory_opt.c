#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <limits.h>
#include "malloc_structs.c"

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
    //printf("\tinitialization complete\n");
}

void debug_printmemlist() {
    memhead *node = free_head;
    printf("\n");
    if(node == NULL){
        printf("\tlist empty\n\n");
        return;
    }
    do {
        printf("\t(start %p, end %p, size %d, next %p)",
            node,
            node_after(node),
            node->size,
            node->next);
        
        node = node->next;
    } while(node != free_head);
    printf("\n");
}

/* drops <size> bytes from the beginning of block described
by node and returns a pointer to the new block.
i.e. 

|         large node          |    
| cropped node |  remainder   |

and cropblock returns the pointer second node
*/
memhead *cropblock(memhead *node, unsigned int isize) {

    //printf("\tcropping block internal size %d from node (head=%p)\n", isize, node);

    unsigned int esize = size_itoe(isize);
    memhead *second = (void *)(node) + esize +sizeof(int);
    
    //printf("\tsecond %p\n", second);
    initmemblock(second, node->size - esize -sizeof(int));

    second->next = node->next;
    initmemblock(node, isize);

    return second;
}

memhead *handleexistingblock(memhead *prev, memhead *scan, unsigned int isize) {
    //garaunteed that prev != next 
    memhead* next;
    if (scan->size - isize > BLOCK_MINSIZE) {
        // crop to size and point to remainder
        next = cropblock(scan, isize);
    } 
    else {
        next = scan->next;
    }

    prev->next = next;
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

    // //printf("\thead of list: %p\n", scan);

    while( scan!=free_head &&  
            scan->size < isize){
        prev = scan;
        scan = scan->next;
    }

    // if at end of list and nothing is big 
    // enough, sbrk something of appropriate size
    if(prev == scan){
        // list is a single cyclical element
        //printf("list is a single element.\n");
        if(scan->size < isize){
            // make a new one if is not right size
            //printf("making new block\n");
            return makenewblock(isize);
        }
        else{
            // otherwise just return
            //printf("returning new element\n");
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

    if (size == 0){
        //printf("SHIT SON SIZE 0\n");
        return NULL;
    }

    //printf("%d resized to ",size);

    if(size%WORDSIZE != 0){
        size = (size/WORDSIZE+1)*WORDSIZE;
    }

    //printf("%d\n",size);

    pthread_mutex_lock(&biglock);
    //initialize head of ptrlst
    if (needsinit){
        initusermalloc();
        needsinit = 0;
    }


    memhead *mptr = findblockfitting(size);
    //printf("\tfound block (%p) fitting: %d\n", mptr, size);

    //printf("\tusermalloc %d complete\n\n", size);
    //debug_printmemlist();
    pthread_mutex_unlock(&biglock);

    return ptr_etoi(mptr);    
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
    //printf("\nmyfree %p (%p)\n", ptr, ptr_itoe(ptr));

    memhead *ext = ptr_itoe(ptr);
    
    //check if in valid range
    if ((sbrk_start > ptr) || (sbrk_end < ptr)){
        return 1;
    }

    //check magic number
    if (!valid_node(ext)){
        //printf(" MAGIC NOT MAGIC!\n");
        return 1;
    }

    //printf("\n");

    if(free_head == NULL) {
        free_head = ext;
        ext->next = ext;
    } else {
        memhead *nxt = free_head->next;

        ext->next = nxt;
        free_head->next = ext;
    }

    //debug_printmemlist();
    pthread_mutex_unlock(&biglock);

    return 0;
}