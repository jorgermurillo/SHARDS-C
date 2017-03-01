#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "splay.h"
#include "murmurhash3.h"

uint64_t calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(int  reuse_dist ,GHashTable **distance_table);

GHashTable* MRC( GHashTable **distance_table);

int numerical_strcmp(const char* a, const char* b);

void print_MRC(GHashTable **MRC, char *nombre_archivo, char ch);

/*
*
*
*
*/

int main(int argc, char *argv[]){
   	/* Arguments
		argv[1] = length of each object in the trace (number of characters)
		argv[2] = Value R which gives the threshhold value T = R*P for the comparison T_i<T
		argv[3] = Trace file
		argv[4] = file where the MRC is going to be written
		argV[5] = If it exists (argc=5) and is equal to 1, SHARDS will print more pertinent data before the MRC
	*/
	GList *histograma;
	
	Tree *tree =NULL;
	GHashTable *time_table = g_hash_table_new_full(g_str_hash, g_str_equal,( GDestroyNotify )free, ( GDestroyNotify )free);
	GHashTable *distance_table = g_hash_table_new_full(g_str_hash, g_str_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);
	GHashTable *mrc = g_hash_table_new_full(g_str_hash, g_str_equal, ( GDestroyNotify )free, ( GDestroyNotify )free);
	int obj_length = (int) strtol(argv[1], NULL, 10);
	
	double R = strtod(argv[2], NULL);
	
	uint64_t P = 1;
	P = P<<24;
	uint64_t T =  R*P;
	uint64_t  hash[2];
	uint64_t shifted_value =0 ;	
	uint64_t T_i =0;
	bool one;	

	uint64_t total_objects=0;
	uint64_t num_obj=0;
	clock_t time_begin, time_end;
	double time_total = 0;	
  	double throughput =0;
	char *object= malloc((obj_length+2)*sizeof(char));

	
	

	unsigned int reuse_dist=0;	
	
	
	FILE *file;
	file = fopen(argv[3], "r");
 
	while(fgets(object, obj_length+2, file)!=NULL){
		time_begin = clock();
		object[obj_length]='\0';	
		total_objects++;
		one = qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
		
		//shifted_value =  (buffer[1] >> (8*n)) & 0xff;

		shifted_value = hash[1];
		
		//The following line is equivalent to T_i = shifted_value % P; because P is a power of 2
		T_i = shifted_value & (P-1);
		
		//sprintf("Objeto #%7u: %12s \n", num_obj, object);
	
		//Calculate Reuse distance

		if(T_i < T){
			num_obj++;
			reuse_dist = calc_reuse_dist( object,  num_obj, &time_table, &tree);
			reuse_dist = (uint64_t)(reuse_dist/R);
			update_dist_table(reuse_dist , &distance_table);	

			printf("%u \n", reuse_dist);
				
		}
		
			
		//tmp_str = (char*) malloc(15*sizeof(char));
		
		object= (char*) malloc((obj_length+2)*sizeof(char));
		// num_obj++ solo debe pasar si se cumple la condicion hash(Li)% P <= T !!!!!!! 
		
	}

	fclose(file);
	printf("\n\n");	
	printtree(tree, 2);
	freetree(tree);
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
	double fraction=0;
	printf("%s \n\n\n", (char*)histograma->data);
	mrc = MRC(&distance_table);	
	if (argc==6 ){
		FILE *file2 = fopen(argv[4],"w");
		fprintf(file2, "T: %"PRIu64"\n", T );
		fprintf(file2, "P: %"PRIu64"\n", P );
		fprintf(file2, "R: %f\n", R );
		fprintf(file2, "Total References: %"PRIu64"\n", total_objects );	
		fprintf(file2, "Accepted References: %"PRIu64"\n", num_obj );	
		fraction = 100* ( num_obj/((double)total_objects));
		fprintf(file2, "Percentage of accepted references: %3.2f\n", fraction);
		fclose(file2);	
		print_MRC(&mrc, argv[4],'a');
	}else{
		print_MRC(&mrc, argv[4],'w');
	}
	
	histograma= g_list_first(histograma);
	g_list_free(histograma);
	g_hash_table_destroy(time_table);
	g_hash_table_destroy(mrc);
	g_hash_table_destroy(distance_table);

	time_end = clock();
	time_total = ((double)(time_end - time_begin))/CLOCKS_PER_SEC;
	//time_total = time_total/1000000;
	throughput =  num_obj/time_total;
	printf("Throughput: %f obj/microsec\n", throughput);
	return 1;
}



