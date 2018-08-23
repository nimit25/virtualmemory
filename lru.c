#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

struct Node {
    struct Node *prev;
    struct Node *next;
    unsigned int frame;
};

struct double_list {
    struct Node *first;
    struct Node *last;
};
struct double_list *list1;
struct Node **common_list;
int count;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
    if (list1->last == NULL){
        return 0; // list is empty
    }
    struct Node *last_node = list1->last;
    list1->last = last_node->prev;
    list1->last->next = NULL;
    int value = last_node->frame;
    coremap[value].ref_bit = 0; // the node is longer in the list
    free(last_node); // free memory
    return value;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
    int frame_p = p->frame >> PAGE_SHIFT;
    if (count == 0 || list1->first == NULL){ // when the list is empty
        struct Node *new_node = malloc(sizeof(struct Node));
        new_node->frame = (unsigned int) frame_p;
        list1->first = new_node;
        list1->last = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
        coremap[frame_p].ref_bit = 1;
        common_list[frame_p] = new_node;
        count++; // sanity check to see if the list is empty
    } else if (coremap[frame_p].ref_bit != 1) { // if the node is new but the list's first element is there
        coremap[frame_p].ref_bit = 1;
        struct Node *new_node = malloc(sizeof(struct Node));
        new_node->frame = (unsigned int) frame_p;
        list1->first->prev = new_node;
        new_node->prev = NULL;
        new_node->next = list1->first;
        list1->first = new_node;
        common_list[frame_p] = new_node;
    } else if (coremap[frame_p].ref_bit == 1){ // if the node is already in the list
        struct Node *new_node = common_list[frame_p];
        if (new_node->prev) {
            new_node->prev->next = new_node->next;
        }
        if (new_node->next) { // if the next node exists
            new_node->next->prev = new_node->prev;
        }
//        if (list1->last->frame == new_node->frame){
        else { // otherwise it's the last one
            list1->last = new_node->prev;
        }
        list1->first->prev = new_node;
        new_node->prev = NULL;
        new_node->next = list1->first;
        list1->first = new_node;
    }

	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
    list1 = malloc(sizeof(struct double_list));
    list1->first = NULL;
    list1->last = NULL;
    common_list = malloc(sizeof(struct Node *) * memsize);
    count = 0;

}
