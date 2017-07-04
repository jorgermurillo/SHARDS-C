CC=gcc
CFLAGS= `pkg-config --cflags --libs glib-2.0`
LFLAGS=-std=c99


test4:  SHARDS.o splay.o murmurhash3.o test4.o
	$(CC) -g -Wall test4.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) -std=c99 -o test4

test3:  SHARDS.o splay.o murmurhash3.o test3.o
	$(CC) -g -Wall test3.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) -std=c99 -o test3

test4.o: test4.c 
	$(CC) -g -c -Wall test4.c $(CFLAGS)  -std=c99

test3.o: test3.c 
	$(CC) -g -c -Wall test3.c $(CFLAGS)  -std=c99

test2:  SHARDS.o splay.o murmurhash3.o test2.o
	$(CC) -g -Wall test2.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) -std=c99 -o test2

test2.o: test2.c 
	$(CC) -g -c -Wall test2.c $(CFLAGS)  -std=c99

dist: dist.c
	$(CC) -g -Wall dist.c splay.c $(CFLAGS) -o dist

SHARDS_fixed_rate: SHARDS_fixed_rate.o shards_utils.o splay.o murmurhash3.o
	$(CC) -g -Wall SHARDS_fixed_rate.o shards_utils.o  splay.o murmurhash3.o $(CFLAGS) $(LFLAGS) -o SHARDS_fixed_rate

SHARDS_fixed_size: SHARDS_fixed_size.o shards_utils.o splay.o murmurhash3.o
	$(CC) -g -Wall SHARDS_fixed_size.o shards_utils.o splay.o murmurhash3.o  $(CFLAGS) $(LFLAGS) -o SHARDS_fixed_size

SHARDS_fixed_rate.o: SHARDS_fixed_rate.c
	$(CC) -g -c -Wall SHARDS_fixed_rate.c $(CFLAGS) $(LFLAGS) 

SHARDS_fixed_size.o: SHARDS_fixed_size.c
	$(CC) -g -c -Wall SHARDS_fixed_size.c  $(CFLAGS) $(LFLAGS)

SHARDS.o : SHARDS.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  SHARDS.c

shards_utils.o: shards_utils.c 
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  shards_utils.c

splay.o: splay.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  splay.c

murmurhash3.o: murmurhash3.c 
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  murmurhash3.c


