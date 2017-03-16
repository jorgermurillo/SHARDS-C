#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

typedef enum {
    BM_READ_OP,
    BM_WRITE_OP,
} bm_op_type_t;

typedef struct {
    bm_op_type_t type;
    uint32_t     key_hv;
} bm_op_t;

int main (void)
{
    //  Socket to talk to clients
    void* context = zmq_ctx_new ();
    void* consumer = zmq_socket (context, ZMQ_SUB);
    zmq_connect (consumer, "tcp://localhost:5555");
    zmq_setsockopt(consumer, ZMQ_SUBSCRIBE, NULL, 0);
    fprintf (stderr, "Connected consumer...\n");

    while (1) {
        char buffer[sizeof(bm_op_t)];
        int nbytes = zmq_recv(consumer, buffer, sizeof(bm_op_t), ZMQ_DONTWAIT);
        if (sizeof(bm_op_t) == nbytes) {
            bm_op_t* op_ptr = (bm_op_t*) buffer;
            fprintf(stderr, "Received op: %d, key: %"PRIu32"\n", op_ptr->type, op_ptr->key_hv);    
        }
    }
    return 0;
}