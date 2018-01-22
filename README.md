# SHARDS-C

This is an implementation of the SHARDS algorithm written in C. It is used to estimate the Miss Rate Curve for a cache using a stack algorithm (LRU, LFU, MRU but this implementation works only with LRU) as a eviction policy. To read more on SHARDS, check out the following link: https://www.usenix.org/system/files/conference/fast15/fast15-paper-waldspurger.pdf

It uses Glib (https://developer.gnome.org/glib/2.54/) for most of the data structures.

Also, it uses an implementation of the top-down splaying binary tree with sizes ( D. Sleator <sleator@cs.cmu.edu>, January 1994.), taken from the PARDA repository (https://bitbucket.org/trauzti/parda). I modified it further so that it could calculate the sum of the sizes from the search node to the right most node.

### TODO

- [ ] Implement the SHARDS_adj, to deal with the error produced by vertical shifts of the estimated curve.
- [ ] Implement a function that returns the updated reuse distance distribution from the SHARDS_fixed_size version (It doesnt automatically update after an eviction).
- [ ] Restructure the project as a shared and a static library.
This work was done as part of a larger project for Dr. Cristina Abad (https://sites.google.com/site/cristinaabad/). 

## Instructions on how to use SHARDS

To construct a Miss Rate Curve using SHARDS, you first need to declare a SHARDS element and initialize it using one of the three initilization functions. There are two versions of SHARDS:  Fixed-Rate and Fixed-Size. Generally speaking you want to use the Fixed-Size version, as it has a limit on the amount of memory it uses, the other version I implemented in order to understand the algorithm. To initialize a SHARDS Fixed-Size structure you write:
```
	SHARDS* shards = SHARDS* SHARDS_fixed_size_init(16000, 10, String);
```

This gives you a SHARDS structure that allows a maximum of 16000 unique objects to be accepted before triggering eviction, has a bucket size of 10 for the reuse distance histogram and works with objects of type String (so char* basically). This version sets the value of R_initial to 0.1. If you want to use a different value (for example 0.5), you can use the alternative initialization function:

```
	SHARDS* shards = SHARDS_fixed_size_init_R(16000, 0.5, 10, String);	
```

Now that the SHARDS element is initialized, you just need to feed it the objects that go into the cache. To do this, you initialize a pointer of the type that the SHARDS structure accepts (in this case String, meaning char* ), with the value of the object. Afterwards you use the SHARDS_feed_obj to have the shards structure analyze the object.

```
	int length = ...;//Length of the object in bytes
	char * obj = malloc(length* sizeof(char)); 

	... // Here you set the objects value

	SHARDS_feed_obj(shards, obj, length);
```

Once you finished feeding the objects of the workload, you can construct the MRC by using the following function: 

```
	MRC_fixed_size(shards);
```

or

```
	MRC_fixed_size_empty(shards);
```

The first one returns a GHashTable with the key-value pairs [size : MissRate]. The second function does the same, but in addition it erases all the data from the internal data structures, such that you can start another analyzing another workload in order to estimate its MRC.

Once finished, you can free the SHARDS structure with 

```	
	SHARDS_free(shards);
```

The file shards_test.c has an example of a program that reads a trace from another file and constructs the MRC for it.


## Referencing our work

If you found our tool useful and use it in research, please cite our work as follows:

Instrumenting cloud caches for online workload monitoring 
Jorge R. Murillo, Gustavo Totoy, Cristina L. Abad 
16th Workshop on Adaptive and Reflective Middleware (ARM), co-located with ACM/IFIP/USENIX Middleware, 2017. 
Code available at: https://github.com/jorgermurillo/SHARDS-C

## Acknowledgements

This work was funded in part by a Google Faculty Research Award.

This work was possible thanks to the Amazon Web Services Cloud Credits for Research program.

miau :cat:
