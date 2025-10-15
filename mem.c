/* mem.c A template
 * My Name is
 * My User ID is
 * My GitHub ID is
 * Lab4: Dynamic Memory Allocation
 * ECE 2230, Fall 2025
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>

#include "mem.h"

// Global variables required in mem.c only
static int Coalescing = FALSE;      // default FALSE. TRUE if memory returned to free list is coalesced 
static int SearchPolicy = FIRST_FIT;   // default FIRST_FIT.  Can change to BEST_FIT
// NEVER use DummyChunk in your allocation or free functions!!
static mem_chunk_t DummyChunk = {0, &DummyChunk};
static mem_chunk_t *Rover = &DummyChunk;   // one time initialization

static int NumPages = 0;
static int NumSbrkCalls = 0;

// private function prototypes
void mem_validate(void);

/* function to request 1 or more pages from the operating system.
 *
 * new_bytes must be the number of bytes that are being requested from
 *           the OS with the sbrk command.  It must be an integer 
 *           multiple of the PAGESIZE
 *
 * returns a pointer to the new memory location.  If the request for
 * new memory fails this function simply returns NULL, and assumes some
 * calling function will handle the error condition.  Since the error
 * condition is catastrophic, nothing can be done but to terminate 
 * the program.
 */
mem_chunk_t *morecore(int new_bytes) 
{
    char *cp;
    mem_chunk_t *new_p;
    // preconditions that must be true for all designs
    assert(new_bytes % PAGESIZE == 0 && new_bytes > 0);
    assert(PAGESIZE % sizeof(mem_chunk_t) == 0);
    cp = sbrk(new_bytes);
    if (cp == (char *) -1)  /* no space available */
        return NULL;
    new_p = (mem_chunk_t *) cp;
    // Code to count the number of calls to sbrk, and the number of 
    // pages that have been requested
    NumSbrkCalls++; 
    NumPages += new_bytes/PAGESIZE;
    return new_p;
}

/* A function to change default operation of dynamic memory manager.
 * This function should be called before the first call to Mem_alloc.
 *
 * search_type: sets Search Policy to BEST_FIT (defults to FIRST_FIT)
 *
 * coalescing_state: sets Coalescing to TRUE (defaults to FALSE)
 */
void Mem_configure(int coalescing_state, int search_type)
{
    assert(coalescing_state == TRUE || coalescing_state == FALSE);
    assert(search_type == FIRST_FIT || search_type == BEST_FIT);
    Coalescing = coalescing_state;
    SearchPolicy = search_type;
}

/* deallocates the space pointed to by return_ptr; it does nothing if
 * return_ptr is NULL.  
 *
 * This function assumes that the rover pointer has already been 
 * initialized and points to some memory block in the free list.
 */
void Mem_free(void *return_ptr)
{
    // precondition
    assert(Rover != NULL && Rover->next != NULL);

    // two cases: if coalescing or not
    if (Coalescing == FALSE) {
        // insert after rover
    } else {
        // step 1: loop to find before and after location
        // step 2: insert between before and after and coalesce if possible
    }

    // obviously the next line is WRONG!!!!  You must fix it.
    free(return_ptr);
}

/* returns a pointer to space for an object of size nbytes, or NULL if the
 * request cannot be satisfied.  The memory is uninitialized.
 *
 * This function assumes that there is a Rover pointer that points to
 * some item in the free list.  
 */
void *Mem_alloc(const int nbytes)
{
    // precondition
    assert(nbytes > 0);
    assert(Rover != NULL && Rover->next != NULL);

    // Insert your code here to find memory block
    // mem_chunk_t *p;

    /**************** post-conditions   ************************************/
    // Before returning, you must check the block with post-conditions.
    //
    // Here are possible post-conditions. 
    // Include these tests before returning from this function.
    //
    // assume p is a pointer to the starting address of the memory block 
    // to be given to the user. The address returned to the user is p+1.
    //
    // Test that the memory block is just big enough.
    // The minus one in the next two tests accounts for the header
    //
    // assert((p->num_units-1)*sizeof(mem_chunk_t) >= nbytes);
    // assert((p->num_units-1)*sizeof(mem_chunk_t) < nbytes + sizeof(mem_chunk_t));
    // assert(p->next == NULL);  // saftey first!
    // return p+1;


    // obviously the next line is WRONG!!!!  You must fix it.
    return malloc(nbytes);
}

