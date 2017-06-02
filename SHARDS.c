#include "SHARDS.h"

SHARDS* SHARDS_fixed_rate_init(double R_init, unsigned int bucket_size, objectType type){

	SHARDS *shards = malloc(sizeof(SHARDS));
	shards->version = FIXED_RATE;
	shards->R = R_init;

	uint64_t tmp = 1;
	tmp = tmp <<24;
	shards->P = tmp;

	shards->T = R_init*tmp;

	shards->bucket_size = bucket_size;

	shards->dist_tree = NULL;

	switch(type){
		case String:
			shards->time_table = g_hash_table_init_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int:
			shards->time_table = g_hash_table_init_full(g_int_hash, g_int_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int64:
			shards->time_table = g_hash_table_init_full(g_int64_hash, g_int64_equal, NULL, ( GDestroyNotify )free);		
			break;
		case Double:
			shards->time_table = g_hash_table_init_full(g_double_hash, g_double_equal, NULL, ( GDestroyNotify )free);
			break;
		case Pointer:
			shards->time_table = g_hash_table_init_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )free);
			break;
	}

	shards->dist_histogram = g_hash_table_new_full(g_int_hash, g_int_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);

	shards->set_table = NULL;	
	shards->set_tree = NULL;
	shards->set_list = NULL;
	shards->set_list_search = NULL;

	shards->S_max = 0;
	
	shards->set_size= 0;
	shards->evic_obj = 0; 
	

	shards->total_objects = 0;
	shards->num_obj = 0;
	shards->fraction = 0;

	return shards;
}


SHARDS* SHARDS_fixed_size_init(unsigned int max_setsize, unsigned int bucket_size, objectType type){

	SHARDS *shards = malloc(sizeof(SHARDS));
	shards->version = FIXED_SIZE;
	shards->S_max = max_setsize;

	shards->R = 0.1;

	uint64_t tmp = 1;
	tmp = tmp <<24;
	shards->P = tmp;

	shards->T = (shards->R)*tmp;

	shards->bucket_size = bucket_size;

	shards->dist_tree = NULL;

	switch(type){
		case String:
			shards->time_table = g_hash_table_init_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int:
			shards->time_table = g_hash_table_init_full(g_int_hash, g_int_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int64:
			shards->time_table = g_hash_table_init_full(g_int64_hash, g_int64_equal, NULL, ( GDestroyNotify )free);		
			break;
		case Double:
			shards->time_table = g_hash_table_init_full(g_double_hash, g_double_equal, NULL, ( GDestroyNotify )free);
			break;
		case Pointer:
			shards->time_table = g_hash_table_init_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )free);
			break;
	}

	shards->dist_histogram = g_hash_table_new_full(g_int_hash, g_int_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);

	shards->set_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )g_list_free);;	
	shards->set_tree = NULL;
	shards->set_list = NULL;
	shards->set_list_search = NULL;

	shards->set_size= 0;
	shards->evic_obj = 0; 
	

	shards->total_objects = 0;
	shards->num_obj = 0;
	shards->fraction = 0;

	return shards;


}

SHARDS* SHARDS_fixed_size_init_R(unsigned int  max_setsize, double R_init, unsigned int bucket_size, objectType type){

	SHARDS *shards = SHARDS_fixed_size_init(max_setsize, bucket_size, type);
	shards->R = R_init;
	shards->T = R_init*(shards->T);

	return shards;

}

void SHARDS_feed_obj(SHARDS *shards, void* object)

