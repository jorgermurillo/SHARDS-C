CC=gcc
CFLAGS= `pkg-config --cflags --libs glib-2.0`


dist: dist.c
	$(CC) -g -Wall dist.c splay.c $(CFLAGS) -o dist

SHARDS_fixed_rate: SHARDS_fixed_rate.c
	$(CC) -g -Wall SHARDS_fixed_rate.c splay.c murmurhash3.c $(CFLAGS) -std=c99 -o SHARDS_fixed_rate

SHARDS_fixed_size: SHARDS_fixed_size.c
	$(CC) -g -Wall SHARDS_fixed_size.c splay.c murmurhash3.c $(CFLAGS) -std=c99 -o SHARDS_fixed_size

SHARDS_fixed_rate_custom: SHARDS_fixed_rate_custom.c
	$(CC) -g -Wall SHARDS_fixed_rate_custom.c splay.c murmurhash3.c $(CFLAGS) -std=c99 -o SHARDS_fixed_rate_custom


