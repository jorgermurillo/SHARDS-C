# SHARDS-C

This is an implementation for the SHARDS algorithm written in C. It is used to estimate the Miss Rate Curve for a cache using a stack algorithm (LRU, LFU, MRU) as a eviction policy.

It uses Glib (https://developer.gnome.org/glib/2.54/) for most of the data structures.

Also, it uses an implementation of the top-down splaying binary tree with sizes ( D. Sleator <sleator@cs.cmu.edu>, January 1994.), taken from the PARDA repository (https://bitbucket.org/trauzti/parda). I modified it further so that it could calculate the sum of the sizes from the search node to the right most node.

This work was done as part of a larger project for Dr. Cristina Abad (https://sites.google.com/site/cristinaabad/). 

This work was funded in part by a Google Faculty Research Award.