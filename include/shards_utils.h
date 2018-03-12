#ifndef SHARDS_UTILS
#define SHARDS_UTILS


#ifdef __cplusplus
extern "C"{
#endif 

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

 //  vvvvvvvvvvvvv This is  murmurhash3.h vvvvvvvvvvvvv
bool qhashmurmur3_128(const void *data, size_t nbytes, void *retbuf);

// vvvvvvvvvvvvv This is splay.h vvvvvvvvvvvvv

/*
           An implementation of top-down splaying with sizes
             D. Sleator <sleator@cs.cmu.edu>, January 1994.
Modified a little by Qingpeng Niu for tracing the global chunck library memory use. Just add a compute sum of size from search node to the right most node.
*/




//#pragma warning(disable:593)
typedef struct tree_node Tree;
typedef int T;
struct tree_node {
    Tree * left, * right;
    T key;
    T size;   /* maintained to be the number of nodes rooted here */
};

#define compare(i,j) ((i)-(j))
/* This is the comparison.                                       */
/* Returns <0 if i<j, =0 if i=j, and >0 if i>j                   */
 
#define node_size(x) (((x)==NULL) ? 0 : ((x)->size))
/* This macro returns the size of a node.  Unlike "x->size",     */
/* it works even if x=NULL.  The test could be avoided by using  */
/* a special version of NULL which was a real node with size 0.  */
 
Tree * splay (T i, Tree *t);
Tree * insert(T i, Tree * t); 
Tree * deletetree(T i, Tree *t); // Changed the name of this function from delete() to deletetree()
Tree * find_rank(T r, Tree *t); 
void printtree(Tree * t, int d); 
void freetree(Tree* t);
//calc_distance() implemented by Jorge Murillo, 2017.
int calc_distance(T timestamp, Tree *t);

#ifdef __cplusplus
}
#endif

#endif 

