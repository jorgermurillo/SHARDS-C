#include "SHARDS.h"

SHARDS* SHARDS_fixed_rate_init(double R_init, unsigned int bucket_size, object_Type type){

	//	Validation
	if(R_init<=0 || R_init > 1){
		printf("Value of R must be in the range (0,1].\n");
		return NULL;
	}



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
			shards->time_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int:
			shards->time_table = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int64:
			shards->time_table = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, ( GDestroyNotify )free);		
			break;
		case Double:
			shards->time_table = g_hash_table_new_full(g_double_hash, g_double_equal, NULL, ( GDestroyNotify )free);
			break;
		case Pointer:
			shards->time_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )free);
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


SHARDS* SHARDS_fixed_size_init(unsigned int max_setsize, unsigned int bucket_size, object_Type type){

	if(max_setsize<=0 ){
		printf("The maximum size of the working set must be greater then 0.\n");
		return NULL;
	}


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
			shards->time_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int:
			shards->time_table = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, ( GDestroyNotify )free);
			break;
		case Int64:
			shards->time_table = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, ( GDestroyNotify )free);		
			break;
		case Double:
			shards->time_table = g_hash_table_new_full(g_double_hash, g_double_equal, NULL, ( GDestroyNotify )free);
			break;
		case Pointer:
			shards->time_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )free);
			break;
	}

	shards->dist_histogram = g_hash_table_new_full(g_int_hash, g_int_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);

	shards->set_table = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )g_list_free);
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

SHARDS* SHARDS_fixed_size_init_R(unsigned int  max_setsize, double R_init, unsigned int bucket_size, object_Type type){

	if(R_init<=0 || R_init > 1){
		printf("Value of R must be in the range (0,1].\n");
		return NULL;
	}

	if(max_setsize<=0 ){
		printf("The maximum size of the working set must be greater then 0.\n");
		return NULL;
	}

	SHARDS *shards = SHARDS_fixed_size_init(max_setsize, bucket_size, type);
	shards->R = R_init;
	shards->T = R_init*(shards->T);

	return shards;

}

void SHARDS_feed_obj(SHARDS *shards, void* object, size_t nbytes){

		uint64_t  hash[2];
		uint64_t T_i = 0;
		unsigned int reuse_dist=0;
		unsigned int bucket = 0;
		unsigned int bucket_size = shards->bucket_size;
		unsigned int eviction_key = 0;

		qhashmurmur3_128(object ,nbytes ,hash );
		T_i = hash[1] &( (shards->P)-1 );

		if(shards->version==FIXED_SIZE){
			
			if(T_i < shards->T){

				//printf("########\nObject accepted!\n########\n");
			 	shards->num_obj++;

			 	//printf("num_obj: %u\n", num_obj);
			 			
				reuse_dist = calc_reuse_dist(object, shards->num_obj, &(shards->time_table), &(shards->dist_tree));
			 	reuse_dist = (unsigned int)(reuse_dist/shards->R);
			 	if(reuse_dist!=0){
					
					bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

				}else{
					bucket=0;

				}	
			 	//printf("Reuse distance: %5u\n", reuse_dist);
			 	//printf("Bucket: %u\n",bucket );
			 	update_dist_table_fixed_size(bucket, &(shards->dist_histogram), shards->T);
			 	
			 	//Insert <object, T_i> into Set S

			 	shards->set_tree = insert(T_i, shards->set_tree);
			 	//printf("%d %p\n", set_tree->key, set_tree );
			 	//Lookup the list associated with the value T_i
			 	shards->set_list = g_hash_table_lookup(shards->set_table, shards->set_tree);
			 	//If the search returns NULL (e.g the list doesnt exist), create a list and insert it
			 	if(shards->set_list==NULL){

			 		shards->set_list = g_list_append(shards->set_list, object);
			 		g_hash_table_insert(shards->set_table, shards->set_tree ,shards->set_list);
			 		shards->set_size++;
			 		
			 	}else{
			 		//If the search returns a list, search the object in the list	
			 		shards->set_list_search = g_list_find_custom(shards->set_list, object, (GCompareFunc)strcmp);
			 		//If the object is not on the list, add it
			 		if(shards->set_list_search==NULL){
			 			shards->set_list = g_list_append(shards->set_list, object);
			 			shards->set_list_search = NULL;
			 			shards->set_size+=1;
			 			
			 		}else{
			 			free(object);
			 		}

			 	}
			 	//printf("Set_size: %u\n\n", set_size);
			 	shards->set_list = NULL;

			 	if(shards->set_size > shards->S_max){
			 		//Eviction
			 		//printf("EVICTION!!\n\n ");
			 		
			 		

			 		shards->evic_tree = find_rank((shards->set_tree->size) -1, shards->set_tree);
			 		shards->evic_tree = splay(shards->evic_tree->key, shards->evic_tree);
			 		
			 		eviction_key = shards->evic_tree -> key;
			 		
			 		shards->set_list = g_hash_table_lookup(shards->set_table, shards->evic_tree);
			 		

			 		while(1){
	    				
			 			shards->dist_tree = delete( *(unsigned int *)(g_hash_table_lookup( shards->time_table, (char*)shards->set_list->data ) ), shards->dist_tree );
			 			
			 			g_hash_table_remove(shards->time_table, (char*)shards->set_list->data );
			 			shards->set_size-=1;
			 			shards->evic_obj+=1;
			 			free(shards->set_list->data);
	    				if(shards->set_list->next == NULL){
	    			
	    					break;
	    				}
	    				shards->set_list = shards->set_list->next;
	    			}	
	    			//remove and free value from set_table
	    			g_hash_table_remove(shards->set_table, shards->evic_tree );



					//remove and free the eviction_key from set_tree, whose node is also the key for set_table
	    			shards->set_tree = delete(eviction_key, shards->set_tree);
	    			shards->evic_tree =NULL;

	    			shards->T = eviction_key;
	    			shards->R = ((double)shards->T)/shards->P;

	    			//printf("NEW  R: %f  T:  %"PRIu64"\n", R,T);		
	    		}
		 	
			}else{
			 	free(object);
			}

		}else{
			//FIXED RATE CODE GOES HERE

			if(T_i < shards->T){
				shards->num_obj++;

				//printf("num_obj: %u\n", num_obj);
							
				reuse_dist = calc_reuse_dist(object, shards->num_obj, &(shards->time_table), &(shards->dist_tree));
				reuse_dist = (unsigned int)(reuse_dist/shards->R);
				if(reuse_dist!=0){
						
					bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

				}else{
					bucket=0;
				}	
				//printf("Reuse distance: %5u\n", reuse_dist);
				//printf("Bucket: %u\n",bucket );
				update_dist_table(bucket, &(shards->dist_histogram));



			}else{
				free(object);
			}

		}
}

