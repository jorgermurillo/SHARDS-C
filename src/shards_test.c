#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <stdint.h>
#include "SHARDS.h"

int main(int argc, char** argv){

	/*	argv[1] = length of each object.
		argv[2] = bucket size
		argv[3] = R
		argv[4] = Tracefile
		argv[5] = mrc file 
	*/
	
	printf("SHARDS\n");
	int obj_length = strtol(argv[1],NULL,10);

	char* object = (char*)calloc((obj_length+2),sizeof(char));

	int bucket = strtol(argv[2],NULL,10);
	
	

	double R = strtod(argv[3], NULL);

	SHARDS *shards = SHARDS_fixed_size_init_R( 32000,R,  (unsigned int)bucket, String);

	FILE *file;
	file = fopen(argv[4], "r");
	clock_t start_time = clock();

	int cnt = 0;

	while(fgets(object, obj_length+2, file)!=NULL){
		
	
		SHARDS_feed_obj(shards, object, obj_length);
		
		object = (char*)calloc((obj_length+2),sizeof(char));
		cnt++;
		/*
		if(cnt==100000){
			break;
		}
		*/
	}



	

    //fclose(file);

	//printf("Loop 1 ended.\n");
	GHashTable *mrc = MRC(shards);

	
	FILE *mrc_file = fopen(argv[5],"w");
	GList *keys = g_hash_table_get_keys(mrc);
	keys = g_list_sort(keys, (GCompareFunc) intcmp);
    GList *first = keys;
	while(keys!=NULL){
		//printf("%d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );
		fprintf(mrc_file,"%7d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );


		keys=keys->next;
	}

	/*
	g_hash_table_destroy(mrc);


	//HERE STARS THE SECOND LOOP READING WHATS LEFT OF THE TRACE FILE
	while(fgets(object, obj_length+2, file)!=NULL){
		
	
		SHARDS_feed_obj(shards, object, obj_length);
		
		object = (char*)calloc((obj_length+2),sizeof(char));
		cnt++;
		
	}

	free(object);
	printf("Loop 2 ended \n");
	mrc = MRC(shards);
	
	printf("MRC created.\n");
	*/
	clock_t end_time = clock();
	
	fclose(mrc_file);
	g_list_free(first);
	
	g_hash_table_destroy(mrc);
	
	printf("%ld\n", start_time);
	printf("%ld\n", end_time);
	int total_time = ((end_time - start_time))/CLOCKS_PER_SEC;
	printf("TIME: %d\n", total_time);
	unsigned int objects_parsed =  shards->total_objects;
    
	double throughput = objects_parsed/(total_time+1);
    SHARDS_free(shards);
	printf("Throughput: %f\n", throughput);

	//SHARDS_free(shards);
    return 0;

}
