#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>
#include "murmurhash3.h"
#include "splay.h"

unsigned int calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table, uint64_t T_new);

GHashTable *MRC(GHashTable  **dist_table, uint64_t T_new);

int intcmp(const void *x, const void *y);

int doublecmp(const void *x, const void *y);

void printset(GHashTable *t);

int main (int argc, char *argv[]){
	/* Arguments
		argv[1] = length of each object in the trace (number of characters)
		argv[2] = Value S_max corresponding to the max amount of objects in the set S.
		argv[3] = Trace file
		argv[4] = file where the MRC is going to be written
		argv[5] = Bucket size
		argv[6] = If it exists (argc=7) SHARDS will print more pertinent data before the MRC
	*/

	FILE *file;
	file = fopen(argv[3], "r");
	FILE *file2 =NULL;

	int obj_length = strtol(argv[1],NULL,10);
	char* object = (char*)calloc((obj_length+2),sizeof(char));

	uint64_t  hash[2];
	double R = 1.0;
	uint64_t P=1;
	P = P<<24;
	uint64_t T = R*P;

	uint64_t T_i=0; 
	
	Tree *tree=NULL;
	GHashTable *time_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
	GHashTable *dist_table = g_hash_table_new_full(g_int_hash, g_int_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);

	GHashTable *set_table =  g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, ( GDestroyNotify )g_list_free);	
	Tree *set_tree = NULL;
	GList *set_list = NULL;
	GList *set_list_search = NULL;
	const unsigned int S_max= (uint64_t) strtol(argv[2], NULL, 10);
	unsigned int set_size =0;



	unsigned int bucket =0;
	unsigned int bucket_size = strtol(argv[5], NULL,10);

	int total_objects=0;
	unsigned int num_obj=0;
	double fraction=0;

	unsigned int reuse_dist=0;	

	clock_t time_begin, time_end;
	double time_total; 
	// eviction_rank is used to find the largest T_i in set_tree
	unsigned int eviction_key = 0;
	Tree *evic_tree = NULL;

	

	while(fgets(object, obj_length+2, file)!=NULL){
			 	
			 object[obj_length]='\0';
			 printf("Object: %s\n", object);
			 total_objects++;
			 printf("%d\n", total_objects);
			 qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
			 
			 printf("Hash: %"PRIu64"\n", hash[1]);
			
			 T_i = hash[1] &(P-1);
			 printf("T_i: %"PRIu64" T: %"PRIu64"\n", T_i, T);
			 
			if(T_i < T){

				printf("########\nObject accepted!\n########\n");
			 	num_obj++;

			 	printf("num_obj: %u\n", num_obj);
			 			
				reuse_dist = calc_reuse_dist(object, num_obj, &time_table, &tree);
			 	reuse_dist = (unsigned int)(reuse_dist/R);
			 	if(reuse_dist!=0){
					
					bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

				}else{
					bucket=0;
				}	
			 	printf("Reuse distance: %5u\n", reuse_dist);
			 	printf("Bucket: %u\n",bucket );
			 	update_dist_table(bucket, &dist_table, T);
			 	



			 	//Insert <object, T_i> into Set S

			 	set_tree = insert(T_i, set_tree);
			 	printf("%d %p\n", set_tree->key, set_tree );
			 	//Lookup the list associated with the value T_i
			 	set_list = g_hash_table_lookup(set_table, set_tree);
			 	//If the search returns NULL (e.g the list doesnt exist), create a list and insert it
			 	if(set_list==NULL){

			 		set_list = g_list_append(set_list, object);
			 		g_hash_table_insert(set_table, set_tree ,set_list);
			 		set_size++;
			 		
			 	}else{
			 		//If the search returns a list, search the object in the list	
			 		set_list_search = g_list_find_custom(set_list, object, (GCompareFunc)strcmp);
			 		//If the object is not on the list, add it
			 		if(set_list_search==NULL){
			 			set_list = g_list_append(set_list, object);
			 			set_list_search = NULL;
			 			set_size++;
			 			
			 		}else{
			 			free(object);
			 		}

			 	}
			 	printf("Set_size: %u\n\n", set_size);
			 	set_list =NULL;

			 	if(set_size > S_max){
			 		//Eviction
			 		printf("EVICTION!!\n\n ");
			 		
			 	

			 		evic_tree = find_rank((set_tree->size) -1, set_tree);
			 		evic_tree = splay(evic_tree->key, evic_tree);
			 		
			 		eviction_key = evic_tree -> key;
			 		
			 		set_list = g_hash_table_lookup(set_table, evic_tree);
			 		

			 		while(1){
	    				
			 			tree = delete( *(unsigned int *)(g_hash_table_lookup( time_table, (char*)set_list->data ) ), tree );
			 			
			 			g_hash_table_remove(time_table, (char*)set_list->data );
			 			set_size--;
			 			free(set_list->data);
	    				if(set_list->next == NULL){
	    			
	    					break;
	    				}
	    				set_list = set_list->next;
	    			}	
	    			//remove and free value from set_table
	    			g_hash_table_remove(set_table, evic_tree );



					//remove and free the eviction_key from set_tree, whose node is also the key for set_table
	    			set_tree = delete(eviction_key, set_tree);
	    			evic_tree =NULL;

	    			T = eviction_key;
	    			R = ((double)T)/P;

	    			printf("NEW  R: %f  T:  %"PRIu64"\n", R,T);			 	}
		 	
			}else{
			 	free(object);
			}
			 
			object = (char*)calloc((obj_length+2), sizeof(char));
		}
		time_begin = clock();
		printf("Tamaño de dist_table: %d \n", g_hash_table_size(dist_table));

		time_end = clock();
		
		time_total= ((double)(time_end - time_begin))/CLOCKS_PER_SEC;

		GList *keys = NULL; 
		
		keys = g_list_first(keys);
		
		g_list_free(keys);
		
		keys = g_hash_table_get_keys(dist_table);

		keys = g_list_sort(keys, (GCompareFunc) intcmp);
		printf("HISTOGRAM\n");

		uint64_t *tp; 
		while(1){
			tp = (uint64_t*)g_hash_table_lookup(dist_table, keys->data);
			printf("%d %"PRIu64" T: %"PRIu64"  T actual: %"PRIu64"\n", *(int*)keys->data, tp[0], tp[1], T );
			if(keys->next==NULL){
				break;
			}
			keys = keys->next;
		}


		//Checking if the expected number of references equals the actual number of references

		unsigned int expected_sampled_refs = total_objects*R;

		if( expected_sampled_refs != set_size ){
			
			int *y = malloc(sizeof(int));
			*y = 0;



			uint64_t *v = g_hash_table_lookup(dist_table, y);
			printf("Anterior: %"PRIu64"\n", *v);
			*v = *v + ( expected_sampled_refs - set_size );
			printf("Posterior: %"PRIu64"\n", *v);

			free(y);
		}


		keys = g_list_first(keys);
		g_list_free(keys);

		GHashTable *miss_rates = MRC(&dist_table, T);
		keys = g_hash_table_get_keys (miss_rates);
		keys = g_list_sort (keys ,(GCompareFunc) intcmp );

		file2 =  fopen(argv[4],"w");
		
		if (argc== 7 ){
			fprintf(file2, "Trace file: %s\n", argv[3] );
			fprintf(file2, "Set size: %u\n", set_size);
			fprintf(file2, "T: %1.2"PRIu64"\n", T );
			fprintf(file2, "P: %"PRIu64"\n", P );
			fprintf(file2, "R: %f\n", R );
			fprintf(file2, "Total References: %d\n", total_objects );	
			fprintf(file2, "Accepted References: %u\n", num_obj );
			fprintf(file2, "Unique Accepted References: %d\n", g_hash_table_size(time_table) );	
			fraction = 100* ( num_obj/((double)total_objects));
			fprintf(file2, "Percentage of accepted references: %3.2f %%\n", fraction);
			fprintf(file2, "Expected sampled references: %u\n", expected_sampled_refs);
			
			
		}

		while(1){	
			//fprintf(file2, "%d %d\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ) );
			fprintf(file2, "%d %f  \n", *(int*)(keys->data), *(double*)( g_hash_table_lookup(miss_rates, (int*)keys->data) ) );	
			printf( "%d %f  \n", *(int*)(keys->data), *(double*)( g_hash_table_lookup(miss_rates, (int*)keys->data) ));
			//printf( "%d %d  p1: %p  p2: %p\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ),(keys->data), ( g_hash_table_lookup(dist_table, keys->data) ) );
			if(keys->next ==NULL){
				break;
			}
			keys= keys->next;		
		}


		


		printf("Numero de objetos unicos: %d \n", g_hash_table_size(time_table) );

		free(object);
		fclose(file);
		fclose(file2);
		/*
		g_hash_table_remove_all(time_table);
		g_hash_table_unref (time_table);
		*/
		

		freetree(tree);
		keys = g_list_first(keys);
		g_list_free(keys);
		printf("time: %f\n",time_total);

		printf("Printing the set of objects.\n");
		GHashTableIter iter;
		gpointer key, value;
		GList *plist=NULL;
		g_hash_table_iter_init (&iter, set_table);
		unsigned int set_cnt =0;
		while (g_hash_table_iter_next (&iter, &key, &value)){
	    	plist = (GList*) value;
	    	printf("%d\n", ((Tree*)key)->key );
	    	while(1){
	    		printf("object: %s\n", (char*)plist->data);
	    		set_cnt +=1;
	    		free(plist->data);
	    		if(plist->next == NULL){
	    			printf("\n");
	    			break;
	    		}
	    		
	    		
	    		plist = plist->next;
	    	}
	  	}

	  	

	  	g_hash_table_iter_init (&iter, time_table);
	
		while (g_hash_table_iter_next (&iter, &key, &value)){
	    	
	    	//free(key);
	    	//free(value);
	    	
	  	}
	  	g_hash_table_destroy(time_table);

		g_hash_table_destroy(dist_table);
		g_hash_table_destroy(miss_rates);
		freetree(set_tree);
		g_hash_table_destroy(set_table);
		
		printf("Objetos en el set: %u\n", set_cnt);
		return 0;
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

	printf("num_obj_ptr: %u \n", *num_obj_ptr);
	
	return reuse_dist;
}

