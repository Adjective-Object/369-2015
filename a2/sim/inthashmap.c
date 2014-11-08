#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef HASHMAP_TERRIBLE_KUN
#define HASHMAP_TERRIBLE_KUN

// below is a hashmap of int -> dynamically sized list of ints
// in order to ap the actual pointer values to the indecies of 
// the pointer-access order arrays

#define NUM_BINS 10

struct lstelem{
	int val;
	struct lstelem *next;
};

struct hashpair{
	int key;
	struct hashpair *next;
	
	struct lstelem *head;
	struct lstelem *trace;
};

// array of pointers to hashpairs;
struct hashpair *hashbins[NUM_BINS];


// laziest hashfunction ever
int hash(int x){
	return x % NUM_BINS;
}

// appends an item to a list
void lst_append(struct lstelem *node, int value) {
	struct lstelem *new = malloc(sizeof(struct lstelem));
	new->val = value;
	node->next = new;
}

// initializes an all-null set of bins
void map_init(){
	int i;
	for (i=0; i<NUM_BINS; i++) {
		hashbins[i] = NULL;
	}
}

// creates a hashpair with an empty list element
struct hashpair *makenewhashpair(int key, int value){
	struct hashpair *pair = malloc(sizeof(struct hashpair));
	struct lstelem *elem = malloc(sizeof(struct lstelem));
	elem->val = value;
	elem->next = NULL;

	pair->trace = elem;
	pair->head = elem;
	pair->key = key;

	return pair;
}

// puts an item at the end of the list pointed to by a given bin
void map_insert(int key, int value) {
	int hval = hash(key);

	struct hashpair *prev = NULL;
	struct hashpair *cur  = hashbins[hval];
	while (cur != NULL){
		if (cur->key == key){
			lst_append(cur->trace, value);
			cur->trace = cur->trace->next;
			return;		
		}
		prev = cur;
		cur = cur->next;
	}
	if (prev != NULL)
		prev->next = makenewhashpair(key, value);
	else
		hashbins[hval] = makenewhashpair(key,value);
}

// gets an item from the 'trace' element of the ll pointed to
// by the key in this hashmap
int map_retr(int key, bool advancePtr) {
	int hval = hash(key);
	struct hashpair *cur  = hashbins[hval];
	while (cur!= NULL){
		if (cur->key == key){
			if(cur->trace == NULL){
				return INT_MAX;
			}
			int toret = cur->trace->val;
			if (advancePtr){
				cur->trace = cur->trace->next;
			}
			return toret;
		}
		cur = cur->next;
	}
	return -1;
}

int map_next(int key){
	return map_retr(key, true);
}

int map_peek(int key){
	return map_retr(key, false);
}

void map_rsttrace(){
	int i;
	for (i=0; i<NUM_BINS; i++){
		struct hashpair *pair = hashbins[i];
		while (pair != NULL) {
			pair->trace = pair->head;
			pair = pair->next;
		}
	}
}

void lst_print(struct lstelem *node) {
	if(node == NULL){
		printf("\n");
	} else{
		printf("%d ", node->val);
		fflush(stdout);
		lst_print(node->next);
	}
}


void hashl_print(struct hashpair *node) {
	if(node == NULL){
		printf("\n");
	} else{
		printf("(%x)\t", node->key);
		lst_print(node->head);
		printf("    ");
		fflush(stdout);
		hashl_print(node->next);
	}
}

void map_print() {
	int i;
	for (i=0; i<NUM_BINS; i++) {
		printf("%d : ", i);
		fflush(stdout);
		hashl_print(hashbins[i]);
	}
}

#endif
