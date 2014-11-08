#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef HASHMAP_TERRIBLE_KUN
#define HASHMAP_TERRIBLE_KUN

// below is a hashmap of int -> dynamically sized list of ints
// in order to map the actual pointer values to the indecies of 
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
		prev = cur;
		cur = cur->next;
		if (cur->key == key){
			lst_append(cur->trace, value);
			cur->trace = cur->trace->next;
			return;
		}
	}
	prev->next = makenewhashpair(key, value);
}

// gets an item from the 'cur' element of the ll pointed to
// by the key in this hashmap
int map_retr(int key) {
	int hval = hash(key);
	struct hashpair *cur  = hashbins[hval];
	while (cur!= NULL){
		if (cur->key == key){
			int toret = cur->trace->val;
			cur->trace = cur->trace->next;		
			return toret;
		}
		cur = cur->next;
	}
	return -1;
}

void lst_print(struct lstelem *node) {
	if(node == NULL){
		printf("\n");
	} else{
		printf("%d ", node->val);
	}
	lst_print(node->next);
}


void hashl_print(struct hashpair *node) {
	if(node == NULL){
		printf("\n");
	} else{
		printf("%d ", node->key);
	}
	hashl_print(node->next);
}

void map_print() {
	int i;
	for (i=0; i<NUM_BINS; i++) {
		printf("%d - ", i);
		lst_print(hashbins[i]->head);
	}
}

#endif