void update_dist_table(uint64_t  reuse_dist, GHashTable **dist_table, uint64_t T_new){
		
		uint64_t *x = (uint64_t*) g_hash_table_lookup(*dist_table, &reuse_dist);
		uint64_t T_old = 0;
		double tmp =0; 
		if(x == NULL){
			//printf("11111\n");
			x = (uint64_t*)malloc(2*sizeof(uint64_t));
			x[0] = 1;
			x[1] = T_new;
			printf("x[0]: %"PRIu64"  x[1]: %"PRIu64"\n",x[0],x[1]);
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
			printf("x[0]: %"PRIu64"  x[1]: %"PRIu64"\n",x[0],x[1]);
			
		}
	}

GHashTable *MRC(GHashTable  **dist_table, uint64_t T_new){

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

		printf("cache size: %d total_sum: %"PRIu64" T: %"PRIu64" T_new %"PRIu64" \n", *(int*)keys->data, total_sum, hist_value[1], T_new);
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
			printf("cache size: %d part_sum: %f  T: %"PRIu64" T_new %"PRIu64" \n", *cache_size, *missrate, hist_value[1], T_new);
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

void printset(GHashTable *t){

	GHashTableIter iter;
	gpointer key2, value;
	GList *plist=NULL;
	g_hash_table_iter_init (&iter, t);
	
	while (g_hash_table_iter_next (&iter, &key2, &value)){
	   	plist = (GList*) value;
	   	printf("T_i: %10d pointer: %p\n", ((Tree*)key2)->key, key2);
	   	while(1){
	   		printf("	object: %s\n", (char*)plist->data);
	   		if(plist->next == NULL){
	  			
	 			break;
	   		}
	    	
	    	
	    	plist = plist->next;
	   	}
	   	printf("\n");
	}



}