uint64_t calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table,  Tree **tree){
	uint64_t timestamp=0;
	uint64_t reuse_dist=0;
	char* num_obj_str = (char*) malloc(15*sizeof(char)); 
	snprintf(num_obj_str, 15*sizeof(char), "%u", num_obj);
	//char *reuse_dist_str = (char*) malloc(15*sizeof(char));
	char *time_table_value ;

	time_table_value = (char*)g_hash_table_lookup(*time_table, object);
	
	if(time_table_value==NULL){
				
				//AQUI ES DONDE EL SET DE <Li, Ti> puede pasar el maximo de objetos que esta permitido

				//printf("if\n");
				g_hash_table_insert(*time_table, object,  num_obj_str);
				*tree = insert(strtol(num_obj_str,NULL,10) ,*tree);
				//En la tabla de distancias de reuso, la clave de 0 equivale a la clave 'infinito'
				reuse_dist = 0;
			
			
	}else{
				//printf("else\n");
				//Calculate reuse distance
				timestamp = strtol(time_table_value,NULL,10);
				reuse_dist =(uint64_t) calc_distance(timestamp,*tree);
				//Busquemos la distancia de reuso en la hashtable distance_table
				//snprintf(reuse_dist_str, 15*sizeof(char), "%"PRIu64"", reuse_dist);
			
			
			

				//printf("%u \n", reuse_dist);
				//delete old timestamp from tree
				*tree = delete(strtol(time_table_value,NULL,10) ,*tree);
					
				//Insert new timestamp from tree
				*tree = insert(strtol(num_obj_str,NULL,10) ,*tree);
				g_hash_table_insert(*time_table, object, num_obj_str);	
				

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

GHashTable *MRC( GHashTable **distance_table){
	
	char *tmp_str = (char*) malloc(15*sizeof(char));
	
	GList *hist = g_hash_table_get_keys(*distance_table);
	GHashTable *MRC = g_hash_table_new(g_str_hash, g_str_equal);
	
	
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
	
	tmp = NULL;
	
	// ESTE BLOQUE ES UNA PRUEBA
	hist = hist->next;
	while( 1 ){
		
		tmp = (miss_rates->data);
		*tmp = 1.0 - (*tmp/total_sum);
		
		
		snprintf(tmp_str,15*sizeof(char),"%f",*tmp);
				
		
		g_hash_table_insert(MRC, hist->data, tmp_str);			
		
		hist=hist->next;
		miss_rates = miss_rates->next;
		if(miss_rates==NULL){
			break;
		}
		tmp_str = (char*) malloc(15*sizeof(char));
	}
	miss_rates = g_list_first(miss_rates);
	g_list_free_full(  miss_rates, (GDestroyNotify) free);

	hist = g_list_first(hist);
	g_list_free_full( hist, (GDestroyNotify) free);
		
	
	return MRC;
}	

void print_MRC(GHashTable **MRC, char *nombre_archivo, char ch){
	char *c;
	GList *hist = g_hash_table_get_keys(*MRC);
	hist = g_list_sort (hist, (GCompareFunc)numerical_strcmp);
	if(ch=='a'){		
		c = "a";
	}else {
		c="w";
	}
	FILE *file = fopen(nombre_archivo, c);
	char* tmp ;		
	
	while( 1 ){
		
		tmp = g_hash_table_lookup(*MRC, hist->data);
		
		fprintf(file,"%s %s\n", (char*)hist->data , tmp );
		
		
		hist=hist->next;
		
		if(hist==NULL){
			break;
		}
		
	}
	fclose(file);	
	hist = g_list_first(hist);
	g_list_free_full(hist, (GDestroyNotify) free);


}

int numerical_strcmp(const char* a, const char* b) {
	
	int x = strlen(a); 
	int y = strlen(b);	
			
	return (x != y) ? ((x > y) ? 1 : -1) : strcmp(a, b);
}

