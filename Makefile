CC=gcc
CFLAGS= `pkg-config --cflags --libs glib-2.0`
VERSION=-std=c11


shards_test: shards_test.o SHARDS.o splay.o murmurhash3.o
	$(CC) -g -Wall shards_test.o splay.o murmurhash3.o SHARDS.o $(CFLAGS) $(VERSION) -o shards_test

shards_test.o: shards_test.c
	$(CC) -g -c -Wall shards_test.c $(CFLAGS)  $(VERSION)

dist: dist.c
	$(CC) -g -Wall dist.c splay.c $(CFLAGS) -o dist

SHARDS.o : SHARDS.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  SHARDS.c

splay.o: splay.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  splay.c

murmurhash3.o: murmurhash3.c 
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  murmurhash3.c

jenkins_hash.o: jenkins_hash.c
	$(CC) -g -c -Wall $(CFLAGS) $(LFLAGS)  jenkins_hash.c
