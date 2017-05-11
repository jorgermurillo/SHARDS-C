#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include "splay.h"

unsigned int calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table);

void update_dist_table_fixed_size(uint64_t  reuse_dist, GHashTable **dist_table, uint64_t T_new);

GHashTable *MRC(GHashTable  **dist_table);

GHashTable *MRC_fixed_size(GHashTable  **dist_table, uint64_t T_new);

int intcmp(const void *x, const void *y);

int doublecmp(const void *x, const void *y);