void SHARDS_free(SHARDS* shards){

		if(shards->version==FIXED_SIZE){

			g_hash_table_destroy(shards->dist_histogram);
			GList *keys = g_hash_table_get_keys(shards->time_table);
			g_list_free_full(keys, (GDestroyNotify)free);

			g_hash_table_destroy(shards->time_table);

			g_hash_table_destroy(shards->set_table);
			freetree(shards->dist_tree);
			freetree(shards->set_tree);

			free(shards);
		
		}else{

			g_hash_table_destroy(shards->dist_histogram);
			GList *keys = g_hash_table_get_keys(shards->time_table);
			g_list_free_full(keys, (GDestroyNotify)free);

			g_hash_table_destroy(shards->time_table);
			freetree(shards->dist_tree);
			free(shards);
		}





}

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

GHashTable *MRC(SHARDS *shards){

	GList *keys = g_hash_table_get_keys(shards->dist_histogram);
	GHashTable *tabla = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)free, (GDestroyNotify)free);
	keys = g_list_sort(keys, (GCompareFunc) intcmp );
	

	double *missrate = NULL;
	int *cache_size = NULL;
	unsigned int part_sum = 0;
	unsigned int total_sum = *(int*)(g_hash_table_lookup(shards->dist_histogram, keys->data) );
	//printf("TOTAL SUM: %u \n", total_sum);

	keys = keys->next;
	while(1){	
		cache_size = malloc(sizeof(int));			
		missrate = malloc(sizeof(double));
		part_sum = part_sum + *(int*)(g_hash_table_lookup(shards->dist_histogram, keys->data) );
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

GHashTable *MRC_fixed_size(SHARDS *shards){

		GList *keys = g_hash_table_get_keys(shards->dist_histogram);
		GHashTable *tabla = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)free, (GDestroyNotify)free);
		keys = g_list_sort(keys, (GCompareFunc) intcmp );
		uint64_t T_new = shards->T;
		double tmp = 0.0;

		double *missrate = NULL;
		int *cache_size = NULL;
		uint64_t  total_sum = 0;
		uint64_t part_sum = 0;

		uint64_t *hist_value = (g_hash_table_lookup(shards->dist_histogram, keys->data) );	
		
		unsigned int hist_size = g_hash_table_size (shards->dist_histogram);

		if(hist_value[1] != T_new){
			tmp = hist_value[0]*(( (double) T_new )/hist_value[1]) +1 ;
			total_sum = (uint64_t)tmp;
		}else{
			total_sum = *hist_value;
		}
		//printf("TOTAL SUM: %u \n", total_sum);

		//printf("cache size: %d total_sum: %"PRIu64" T: %"PRIu64" T_new %"PRIu64" \n", *(int*)keys->data, total_sum, hist_value[1], T_new);
		if(hist_size>1){
			keys = keys->next;
			while( 1 ){	
				cache_size = malloc(sizeof(int));			
				missrate = malloc(sizeof(double));

				hist_value = g_hash_table_lookup(shards->dist_histogram, keys->data);
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
			missrate = NULL; //We are gonna use miss_rate again as a temp variable
			while(1){	
				
				missrate = g_hash_table_lookup(tabla, keys->data);
				*missrate = 1.0 - (*missrate/total_sum);
				//printf("%d %f\n", *(int*)keys->data, *missrate);
				
				if(keys->next ==NULL){
					break;
				}
				keys= keys->next;		
			}
		}else{

			//	if hist_size == 1	
			printf("WHATSUP\n");
			cache_size = malloc(sizeof(int));			
			missrate = malloc(sizeof(double));
			*cache_size = *(int*)(keys->data);
			*missrate = 1.0;
			g_hash_table_insert(tabla, cache_size, missrate);
			printf("GETOUT\n");
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