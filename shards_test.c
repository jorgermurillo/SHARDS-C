#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <stdint.h>
#include "SHARDS.h"


int main(int argc, char** argv){

	/*	argv[1] = length of each object.
		argv[2] =  bucket size
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

	while(fgets(object, obj_length+2, file)!=NULL){
		
	
		SHARDS_feed_obj(shards, object, obj_length);
		
		object = (char*)calloc((obj_length+2),sizeof(char));
			
	}

	GHashTable *mrc = MRC_fixed_size_empty(shards);

	FILE *mrc_file = fopen(argv[5],"w");
	GList *keys = g_hash_table_get_keys(mrc);
	keys = g_list_sort(keys, (GCompareFunc) intcmp);

	while(1){
		//printf("%d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );
		fprintf(mrc_file,"%7d,%1.7f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );

		if(keys->next==NULL)
			break;
		keys=keys->next;
	}
	fclose(file);
	fclose(mrc_file);
	keys = g_list_first(keys);
	g_list_free(keys);
	g_hash_table_destroy(mrc);
	SHARDS_free(shards);

}
