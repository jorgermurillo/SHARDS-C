#ifndef __SHARDS_H
#define __SHARDS_H


#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include "shards_utils.h"

typedef enum{
	FIXED_RATE,
	FIXED_SIZE,
} shards_version;

typedef enum {
	String,
	Int,
	Uint64,
	Double,
} object_Type;

typedef struct shards_elem SHARDS;

struct shards_elem {
	
	shards_version version;
	object_Type dataType;

	double initial_R_value;
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
	

	unsigned int total_objects;
	int num_obj;
	double fraction;
};


SHARDS* SHARDS_fixed_rate_init(double R_init, unsigned int bucket_size, object_Type type);

SHARDS* SHARDS_fixed_size_init(unsigned int  max_setsize, unsigned int bucket_size, object_Type type);

SHARDS* SHARDS_fixed_size_init_R(unsigned int  max_setsize, double R_init, unsigned int bucket_size, object_Type type);

void SHARDS_feed_obj(SHARDS *shards, void* object, size_t nbytes);

void SHARDS_free(SHARDS* shards);

GHashTable *MRC(SHARDS* shards);

GHashTable *MRC_empty(SHARDS* shards);

//private functions
/*
unsigned int calc_reuse_dist(void *object, unsigned int num_obj, GHashTable **time_table, Tree **tree, shards_version version);

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table);

void update_dist_table_fixed_size(uint64_t  reuse_dist, GHashTable **dist_table, uint64_t T_new);


GHashTable *MRC_fixed_rate(SHARDS* shards);

GHashTable *MRC_fixed_rate_empty(SHARDS* shards);

GHashTable *MRC_fixed_size(SHARDS *shards);

GHashTable *MRC_fixed_size_empty(SHARDS *shards);

int intcmp(const void *x, const void *y);

int uint64cmp(const void *x, const void *y);

guint g_uint64_hash (gconstpointer v);

gboolean g_uint64_equal (gconstpointer v1, gconstpointer v2);

int doublecmp(const void *x, const void *y);

bool dummy(void* x);
*/

#endif