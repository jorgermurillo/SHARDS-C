#ifndef __SHARDS_H
#define __SHARDS_H


#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdint.h>
#include "murmurhash3.h"
#include "splay.h"

typedef enum{
	FIXED_RATE,
	FIXED_SIZE,
} shardsVersion;

typedef enum {
	String,
	Int,
	Int64,
	Double,
	Pointer,
} objectType;

typedef struct shards_elem SHARDS;

struct shards_elem {
	
	shardsVersion version;

	double R;
	uint64_t P;
	uint64_t T;

	//Data structures needed for SHARDS_fixed_rate
	/*
		dist_tree=
		time_table pairs each read object with the last reference time it was read.


	*/
	Tree *dist_tree;
	GHashTable *time_table;
	GHashTable *dist_histogram;	
	//Additional data structures needed for SHARDS_fixed_size
	GHashTable *set_table;	
	Tree *set_tree;
	Tree *evic_tree;
	GList *set_list;
	GList *set_list_search;


	unsigned int S_max;
	unsigned int set_size;	

	unsigned int bucket_size; 

	//Counter for the amount of evicted objects
	unsigned int evic_obj; 
	

	int total_objects;
	int num_obj;
	double fraction;
};


SHARDS* SHARDS_fixed_rate_init(double R_init, unsigned int bucket_size, objectType type);

SHARDS* SHARDS_fixed_size_init(unsigned int  max_setsize, unsigned int bucket_size, objectType type);

SHARDS* SHARDS_fixed_size_init_R(unsigned int  max_setsize, double R_init, unsigned int bucket_size, objectType type);

void SHARDS_feed_obj(SHARDS *shards, void* object, size_t nbytes);

void SHARDS_free(SHARDS* shards);

//private functions
unsigned int calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table);

void update_dist_table_fixed_size(uint64_t  reuse_dist, GHashTable **dist_table, uint64_t T_new);

GHashTable *MRC(SHARDS* shards);

GHashTable *MRC_fixed_size(SHARDS *shards);

GHashTable *MRC_free(SHARDS* shards);

GHashTable *MRC_fixed_size_free(SHARDS *shards);

int intcmp(const void *x, const void *y);

int doublecmp(const void *x, const void *y);

#endif