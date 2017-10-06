# SHARDS-C

This is an implementation of the SHARDS algorithm written in C. It is used to estimate the Miss Rate Curve for a cache using a stack algorithm (LRU, LFU, MRU) as a eviction policy. To read more on SHARDS, check out the following link: https://www.usenix.org/system/files/conference/fast15/fast15-paper-waldspurger.pdf

It uses Glib (https://developer.gnome.org/glib/2.54/) for most of the data structures.

Also, it uses an implementation of the top-down splaying binary tree with sizes ( D. Sleator <sleator@cs.cmu.edu>, January 1994.), taken from the PARDA repository (https://bitbucket.org/trauzti/parda). I modified it further so that it could calculate the sum of the sizes from the search node to the right most node.

TODO: Implement the SHARDS_adj, to deal with the error produced by vertical shifts of the estimated curve.

This work was done as part of a larger project for Dr. Cristina Abad (https://sites.google.com/site/cristinaabad/). 

This work was funded in part by a Google Faculty Research Award.

miau :cat: