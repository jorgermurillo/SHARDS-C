#include "shards_utils.h"

unsigned int calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree){

		
	unsigned int reuse_dist=0;

	unsigned int *time_table_value =(unsigned int*) g_hash_table_lookup(*time_table, object);
	unsigned int* num_obj_ptr = malloc(sizeof(unsigned int));
	*num_obj_ptr = num_obj;
	//snprintf(num_obj_str,15*sizeof(char), "%u", num_obj);

	if(time_table_value==NULL){

		g_hash_table_insert(*time_table, object,  num_obj_ptr);
		*tree = insert(num_obj ,*tree);
		reuse_dist=0;

	}else{
		//timestamp = strtol(time_table_value,NULL,10);
		reuse_dist =(uint64_t) calc_distance( *time_table_value,*tree);
				//Busquemos la distancia de reuso en la hashtable distance_table
				//snprintf(reuse_dist_str, 15*sizeof(char), "%"PRIu64"", reuse_dist);
				//printf("%u \n", reuse_dist);
				//delete old timestamp from tree
		*tree = delete(*time_table_value ,*tree);
					
				//Insert new timestamp from tree
		*tree = insert(num_obj ,*tree);
		g_hash_table_insert(*time_table, object, num_obj_ptr);	
	}

	//printf("num_obj_ptr: %u \n", *num_obj_ptr);
	
	return reuse_dist;
}

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table){
	
	uint64_t *x = (uint64_t*) g_hash_table_lookup(*dist_table, &reuse_dist);
	
	if(x == NULL){
		//printf("11111\n");
		x = (uint64_t*)malloc(sizeof(uint64_t));
		*x = 1;
		uint64_t *dist = (uint64_t*)malloc(sizeof(uint64_t));
		*dist = reuse_dist;
		g_hash_table_insert(*dist_table, dist, x);
		//printf("hashtable value: %d\n", *(int*)g_hash_table_lookup(*dist_table, &reuse_dist));

	}else{
		*x= *x + 1;
		
	}
}

void update_dist_table_fixed_size(uint64_t  reuse_dist, GHashTable **dist_table, uint64_t T_new){
		
		uint64_t *x = (uint64_t*) g_hash_table_lookup(*dist_table, &reuse_dist);
		uint64_t T_old = 0;
		double tmp =0; 
		if(x == NULL){
			//printf("11111\n");
			x = (uint64_t*)malloc(2*sizeof(uint64_t));
			x[0] = 1;
			x[1] = T_new;
			//printf("x[0]: %"PRIu64"  x[1]: %"PRIu64"\n",x[0],x[1]);
			int *dist = (int*)malloc(sizeof(uint64_t));
			*dist = reuse_dist;
			g_hash_table_insert(*dist_table, dist, x);
			//printf("hashtable value: %d\n", *(int*)g_hash_table_lookup(*dist_table, &reuse_dist));

		}else{
			T_old = x[1];
			if(T_old!=T_new){
				tmp = ((double)T_new) /T_old;
				x[0] = (uint64_t) x[0]*tmp  + 1;
				x[1] = T_new;
			}

			*x= *x + 1;
			//printf("x[0]: %"PRIu64"  x[1]: %"PRIu64"\n",x[0],x[1]);
			
		}
}

GHashTable *MRC(GHashTable  **dist_table){

	GList *keys = g_hash_table_get_keys(*dist_table);
	GHashTable *tabla = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)free, (GDestroyNotify)free);
	keys = g_list_sort(keys, (GCompareFunc) intcmp );
	

	double *missrate = NULL;
	int *cache_size = NULL;
	unsigned int total_sum = *(int*)(g_hash_table_lookup(*dist_table, keys->data) );
	//printf("TOTAL SUM: %u \n", total_sum);
	unsigned int part_sum = 0;
	keys = keys->next;
	while(1){	
		cache_size = malloc(sizeof(int));			
		missrate = malloc(sizeof(double));
		part_sum = part_sum + *(int*)(g_hash_table_lookup(*dist_table, keys->data) );
		//printf("PART SUM: %u \n", part_sum);
		*missrate =  (double) part_sum;
		*cache_size = *(int*)(keys->data);
		//printf("%d %f\n", *cache_size, *missrate);
		g_hash_table_insert(tabla, cache_size, missrate);

		if(keys->next ==NULL){
			break;
		}
		keys= keys->next;		
	}
	keys = g_list_first(keys);
	total_sum = total_sum + part_sum;
	//printf("TOTAL SUM: %u \n", total_sum);
	keys= keys->next; //ignoring the zero (infinity) reuse dist
	missrate = NULL;
	while(1){	
		
		missrate = g_hash_table_lookup(tabla, keys->data);
		*missrate = 1.0 - (*missrate/total_sum);
		//printf("%d %f\n", *(int*)keys->data, *missrate);
		
		if(keys->next ==NULL){
			break;
		}
		keys= keys->next;		
	}	
	keys = g_list_first(keys);
	g_list_free (keys);

	return tabla;
}

GHashTable *MRC_fixed_size(GHashTable  **dist_table, uint64_t T_new){

		GList *keys = g_hash_table_get_keys(*dist_table);
		GHashTable *tabla = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)free, (GDestroyNotify)free);
		keys = g_list_sort(keys, (GCompareFunc) intcmp );
		
		double tmp = 0.0;
		double *missrate = NULL;
		int *cache_size = NULL;

		uint64_t *hist_value = (g_hash_table_lookup(*dist_table, keys->data) );	
		if(hist_value[1] != T_new){
			tmp = hist_value[0]*(( (double) T_new )/hist_value[1]);
		}
		uint64_t  total_sum = (uint64_t)tmp;
		//printf("TOTAL SUM: %u \n", total_sum);

		//printf("cache size: %d total_sum: %"PRIu64" T: %"PRIu64" T_new %"PRIu64" \n", *(int*)keys->data, total_sum, hist_value[1], T_new);
		uint64_t part_sum = 0;
		keys = keys->next;
		while(1){	
			cache_size = malloc(sizeof(int));			
			missrate = malloc(sizeof(double));

			hist_value = g_hash_table_lookup(*dist_table, keys->data);
			if(hist_value[1] != T_new){
				tmp = hist_value[0]*(( (double) T_new )/hist_value[1]) + 1 ;
				part_sum = part_sum + (uint64_t)tmp;
			}else{
				
				part_sum = part_sum + hist_value[0];
			}	
			
			
			*missrate =  (double) part_sum;
			*cache_size = *(int*)(keys->data);
			//printf("cache size: %d part_sum: %f  T: %"PRIu64" T_new %"PRIu64" \n", *cache_size, *missrate, hist_value[1], T_new);
			g_hash_table_insert(tabla, cache_size, missrate);

			if(keys->next ==NULL){
				break;
			}
			keys= keys->next;		
		}
		keys = g_list_first(keys);
		total_sum = total_sum + part_sum;
		//printf("TOTAL SUM: %u \n", total_sum);
		keys= keys->next; //ignoring the zero (infinity) reuse dist
		missrate = NULL;
		while(1){	
			
			missrate = g_hash_table_lookup(tabla, keys->data);
			*missrate = 1.0 - (*missrate/total_sum);
			//printf("%d %f\n", *(int*)keys->data, *missrate);
			
			if(keys->next ==NULL){
				break;
			}
			keys= keys->next;		
		}	
		keys = g_list_first(keys);
		g_list_free (keys);

		return tabla;
}

int intcmp(const void *x, const void *y){
		const int a = *(int*)x;
		const int b = *(int*)y;
		return (a < b) ? -1 : (a > b);
}

int doublecmp(const void *x, const void *y){
		const double a = *(double*)x;
		const double b = *(double*)y;
		return (a < b) ? -1 : (a > b);
}