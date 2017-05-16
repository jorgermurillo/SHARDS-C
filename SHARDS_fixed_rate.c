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
		argv[2] = Value R which gives the threshhold value T = R*P for the comparison T_i<T
		argv[3] = Trace file
		argv[4] = file where the MRC is going to be written
		argv[5] = Bucket size
		argv[6] = If it exists (argc=7) SHARDS will print more pertinent data before the MRC
	*/
	FILE *file;
	file = fopen(argv[3], "r");
	FILE *file2 =NULL;
	
	int obj_length = strtol(argv[1],NULL,10);
	char* object = (char*)calloc((obj_length+2) , sizeof(char));

	uint64_t  hash[2];
	double R= strtod(argv[2],NULL);

	if(R<=0.0 || R>1){
		printf("The value of R has to lie between (0,1]\n");
		exit(1);
	}

	uint64_t P=1;
	P = P<<24;
	uint64_t T = R*P;
	uint64_t T_i=0; 

	Tree *tree=NULL;
	GHashTable *time_table = g_hash_table_new_full(g_str_hash, g_str_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);
	GHashTable *dist_table = g_hash_table_new_full(g_int_hash , g_int_equal , ( GDestroyNotify )free, ( GDestroyNotify )free);

	unsigned int bucket =0;
	unsigned int bucket_size = strtol(argv[5], NULL, 10);
	
	int total_objects=0;
	unsigned int num_obj=0;
	double fraction=0;

	unsigned int reuse_dist=0;	

	clock_t time_begin, time_end;
	double time_total; 
	
	int pp=0;

	while(fgets(object, obj_length+2, file)!=NULL){
		 object[obj_length]='\0';
		 //printf("Object: %s\n", object);
		 total_objects++;
		 qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
		 
		 //printf("Hash: %"PRIu64"\n", hash[1]);

		 T_i = hash[1] &(P-1);
		 //printf("T_i: %"PRIu64" T: %"PRIu64"\n", T_i, T);
		  
		 if(T_i<T){
		 	//printf("########\nObject accepted!\n########\n");
		 	num_obj++;

		 	//printf("num_obj: %u\n", num_obj);
		 	//printf("AAAAAAAAAA\n");
		 	reuse_dist = calc_reuse_dist(object, num_obj, &time_table, &tree);
		 	reuse_dist = (unsigned int)(reuse_dist/R);

		 	if(reuse_dist!=0){
				
				bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

			}else{
				pp++;
				bucket=0;
			}	

		 	//printf("Reuse distance: %5"PRIu64"\n", reuse_dist);
		 	//printf("Bucket: %u\n",bucket );
		 	update_dist_table(bucket, &dist_table);
		 	//printf("\n\n");



		 	
		 }else{
		 	free(object);
		 }
		 
		 object = (char*)calloc((obj_length+2) , sizeof(char));
	}
	time_begin = clock();
	GList *keys = g_hash_table_get_keys (dist_table);
	//printf("Tamaño de dist_table: %d \n", g_hash_table_size(dist_table));
	keys = g_list_sort (keys ,(GCompareFunc) intcmp );
	//printf("Data: %d %p\n", *(int*)(keys->data), (keys->data));
	time_end = clock();
	
	time_total= ((double)(time_end - time_begin))/CLOCKS_PER_SEC;
	
		
	char debug_name[256];
	snprintf(debug_name , sizeof(debug_name),"%s%s" , argv[4], "_debug");
	FILE *file_debug = fopen(debug_name, "w");
	uint64_t *tmp_db; 
	while(1){
		tmp_db = g_hash_table_lookup(dist_table, keys->data);
		fprintf(file_debug, "%u %"PRIu64" \n",*(unsigned int*)keys->data, tmp_db[0]);
		if(keys->next == NULL){
			break;
		}
		keys = keys->next;
	}	
	keys = g_list_first(keys);
	
	g_list_free(keys);

	GHashTable *miss_rates = MRC(&dist_table);
	keys = g_hash_table_get_keys (miss_rates);
	keys = g_list_sort (keys ,(GCompareFunc) intcmp );

	file2 =  fopen(argv[4],"w");
	
	if (argc== 7 ){
		
		fprintf(file2, "T: %"PRIu64"\n", T );
		fprintf(file2, "P: %"PRIu64"\n", P );
		fprintf(file2, "R: %f\n", R );
		fprintf(file2, "Total References: %d\n", total_objects );	
		fprintf(file2, "Accepted References: %u\n", num_obj );
		fprintf(file2, "Unique Accepted References: %d\n", g_hash_table_size(time_table) );	
		fraction = 100* ( num_obj/((double)total_objects));
		fprintf(file2, "Percentage of accepted references: %3.2f %%\n", fraction);
		
		
	}

	printf("PP: %d\n", pp);
	printf( "T: %"PRIu64"\n", T );
	printf("P: %"PRIu64"\n", P );
	printf( "R: %f\n", R );
	printf("Total References: %d\n", total_objects );	
	printf( "Accepted References: %u\n", num_obj );
	printf("Unique Accepted References: %d\n", g_hash_table_size(time_table) );	
	fraction = 100* ( num_obj/((double)total_objects));
	printf("Percentage of accepted references: %3.2f %%\n", fraction);


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


	


	//printf("Numero de objetos unicos: %d \n", g_hash_table_size(time_table) );

	free(object);
	fclose(file);
	fclose(file2);
	/*
	g_hash_table_remove_all(time_table);
	g_hash_table_unref (time_table);
	*/
	g_hash_table_destroy(time_table);
	g_hash_table_destroy(dist_table);
	g_hash_table_destroy(miss_rates); 

	freetree(tree);
	keys = g_list_first(keys);
	g_list_free(keys);
	printf("time: %f\n",time_total);
	return 0;
}

