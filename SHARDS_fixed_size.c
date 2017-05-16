#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>
#include "murmurhash3.h"
#include "splay.h"
#include "shards_utils.h"

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
	double R = 0.1;
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

	unsigned int evic_obj = 0;

	unsigned int bucket =0;
	unsigned int bucket_size = strtol(argv[5], NULL,10);

	int total_objects=0;
	int num_obj=0;
	double fraction=0;

	unsigned int reuse_dist=0;	

	clock_t time_begin, time_end;
	double time_total; 
	// eviction_rank is used to find the largest T_i in set_tree
	unsigned int eviction_key = 0;
	Tree *evic_tree = NULL;

	
	time_begin = clock();

	int pp = 0;
	while(fgets(object, obj_length+2, file)!=NULL){
			 	
			 object[obj_length]='\0';
			 //printf("Object: %s\n", object);
			 total_objects++;
			 //printf("%d\n", total_objects);
			 qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
			 
			 //printf("Hash: %"PRIu64"\n", hash[1]);
			
			 T_i = hash[1] &(P-1);
			 //printf("T_i: %"PRIu64" T: %"PRIu64"\n", T_i, T);
			 
			if(T_i < T){

				//printf("########\nObject accepted!\n########\n");
			 	num_obj++;

			 	//printf("num_obj: %u\n", num_obj);
			 			
				reuse_dist = calc_reuse_dist(object, num_obj, &time_table, &tree);
			 	reuse_dist = (unsigned int)(reuse_dist/R);
			 	if(reuse_dist!=0){
					
					bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

				}else{
					pp++;
					bucket=0;
				}	
			 	//printf("Reuse distance: %5u\n", reuse_dist);
			 	//printf("Bucket: %u\n",bucket );
			 	update_dist_table_fixed_size(bucket, &dist_table, T);
			 	



			 	//Insert <object, T_i> into Set S

			 	set_tree = insert(T_i, set_tree);
			 	//printf("%d %p\n", set_tree->key, set_tree );
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
			 	//printf("Set_size: %u\n\n", set_size);
			 	set_list =NULL;

			 	if(set_size > S_max){
			 		//Eviction
			 		//printf("EVICTION!!\n\n ");
			 		
			 		

			 		evic_tree = find_rank((set_tree->size) -1, set_tree);
			 		evic_tree = splay(evic_tree->key, evic_tree);
			 		
			 		eviction_key = evic_tree -> key;
			 		
			 		set_list = g_hash_table_lookup(set_table, evic_tree);
			 		

			 		while(1){
	    				
			 			tree = delete( *(unsigned int *)(g_hash_table_lookup( time_table, (char*)set_list->data ) ), tree );
			 			
			 			g_hash_table_remove(time_table, (char*)set_list->data );
			 			set_size--;
			 			evic_obj++;
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

	    			//printf("NEW  R: %f  T:  %"PRIu64"\n", R,T);		
	    		}
		 	
			}else{
			 	free(object);
			}
			 
			object = (char*)calloc((obj_length+2), sizeof(char));
		}
	
		//printf("Tamaño de dist_table: %d \n", g_hash_table_size(dist_table));

	
		GList *keys = NULL; 
		
		/*
		
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
		*/
		
		//Checking if the expected number of references equals the actual number of references
		
		int expected_sampled_refs = total_objects*R;
		printf("expected ref: %d\n set: %d\n", expected_sampled_refs , set_size);
		/*
		if( expected_sampled_refs != set_size ){
			
			int * y = malloc(sizeof(int));
			*y = 0;



			uint64_t *v = g_hash_table_lookup(dist_table, y);
			//printf("Anterior: %"PRIu64"\n", *v);
			printf("YES: %"PRIu64"\n", *v);
			printf("%d\n", ((int)expected_sampled_refs - (int)set_size));
			*v = *v + ( (int)expected_sampled_refs - (int)set_size );
			//printf("Posterior: %"PRIu64"\n", *v);
			printf("Now: %"PRIu64"\n", *v);
			free(y);
		}
		*/




		keys = g_list_first(keys);
		g_list_free(keys);

		keys = g_hash_table_get_keys(dist_table);
		keys = g_list_sort(keys, (GCompareFunc) intcmp );
		char debug_name[256];
		snprintf(debug_name , sizeof(debug_name),"%s%s" , argv[4], "_debug");
		FILE *file_debug = fopen(debug_name, "w");
		uint64_t *tmp_db; 
		while(1){
			tmp_db = g_hash_table_lookup(dist_table, keys->data);
			fprintf(file_debug, "%u %"PRIu64" %"PRIu64"\n",*(unsigned int*)keys->data, tmp_db[0], tmp_db[1]);
			if(keys->next == NULL){
				break;
			}
			keys = keys->next;
		}	
		keys = g_list_first(keys);
		g_list_free(keys);

		GHashTable *miss_rates = MRC_fixed_size(&dist_table, T);

		//GHashTable *miss_rates = MRC(&dist_table);

		time_end = clock();
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
			fprintf(file2, "Accepted References: %d\n", num_obj );
			fraction = 100* ( num_obj/((double)total_objects));
			fprintf(file2, "Percentage of accepted references: %3.2f %%\n", fraction);
			fprintf(file2, "Expected sampled references: %u\n", expected_sampled_refs);
			
			
		}

		while(1){	
			//fprintf(file2, "%d %d\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ) );
			fprintf(file2, "%d %f  \n", *(int*)(keys->data), *(double*)( g_hash_table_lookup(miss_rates, (int*)keys->data) ) );	
			//printf( "%d %f  \n", *(int*)(keys->data), *(double*)( g_hash_table_lookup(miss_rates, (int*)keys->data) ));
			//printf( "%d %d  p1: %p  p2: %p\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ),(keys->data), ( g_hash_table_lookup(dist_table, keys->data) ) );
			if(keys->next ==NULL){
				break;
			}
			keys= keys->next;		
		}


		
		
		time_total= ((double)(time_end - time_begin))/CLOCKS_PER_SEC;
		double throughput = total_objects/ time_total;


		//printf("Numero de objetos unicos: %d \n", g_hash_table_size(time_table) );

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
		

		//PARA EL SGTE BLOQUE DE CODIGO ESCRIBIR PREPROCESADOR QUE PERMITA ESCOGER ENTRE IMPRIMIR O USAR LA FUNCION 
		// g_list_free_full()

		// 

		//printf("Printing the set of objects.\n");
		GHashTableIter iter;
		gpointer key, value;
		GList *plist=NULL;
		g_hash_table_iter_init (&iter, set_table);
		unsigned int set_cnt =0;
		while (g_hash_table_iter_next (&iter, &key, &value)){
	    	plist = (GList*) value;
	    	//printf("%d\n", ((Tree*)key)->key );
	    	while(1){
	    		//printf("object: %s\n", (char*)plist->data);
	    		set_cnt +=1;
	    		free(plist->data);
	    		if(plist->next == NULL){
	    			//printf("\n");
	    			break;
	    		}
	    		
	    		
	    		plist = plist->next;
	    	}
	  	}

	  	

	  	printf("PP: %d\n", pp);
	  	

		printf("Trace file: %s\n", argv[3] );
		printf("Set size: %u\n", set_size);
		printf("T: %1.2"PRIu64"\n", T );
		printf("P: %"PRIu64"\n", P );
		printf("R: %f\n", R );
		printf("Total References: %d\n", total_objects );	
		printf("Accepted References: %d\n", num_obj );
		fraction = 100* ( num_obj/((double)total_objects));
		printf("Percentage of accepted references: %3.2f %%\n", fraction);
		printf("Expected sampled references: %u\n", expected_sampled_refs);
		
		printf("Objetos en el set: %u \n", set_cnt);
		printf("Time: %.3f seconds\n", time_total);
		printf("Throughput: %.3f objects per second \n", throughput);
		printf("Objects evicted: %u\n", evic_obj);
		g_hash_table_destroy(time_table);

		g_hash_table_destroy(dist_table);
		g_hash_table_destroy(miss_rates);
		freetree(set_tree);
		g_hash_table_destroy(set_table);

		return 0;
}
