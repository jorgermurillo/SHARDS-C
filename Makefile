CC=gcc
CFLAGS= `pkg-config --cflags --libs glib-2.0`
VERSION=-std=c11

exact_mrc: SHARDS.o splay.o murmurhash3.o exact_mrc.o
	$(CC) -g -Wall exact_mrc.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -o exact_mrc

exact_mrc.o: exact_mrc.c
	$(CC) -g -c -Wall exact_mrc.c $(CFLAGS)  $(VERSION)

zeroMQ_Memcached: SHARDS.o splay.o murmurhash3.o zeroMQ_Memcached.o
	$(CC) -g -Wall zeroMQ_Memcached.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -L/usr/local/lib -lzmq -o zeroMQ_Memcached

zeroMQ_Memcached.o: zeroMQ_Memcached.c 
	$(CC) -g -c -Wall zeroMQ_Memcached.c $(CFLAGS) -L/usr/local/lib -lzmq $(VERSION)


zeroMQ_SHARDS: SHARDS.o splay.o murmurhash3.o zeroMQ_SHARDS.o
	$(CC) -g -Wall zeroMQ_SHARDS.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -L/usr/local/lib -lzmq -o zeroMQ_SHARDS

zeroMQ_SHARDS.o: zeroMQ_SHARDS.c 
	$(CC) -g -c -Wall zeroMQ_SHARDS.c $(CFLAGS) -L/usr/local/lib -lzmq $(VERSION)

shards_test: shards_test.o SHARDS.o splay.o murmurhash3.o
	$(CC) -g -Wall shards_test.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -o shards_test

shards_test.o: shards_test.c
	$(CC) -g -c -Wall shards_test.c $(CFLAGS)  $(VERSION)

test5:  SHARDS.o splay.o murmurhash3.o test5.o 
	$(CC) -g -Wall test5.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -o test5


test4:  SHARDS.o splay.o murmurhash3.o test4.o 
	$(CC) -g -Wall test4.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -o test4

test3:  SHARDS.o splay.o murmurhash3.o test3.o
	$(CC) -g -Wall test3.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) -std=c99 -o test3


test5.o: test5.c 
	$(CC) -g -c -Wall test5.c $(CFLAGS)  $(VERSION)

test4.o: test4.c 
	$(CC) -g -c -Wall test4.c $(CFLAGS)  $(VERSION)

test3.o: test3.c 
	$(CC) -g -c -Wall test3.c $(CFLAGS)  $(VERSION)

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

jenkins_hash.o: jenkins_hash.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  jenkins_hash.c
