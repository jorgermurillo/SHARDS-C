#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "splay.h"

unsigned int calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(int  reuse_dist ,GHashTable **distance_table);

void MRC(char *nombre_archivo, GHashTable **distance_table);

int numerical_strcmp(const char* a, const char* b);


/*
*
*
*
*/

int main(int argc, char *argv[]){
   	
	GList *histograma;
	
	Tree *tree =NULL;
	GHashTable *time_table = g_hash_table_new(g_str_hash, g_str_equal);
	GHashTable *distance_table = g_hash_table_new(g_str_hash, g_str_equal);
	
	int obj_length = (int) strtol(argv[1], NULL, 10);
	//timestamp es el valor q se lee de la hashtable	
	
	
	
	unsigned int num_obj=1;
	
	
	char *object= malloc((obj_length)*sizeof(char));

	//char *time_table_value = (char*) malloc(15*sizeof(char));
	char *dist_table_value = (char*) malloc(15*sizeof(char));

	unsigned int reuse_dist=0;	
	
	
	FILE *file;
	file = fopen(argv[2], "r");
 
	while(fgets(object, obj_length+2, file)!=NULL){
		object[11]='\0';	
		
		
		//sprintf("Objeto #%7u: %12s \n", num_obj, object);
	
		//Calculate Reuse distance
		reuse_dist = calc_reuse_dist( object,  num_obj, &time_table, &tree);

		update_dist_table(reuse_dist , &distance_table);	

		printf("%u \n", reuse_dist);
		
		
			
		//tmp_str = (char*) malloc(15*sizeof(char));
		
		object= (char*) malloc((obj_length)*sizeof(char));
		// num_obj++ solo debe pasar si se cumple la condicion hash(Li)% P <= T !!!!!!! 
		num_obj++;	
	}
	printf("\n\n");	
	printtree(tree, 2);

	printf("\n\n");	
	
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
	
	MRC(argv[3], &distance_table);	

	
	
	return 1;
	
}



unsigned int calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table,  Tree **tree){
	unsigned int timestamp=0;
	unsigned int reuse_dist=0;
	char* num_obj_str = (char*) malloc(15*sizeof(char)); 
	snprintf(num_obj_str, 15*sizeof(char), "%u", num_obj);
	char *reuse_dist_str = (char*) malloc(15*sizeof(char));
	char *time_table_value = (char*) malloc(15*sizeof(char));

	time_table_value = g_hash_table_lookup(*time_table, object);
	
	if(time_table_value==NULL){
				//printf("if\n");
				g_hash_table_insert(*time_table, object,  num_obj_str);
				*tree = insert(strtol(num_obj_str,NULL,10) ,*tree);
				//En la tabla de distancioas de reuso, la clave de 0 equivaler a la clave 'infinito'
				reuse_dist = 0;
			
			
	}else{
				//printf("else\n");
				//Calculate reuse distance
				timestamp = strtol(time_table_value,NULL,10);
				reuse_dist = calc_distance(timestamp,*tree);
				//Busquemos la distancia de reuso en la hashtable distance_table
				snprintf(reuse_dist_str, 15*sizeof(char), "%u", reuse_dist);
			
			
			

				//printf("%u \n", reuse_dist);
				//delete old timestamp from tree
				*tree = delete(strtol(time_table_value,NULL,10) ,*tree);
					
				//Insert new timestamp from tree
				*tree = insert(strtol(num_obj_str,NULL,10) ,*tree);
				g_hash_table_replace(*time_table, object, num_obj_str);	

	}
	
	return reuse_dist;

}

void update_dist_table(int  reuse_dist ,GHashTable **distance_table){

	char *reuse_dist_str = (char*) malloc(15*sizeof(char));
	int tmp =0;
	snprintf(reuse_dist_str,15*sizeof(char),"%u", reuse_dist);
	
	char *tmp_str = g_hash_table_lookup(*distance_table, reuse_dist_str); 
	if(tmp_str!=NULL){
		tmp = strtol(tmp_str,NULL,10);
		tmp++;
	}else{
		tmp =1;
	}	
	tmp_str = (char*) malloc(15*sizeof(char));
	snprintf(tmp_str,15*sizeof(char),"%u",tmp);
	
	g_hash_table_insert(*distance_table, reuse_dist_str,  tmp_str);
}

void MRC(char *nombre_archivo, GHashTable **distance_table){

	FILE *file = fopen(nombre_archivo, "w");	
	
	GList *hist = g_hash_table_get_keys(*distance_table);
	

	//hist = g_list_sort (hist, (GCompareFunc)g_ascii_strcasecmp);
	hist = g_list_sort (hist, (GCompareFunc)numerical_strcmp);
	GList *iter =hist;
	
	GList *miss_rates =NULL;

	unsigned long part_sum = 0;
	double *tmp;
	//Aqui le asigno a total_sum el valor de de la clave 0, que equivale a Hit[infinito], para al final sumarlo a partial_sum
	unsigned int total_sum = strtol(  g_hash_table_lookup(*distance_table,iter->data)   ,NULL,10);
	iter=iter->next;
	while( 1 ){
			
		
		part_sum = part_sum + strtol(  g_hash_table_lookup(*distance_table, iter->data )   ,NULL,10);
		tmp = (double*)malloc(sizeof(double));	
		*tmp = part_sum; 	
		//Storing the partial sums in the miss_rate GList momentarily		
		miss_rates = g_list_append(miss_rates,tmp);
		
		iter = iter->next;
		if(iter==NULL){
			break;
		}
	}
	total_sum = total_sum + part_sum;
	printf("Total sum: %u \n", total_sum);
	tmp = NULL;
	
	// ESTE BLOQUE ES UNA PRUEBA
	hist = hist->next;
	while( 1 ){
		
		tmp = (miss_rates->data);
		*tmp = 1.0 - (*tmp/total_sum);
		printf("Cache Size : %s  Miss Rates: %f \n", (char*)hist->data , (double)(*tmp) );
		fprintf(file,"%s %f\n", (char*)hist->data , (double)(*tmp) );
		hist=hist->next;
		miss_rates = miss_rates->next;
		if(miss_rates==NULL){
			break;
		}
	}

	
		
	fclose(file);	
}	

int numerical_strcmp(const char* a, const char* b) {
	
	int x = strlen(a); 
	int y = strlen(b);	
			
	return (x != y) ? ((x > y) ? 1 : -1) : strcmp(a, b);
}

