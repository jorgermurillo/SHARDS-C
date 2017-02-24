#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "splay.h"
#include "murmurhash3.h"

uint64_t calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table, Tree **tree);

void update_dist_table(int  reuse_dist ,GHashTable **distance_table);

GHashTable* MRC( GHashTable **distance_table);

int numerical_strcmp(const char* a, const char* b);

void print_MRC(GHashTable **MRC, char *nombre_archivo, char ch);

int strcmp2(const char* a, const char *b);

/*
*
*
*
*/

int main(int argc, char *argv[]){
   	/* Arguments
		argv[1] = length of each object in the trace (number of characters)
		argv[2] = Value S_max which gives the max size of the set S
		argv[3] = Trace file
		argv[4] = file where the MRC is going to be written
		argV[5] = If it exists (argc=5) and is equal to 1, SHARDS will print more pertinent data before the MRC
	*/
	GList *histograma;
	
	Tree *tree =NULL;
	// time_table stores the trace objects with the last time the object was accepted
	GHashTable *time_table = g_hash_table_new_full(g_str_hash, g_str_equal,NULL, ( GDestroyNotify )free);
	// distance_table stores the counters for the reuse distances 
	GHashTable *distance_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, ( GDestroyNotify )free);
	GHashTable *mrc = g_hash_table_new(g_str_hash, g_str_equal);
	

	//The two data structures below represent the Set S
	GHashTable *set_table =  g_hash_table_new_full( g_direct_hash,  g_direct_equal,NULL,NULL);	
	Tree *set_tree = NULL;
	GList *set_list=NULL;
	//set_list2 is used to free the elemtns of the list in case of eviction
	GList *set_list2 =NULL;
	const uint64_t S_max= (uint64_t) strtol(argv[2], NULL, 10);
	uint64_t set_size =0;
		
	

	int obj_length = (int) strtol(argv[1], NULL, 10);
	
	double R = 0.1;
	
	

	uint64_t P = 1;
	P = P<<24;
	uint64_t T =  R*P;
	uint64_t  hash[2];
	uint64_t shifted_value =0 ;	
	uint64_t T_i =0;
	bool object_evicted = false;	

	uint64_t total_objects=0;
	uint64_t num_obj=0;
	
	
	char *object= malloc((obj_length+1)*sizeof(char));

	
	

	unsigned int reuse_dist=0;	
	
	
	FILE *file;
	file = fopen(argv[3], "r");
 
	while(fgets(object, obj_length+2, file)!=NULL){
		object[11]='\0';	
		total_objects++;
		qhashmurmur3_128(object ,obj_length*sizeof(char) ,hash );
		
		//shifted_value =  ([1] >> (8*n)) & 0xff;

		shifted_value = hash[1];
		
		//The following line is equivalent to T_i = shifted_value % P; because P is a power of 2
		T_i = shifted_value & (P-1);
		
		//sprintf("Objeto #%7u: %12s \n", num_obj, object);
	
		//Calculate Reuse distance

		if(T_i < T){


			//First, we save the tuple <object,T_i> on the hashtable called set_table.
			set_tree=insert(T_i,set_tree);	
			set_list = g_hash_table_lookup(set_table, set_tree );			
			//	if by searching the hashtable with the node we get a value of NULL, that means that we have to insert the value			
			if(set_list ==NULL){
				set_list = g_list_append(set_list, object);
				//Is this necessary??
				g_hash_table_insert(set_table, set_tree, set_list); 
				set_size += 1;
				
			}else{
				//we search if the object read from the trace is on the list (several objects might have the same value of T_i)
				
				set_list = g_list_find_custom(set_list, object, (GCompareFunc)strcmp);
				//If its not in the list, we search insert the object at the end of the list
				if(set_list ==NULL){
					set_list = g_list_append(set_list, object);
					set_list = g_list_first(set_list);
					g_hash_table_insert(set_table,set_tree,set_list);
					set_size += 1;
				}
					
				
			
			}
			set_list = NULL;
			//After storing the object in the Set S, we check if we went over the allowed limit S_max
			if(set_size > S_max){
				//Eviction!!!

				//Find the largest value in set_tree
				Tree *evic_tree = NULL;
				
				evic_tree = find_rank((set_tree->size) -1, set_tree);	
				evic_tree =splay( evic_tree->key, evic_tree);
				//Find the objects related mapped to the value from evic_tree
				set_list = g_hash_table_lookup(set_table, evic_tree);
				//Iterate over the list to remove the objects from the other data structures. (tree and time_table)			
			
				while(set_list !=NULL){
				
					//First, remove correspondint time from time_table
					if(set_list->data == object){
						object_evicted=true ;
					}
					set_list2 = set_list;
					//We get each object from the list (that is stored in the data variable) and evict it from the hashtable
					g_hash_table_remove(time_table, (char*) set_list->data);
					
					set_list = g_list_remove_link (set_list, set_list2);
					free(set_list2->data);
					g_list_free (set_list2);
					set_list2=NULL;
					set_size -=1;
	
				}
				
				tree = delete(evic_tree->key, tree );	

				
				//Rescaling
				//Find the value of T_max
				T = ((find_rank((evic_tree->size-1), evic_tree))->key);	
				//POSSIBLE BUG!!!! CASTING SHEANANIGANGS(sp??)					
				R = ((double)T)/P;

				//Check whether the object read from the trace was evicted. If not, calculate reuse distance
				if(!object_evicted){
					

					
		
					num_obj++;
					reuse_dist = calc_reuse_dist( object,  num_obj, &time_table, &tree);
					reuse_dist = (uint64_t)(reuse_dist/R);
					update_dist_table(reuse_dist , &distance_table);	

					printf("%u \n", reuse_dist);
									
				}
				
				object_evicted=false;
			}else{
		
			
				
	
				num_obj++;
				reuse_dist = calc_reuse_dist( object,  num_obj, &time_table, &tree);
				reuse_dist = (uint64_t)(reuse_dist/R);
				update_dist_table(reuse_dist , &distance_table);	

				printf("%u \n", reuse_dist);
			}
				
		}
		
			
		//tmp_str = (char*) malloc(15*sizeof(char));
		
		object= (char*) malloc((obj_length)*sizeof(char));
		// num_obj++ solo debe pasar si se cumple la condicion hash(Li)% P <= T !!!!!!! 
		
	}
	fclose(file);
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
	
	
	
	
	
	return 1;
	
}



