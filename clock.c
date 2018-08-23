#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_hand;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
    while (clock_hand < memsize) {
        if (coremap[clock_hand].pte->frame & PG_REF){ // if the page has been reference and the clock hand reaches it then set ref bit to 0
            coremap[clock_hand].pte->frame = coremap[clock_hand].pte->frame & ~PG_REF;
        } else {
            clock_hand++; // reached a page that has not been referenced
            return clock_hand - 1;
        }
        clock_hand++;
    }
    clock_hand = 0;
	
	return clock_evict(); // repeat the cycle
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {

	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
    clock_hand = 0;
}
