#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "splay.h"

unsigned int calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);
						
void update_dist_table(uint64_t  reuse_dist, GHashTable **dist_table);

GHashTable *MRC(GHashTable  **dist_table);

int intcmp(const void *x, const void *y);


/*
*
*
*
*/

int main(int argc, char *argv[]){
   	
	GList *keys;
	
	Tree *tree =NULL;
	GHashTable *time_table = g_hash_table_new_full(g_str_hash, g_str_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);
	GHashTable *dist_table = g_hash_table_new_full(g_int_hash, g_int_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);
	
	int obj_length = (int) strtol(argv[1], NULL, 10);
	//timestamp es el valor q se lee de la hashtable	
	
	
	
	unsigned int num_obj=1;
	
	
	char *object= malloc((obj_length+2)*sizeof(char));

	//char *time_table_value = (char*) malloc(15*sizeof(char));
	
	unsigned int reuse_dist=0;	
	
	
	FILE *file;
	file = fopen(argv[2], "r");
 
	while(fgets(object, obj_length+2, file)!=NULL){
		object[11]='\0';	
		
		
		//sprintf("Objeto #%7u: %12s \n", num_obj, object);
	
		//Calculate Reuse distance
		reuse_dist = calc_reuse_dist( object,  num_obj, &time_table, &tree);

		update_dist_table(reuse_dist , &dist_table);	

		//printf("%u \n", reuse_dist);
		
		
			
		//tmp_str = (char*) malloc(15*sizeof(char));
		
		object= (char*) malloc((obj_length+2)*sizeof(char));
		// num_obj++ solo debe pasar si se cumple la condicion hash(Li)% P <= T !!!!!!! 
		num_obj++;	
	}
	printf("\n\n");	
	//sprinttree(tree, 2);

	printf("\n\n");	
	/*
	GHashTableIter iter;
	void *key, *value;
	
	g_hash_table_iter_init (&iter, time_table);
	while (g_hash_table_iter_next (&iter, &key, &value))
 	{
    		printf("Objeto: %7s; Ultimo timestamp: %5s \n", (char*)key, (char*)value);
  	}	

	printf("\n\n");
	
	histograma = g_hash_table_get_keys(distance_table);
	
	histograma =g_list_sort (histograma, (GCompareFunc)numerical_strcmp);
	GList *iterador = histograma;
	
	//Imprimir el histograma de distancias de reuso
		
	while( 1 ){
		
		printf("Distancia de reuso: %s ; Cantidad: %s \n", (char*)iterador->data, (char*)g_hash_table_lookup(distance_table, iterador->data));
		iterador = iterador->next;
		if(iterador==NULL){
			break;
		}
	}	
	
	printf("%s \n\n\n", (char*)histograma->data);
	*/
	FILE *file2;
	file2 = fopen(argv[3], "w");

	GHashTable *miss_rates = MRC(&dist_table);
	keys = g_hash_table_get_keys (miss_rates);
	keys = g_list_sort (keys ,(GCompareFunc) intcmp );

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
	fclose(file);
	fclose(file2);
	freetree(tree);
	keys = g_list_first(keys);
	g_list_free(keys);
	g_hash_table_destroy(time_table);
	g_hash_table_destroy(dist_table);
	g_hash_table_destroy(miss_rates);

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

	//printf("num_obj_ptr: %u \n", *num_obj_ptr);
	
	return reuse_dist;
}

void update_dist_table(uint64_t  reuse_dist, GHashTable **dist_table){
		
		uint64_t *x = (uint64_t*) g_hash_table_lookup(*dist_table, &reuse_dist);
		
		if(x == NULL){
			//printf("11111\n");
			x = (uint64_t*)malloc(sizeof(uint64_t));
			*x = 1;
			
			//printf("x[0]: %"PRIu64"  x[1]: %"PRIu64"\n",x[0],x[1]);
			int *dist = (int*)malloc(sizeof(uint64_t));
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