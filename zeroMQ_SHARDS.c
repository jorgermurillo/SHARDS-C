#include <stdint.h>
#include "k_v_benchmark.h"
#include "SHARDS.h"
#include <zmq.h>
#include <assert.h>
#include <inttypes.h>


int main (int argc, char *argv [])
{
    //  Socket to talk to server
    
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, "tcp://localhost:5555");
    assert (rc == 0);

  
    //rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, filter, strlen (filter));
    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);
    assert (rc == 0);

    SHARDS *shards = SHARDS_fixed_size_init(32000, 11, Uint64);
         
    bm_op_t rec_op = {BM_WRITE_OP, 0};
    uint64_t *key = malloc(sizeof(uint64_t)); 
    
    int cnt =0;

    while(1){

        if (cnt==611968){
            break;
        }

        zmq_recv(subscriber,&rec_op , sizeof(bm_op_t), 0);
        printf("%"PRIu64"\n", rec_op.key_hv);
        *key = rec_op.key_hv;
        SHARDS_feed_obj(shards, key, sizeof(uint64_t));
        key = malloc(sizeof(uint64_t)); 
        cnt++;
        

    }
    
    GHashTable *mrc = MRC_fixed_size_empty(shards);

    SHARDS_free(shards);

    GList *keys = g_hash_table_get_keys(mrc);

    keys = g_list_sort(keys, (GCompareFunc) intcmp); 
    
    FILE *file = fopen("./Results/zeroMQ_YT_32000.dat","w");

    while(1){
        printf("%7d  %2.7f\n", *(int*)keys->data, *(double*)g_hash_table_lookup(mrc,keys->data ));
        fprintf(file, "%d,%1.7f\n", *(int*)keys->data, *(double*)g_hash_table_lookup(mrc,keys->data ));
        if(keys->next==NULL)
            break;
        keys=keys->next;
    }
    keys = g_list_first(keys);

    g_list_free_full(keys, ( GDestroyNotify)free);
    fclose(file);

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}