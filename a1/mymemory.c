#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#define SBRK_INITIAL 512
#define BLOCK_OVERHEAD (sizeof(int) + sizeof(memhead))
#define BLOCK_MININTERNAL 8
#define BLOCK_MINSIZE (BLOCK_MININTERNAL + BLOCK_OVERHEAD)
#define MAGIC 0x45

// blocks of malloced memory are stored as follows:
// next
// size 
// ... 
// malloced memory of size SIZE
// size

typedef struct memhead {
    void * next;
    unsigned int size;
    unsigned int magic;
} memhead;

pthread_mutex_t biglock = PTHREAD_MUTEX_INITIALIZER;

// pointer to the head of the linked list of nodesmemhead *free_head = NULL;
memhead *free_head;
int sbrk_start = NULL;    // first address of sbrked memory
void *sbrk_end = NULL;      // last address of sbrked memory
int needsinit = 1;

// converts external size of block to internal size
unsigned int size_etoi(unsigned int size){
    return size - BLOCK_OVERHEAD;
}
// converts internal size of block to external size 
unsigned int size_itoe(unsigned int internalsize){
    return internalsize + BLOCK_OVERHEAD;
}

//points to the space at the end of the node (i.e. after it)
void *nodeend(memhead *node){
    return (void *)(node) + (node->size) + BLOCK_OVERHEAD;
}

void *getinternalptr(memhead *node){
    return (void *)(node) + sizeof(memhead);
}

void *getexternalptr(memhead *internal){
    return (void *)(internal) - sizeof(memhead);
}

// initializes a block of memory
void initmemblock(memhead* head, unsigned int isize){
    unsigned int esize = size_itoe(isize);
    void *tailpointer = (void *)(head) + esize;

    /*
    printf("\tdeclaring mem block from %p to %p\n\t\t(size i=%d e=%d)\n", 
        head, tailpointer,
        isize, esize);
    */
    head->size = isize;
    head->magic = MAGIC;
    *((int *)tailpointer) = esize;
}

//inits the user malloc
void initusermalloc() {
    //printf("\t\ninitializing user malloc\n");

    sbrk_start = sbrk(SBRK_INITIAL+sizeof(unsigned int)); 
    free_head = (memhead *)sbrk_start;
    sbrk_end = (void *)(sbrk(0));

    //printf("\tsbrk starts at %p\n", sbrk_start);
    //printf("\tsbrk ends at %p\n", sbrk_end);

    unsigned int diff = (intptr_t)(sbrk_end) - (intptr_t)(free_head);

    initmemblock(
        free_head, 
        size_etoi(diff));
    free_head->next = NULL;
    //printf("\tinitialization complete\n");
}

void debug_printmemlist() {
    memhead *node = free_head;
    //printf("\n");
    if(node == NULL){
        //printf("\tlist empty\n");
    }
    while(node != NULL){
        /*printf("\t(start %p, end %p, size %d, next %p)\n",
            node,
            nodeend(node),
            node->size,
            node->next);
            */
        node = node->next;
        if(node == free_head){
            printf("--INFINITE_LOOP--\n");
            exit(1);
        }
    }
    //printf("\n");
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
    printf("\tcropping block internal size %d from node (head=%p)\n", 
        isize, node);
    */

    unsigned int esize = size_itoe(isize);
    memhead *second = (void *)(node) + esize +sizeof(int);
    //printf("\tsecond %p\n", second);
    initmemblock(second, node->size - esize -sizeof(int));
    second->next = node->next;
    initmemblock(node, isize);
    return second;
}

memhead *existingblock(memhead *prev, memhead *scan, unsigned int isize) {
    memhead **g;
    if (prev != NULL){
        g = &(prev->next);
    } else{
        //printf ("overwriting head\n");
        g = &(free_head);
    }
    if (scan->size - isize > BLOCK_MINSIZE) {
        // crop to size and point to remainder
        /*
        printf("\tfound block (%p, %d), cropping to correct size\n",
            scan, scan->size);
        */
        *g = cropblock(scan, isize);
        return scan;
    } 
    else {
        // remove the block from the list
        /*
        printf("\tfound block (%p, %d), size is close enough\n",
            scan, scan->size);
        */
        *g = scan->next;
        return scan;
    }
}

memhead *newblock(unsigned int isize) {
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
    memhead *scan = free_head;
    memhead *prev = NULL;

    ////printf("\thead of list: %p\n", scan);

    while ( scan!=NULL &&  
            scan->size <= isize){
        prev = scan;
        scan = scan->next;
    }

    ////printf("\tsbrk_end is %p\n", sbrk_end);

    ////printf("\tstopped at (prev %p) (scan %p)\n", prev, scan);
    

    // if at end of list and nothing is big 
    // enough, sbrk something of appropriate size
    if(scan == NULL){
        //printf("\t> making new block\n");
        return newblock(isize);
    }
    else {
        //printf("\t> returning existing block\n");
        return existingblock(prev, scan, isize);
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

    pthread_mutex_lock(&biglock);
    //initialize head of ptrlst
    if (needsinit){
        initusermalloc();
        needsinit = 0;
    }

    //printf("\t\nmymalloc %d (e %d)\n", size, size_itoe(size));

    memhead *mptr = findblockfitting(size);
    //printf("\tfound block (%p) fitting: %d\n", mptr, size);

    //printf("\tusermalloc %d complete\n\n", size);
    //debug_printmemlist();
    pthread_mutex_unlock(&biglock);


    return getinternalptr(mptr);    
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
    //printf("\nmyfree %p (%p)", ptr, getexternalptr(ptr));

    memhead *ext = getexternalptr(ptr);
    
    //check magic number
    if (ext->magic != MAGIC){
        //printf(" MAGIC NOT MAGIC!\n");
        return 1;
    }

    //printf("\n");
    ext->next = free_head;
    free_head = ext;
    //debug_printmemlist();
    pthread_mutex_unlock(&biglock);

    return 0;
}