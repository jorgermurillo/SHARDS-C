#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <time.h>
#include <stdint.h>
#include "SHARDS.h"


int main(int argc, char** argv){

	/*	argv[1] = length of each object.
		argv[2] = Tracefile
		argv[3] = 
	*/
	SHARDS *shards = SHARDS_fixed_size_init_R( 32000,0.5,  10, Uint64);
	
	int obj_length = strtol(argv[1],NULL,10);

	char* object = (char*)calloc((obj_length+2),sizeof(char));

	FILE *file;
	file = fopen(argv[2], "r");
	uint64_t *key = calloc(1,sizeof(uint64_t));
	int cnt =0;

	while(fgets(object, obj_length+2, file)!=NULL){
		cnt++;
		*key = strtoull(object, NULL, 10);
		printf("CNT: %d %"PRIu64"\n", cnt, *key);
		SHARDS_feed_obj(shards, key, sizeof(uint64_t));
		free(object);
		object = (char*)calloc((obj_length+2),sizeof(char));
		key =  calloc(1,sizeof(uint64_t));

		//if(cnt == 50)
		//break;	

	}

	GHashTable *mrc = MRC_fixed_size(shards);

	FILE *mrc_file = fopen(argv[3],"w");
	GList *keys = g_hash_table_get_keys(mrc);
	keys = g_list_sort(keys, (GCompareFunc) intcmp);

	while(1){
		printf("key: %7d  Value: %1.6f\n",*(int*)keys->data, *(double*)g_hash_table_lookup(mrc, keys->data) );
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