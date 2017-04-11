#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>
#include "murmurhash3.h"
#include "splay.h"

uint64_t calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(uint64_t  reuse_dist ,GHashTable **dist_table);

GHashTable *MRC(GHashTable  **tabla);

int intcmp(const void *x, const void *y);

int doublecmp(const void *x, const void *y);

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
	char* object = (char*)malloc((obj_length+2)*sizeof(char));

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

	uint64_t reuse_dist=0;	

	clock_t time_begin, time_end;
	double time_total; 
	
	while(fgets(object, obj_length+2, file)!=NULL){
		 object[obj_length]='\0';
		 printf("Object: %s\n", object);
		 total_objects++;
		 qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
		 
		 printf("Hash: %"PRIu64"\n", hash[1]);

		 T_i = hash[1] &(P-1);
		 printf("T_i: %"PRIu64" T: %"PRIu64"\n", T_i, T);
		  
		 if(T_i<T){
		 	printf("########\nObject accepted!\n########\n");
		 	num_obj++;

		 	printf("num_obj: %u\n", num_obj);
		 	//printf("AAAAAAAAAA\n");
		 	reuse_dist = calc_reuse_dist(object, num_obj, &time_table, &tree);
		 	reuse_dist = (uint64_t)(reuse_dist/R);

		 	if(reuse_dist!=0){
				
				bucket = ((reuse_dist-1)/bucket_size)*bucket_size + bucket_size;

			}else{
				bucket=0;
			}	

		 	printf("Reuse distance: %5"PRIu64"\n", reuse_dist);
		 	printf("Bucket: %u\n",bucket );
		 	update_dist_table(bucket, &dist_table);
		 	printf("\n\n");



		 	
		 }else{
		 	free(object);
		 }
		 
		 object = (char*)malloc((obj_length+2)*sizeof(char));
	}
	time_begin = clock();
	GList *keys = g_hash_table_get_keys (dist_table);
	printf("Tamaño de dist_table: %d \n", g_hash_table_size(dist_table));
	keys = g_list_sort (keys ,(GCompareFunc) intcmp );
	//printf("Data: %d %p\n", *(int*)(keys->data), (keys->data));
	time_end = clock();
	
	time_total= ((double)(time_end - time_begin))/CLOCKS_PER_SEC;
	/*
	while(1){	
		//fprintf(file2, "%d %d\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ) );
			
		printf( "%d %d  \n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, (int*)keys->data) ));
		//printf( "%d %d  p1: %p  p2: %p\n", *(int*)(keys->data), *(int*)( g_hash_table_lookup(dist_table, keys->data) ),(keys->data), ( g_hash_table_lookup(dist_table, keys->data) ) );
		if(keys->next ==NULL){
			break;
		}
		keys= keys->next;		
	}
	*/
	printf("\n");

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
	g_hash_table_destroy(time_table);
	g_hash_table_destroy(dist_table);
	g_hash_table_destroy(miss_rates);

	freetree(tree);
	keys = g_list_first(keys);
	g_list_free(keys);
	printf("time: %f\n",time_total);
	return 0;
}
uint64_t calc_reuse_dist(char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree){

	uint64_t reuse_dist=0;

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