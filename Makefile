CC=gcc
CFLAGS= `pkg-config --cflags --libs glib-2.0`


dist: dist.c
	$(CC) -g -Wall dist.c splay.c $(CFLAGS) -o dist

SHARDS_fixed_rate: SHARDS_fixed_rate.o shards_utils.o splay.o murmurhash3.o
	$(CC) -g -Wall SHARDS_fixed_rate.o shards_utils.o  splay.o murmurhash3.o $(CFLAGS) -std=c99 -o SHARDS_fixed_rate


SHARDS_fixed_size: SHARDS_fixed_size.o shards_utils.o splay.o murmurhash3.o
	$(CC) -g -Wall SHARDS_fixed_size.o shards_utils.o splay.o murmurhash3.o  $(CFLAGS) -std=c99 -o SHARDS_fixed_size

SHARDS_fixed_rate.o: SHARDS_fixed_rate.c
	$(CC) -c -g -Wall SHARDS_fixed_rate.c $(CFLAGS) -std=c99 

SHARDS_fixed_size.o: SHARDS_fixed_size.c
	$(CC) -g -c -Wall SHARDS_fixed_size.c  $(CFLAGS) -std=c99 

shards_utils.o: shards_utils.c 
	$(CC) -g -c -Wall $(CFLAGS) -std=c99  shards_utils.c

splay.o: splay.c
	$(CC) -g -c -Wall $(CFLAGS) -std=c99  splay.c

murmurhash3.o: murmurhash3.c 
	$(CC) -g -c -Wall $(CFLAGS) -std=c99  murmurhash3.c



