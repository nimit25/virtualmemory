//
// Created by Nimit Sachdeva on 2018-07-08.
//

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "sim.h"
#include "pagetable.h"


// extern int memsize;

extern int debug;

extern struct frame *coremap;

struct node {
    addr_t address;
    struct node *next;
};
int ref;
struct node *current; // the  current position in trace file in a linked list
struct node *next; // helper to make the list

/*
 * Find the next occurrence of the frame in the trace file
 */
void find_occurrence(int frame){
    struct node *ref_node = current;
    int count = -1;
    while (ref_node != NULL && ref_node->address != coremap[frame].vaddr){
        count++;
        ref_node = ref_node->next;
    }
    coremap[frame].next_ref = count;
}

/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {

    int max_ref = 0;
    int frame = 0;
    int next_ref = 0;
    for (int i = 0; i < ref; i++){
        find_occurrence(i);
        next_ref  = coremap[i].next_ref;
        if (next_ref == -1){ // if there is no occurrence
            return i;
        } else if (next_ref > max_ref){
            max_ref = next_ref;
            frame = i;
        }
    }
    return frame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
    if (ref < memsize){
        ref++;
    }
    current = current->next;

    return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
    ref = 0;
    FILE *file;
    char buf[MAXLINE];
    addr_t address = 0;
    char type;

    if ((file = fopen(tracefile, "r")) == NULL ){
        perror("fopen:");
        exit(1);
    }

    current = NULL;
    next = NULL;

    while (fgets(buf, MAXLINE, file) !=  NULL){
        if (buf[0] != '=') {
            sscanf(buf, "%c %lx", &type, &address);


            struct node *new_node = malloc(sizeof(struct node));
            if (new_node == NULL){
                perror("malloc:");
                exit(1);
            }
            new_node->address = address;
            new_node->next = NULL;
            if (current == NULL) {
                next = new_node;
                current = next;
            } else {
                next->next = new_node;
                next = new_node;
            }
        }


    }
    fclose(file);

}