uint64_t calc_reuse_dist( char *object, unsigned int num_obj, GHashTable **time_table,  Tree **tree){
	uint64_t timestamp=0;
	uint64_t reuse_dist=0;
	char* num_obj_str = (char*) malloc(15*sizeof(char)); 
	snprintf(num_obj_str, 15*sizeof(char), "%u", num_obj);
	char *reuse_dist_str = (char*) malloc(15*sizeof(char));
	char *time_table_value ;

	time_table_value = (char*)g_hash_table_lookup(*time_table, object);
	
	if(time_table_value==NULL){
				
				

				//printf("if\n");
				g_hash_table_insert(*time_table, object,  num_obj_str);
				*tree = insert(strtol(num_obj_str,NULL,10) ,*tree);
				//En la tabla de distancias de reuso, la clave de 0 equivaler a la clave 'infinito'
				reuse_dist = 0;
			
			
	}else{
				//printf("else\n");
				//Calculate reuse distance
				timestamp = strtol(time_table_value,NULL,10);
				reuse_dist =(uint64_t) calc_distance(timestamp,*tree);
				//Busquemos la distancia de reuso en la hashtable distance_table
				snprintf(reuse_dist_str, 15*sizeof(char), "%"PRIu64"", reuse_dist);
			
			
			

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
	tmp_str = (char*) malloc(15*sizeof(char));	
	
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
	char* tmp = (char*) malloc(15*sizeof(char));		
	
	while( 1 ){
		
		tmp = g_hash_table_lookup(*MRC, hist->data);
		
		fprintf(file,"%s %s\n", (char*)hist->data , tmp );
		
		
		hist=hist->next;
		
		if(hist==NULL){
			break;
		}
		tmp = (char*) malloc(15*sizeof(char));
	}
	fclose(file);	

}

int strcmp2(const char* a, const char *b){

	return strcmp(a,b);

}

int numerical_strcmp(const char* a, const char* b) {
	
	int x = strlen(a); 
	int y = strlen(b);	
			
	return (x != y) ? ((x > y) ? 1 : -1) : strcmp(a, b);
}