/* prints stats about the current free list
 *
 * -- number of items in the linked list including dummy item
 * -- min, max, and average size of each item (in bytes) but not considering
 *    dummy because its size is zero
 * -- total memory in free list (in bytes)
 * -- number of calls to sbrk and number of pages requested
 *
 * A message is printed if all the memory is in the free list
 */
void Mem_stats(void)
{
    int items_in_free_list = 0;   // count how many blocks are in the free list including dummy
    int bytes_in_free_list = 0;
    double average_block_size = 0; // do not include dummy for avg/min/max
    int min_block_size = INT_MAX;
    int max_block_size = 0;

    printf("\n\tthe student must implement mem stats.  When done delete this line\n\n");

    printf("  --- Free list stats ---\n");
    printf("\tCount of items : %d\n", items_in_free_list);
    printf("\tMemory in list : %d (bytes)\n", bytes_in_free_list);
    printf("\t           avg : %g (bytes)\n", average_block_size);
    printf("\t           min : %d (bytes)\n", min_block_size);
    printf("\t           max : %d (bytes)\n", max_block_size);
    printf("\tCalls to sbrk  : %d\n", NumSbrkCalls);
    printf("\tNumber of pages: %d\n", NumPages);
    if (bytes_in_free_list == NumPages * PAGESIZE) {
        printf("  all memory is in the heap -- no leaks are possible\n");
    }
    // if list is empty then just one item in the list.  It is the dummy
    // So min is zero by default.  Do not consider dummy in avg/min/max.
    assert(min_block_size > 0 || items_in_free_list == 1);
}

/* print table of memory in free list 
 *
 * The print should include the dummy item in the list 
 *
 * A unit is the size of one mem_chunk_t structure
 */
void Mem_print(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    mem_chunk_t *p = Rover;
    mem_chunk_t *start = p;
    do {
        // example format.  Modify for your design
        printf("p=%p, num=%d (units), end=%p, next=%p %s\n", 
                p, p->num_units, p + p->num_units, p->next, 
                p->num_units!=0?"":"<-- dummy");
        p = p->next;
    } while (p != start);
    mem_validate();
}

/* This is an experimental function to attempt to validate the free
 * list when coalescing is used.  It is not clear that these tests
 * will be appropriate for all designs.  If your design utilizes a different
 * approach, that is fine.  You do not need to use this function and you
 * are not required to write your own validate function.
 */
void mem_validate(void)
{
    // note position of Rover is not changed by this function
    assert(Rover != NULL && Rover->next != NULL);
    assert(Rover->num_units >= 0);
    int wrapped = FALSE;
    int found_dummy = FALSE;
    mem_chunk_t *p, *largest, *smallest;

    p = Rover;
    do {
        if (p->num_units == 0) {
            assert(found_dummy == FALSE);
            found_dummy = TRUE;
        } else {
            assert(p->num_units > 0);
        }
        p = p->next;
    } while (p != Rover);
    assert(found_dummy == TRUE);

    if (Coalescing) {
        do {
            if (p >= p->next) {
                // this is not good unless at the one wrap around
                if (wrapped == TRUE) {
                    printf("validate: List is out of order, already found wrap\n");
                    printf("first largest %p, smallest %p\n", largest, smallest);
                    printf("second largest %p, smallest %p\n", p, p->next);
                    assert(wrapped == FALSE);   // stop and use gdb
                } else {
                    wrapped = TRUE;
                    largest = p;
                    smallest = p->next;
                }
            } else {
                assert(p + p->num_units < p->next);
            }
            p = p->next;
        } while (p != Rover);
        assert(wrapped == TRUE);
    }
}
/* vi:set ts=8 sts=4 sw=4 et: */

