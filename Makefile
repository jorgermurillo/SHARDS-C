CC=gcc
CFLAGS= -Wall -I./include `pkg-config --cflags --libs glib-2.0`
VERSION=-std=c11
ODIR=obj
AR=ar
ARFLGAS=rcs

shards_test: obj/shards_test.o obj/splay.o obj/murmurhash3.o lib/libshardsc.a
	$(CC) -g $^ $(CFLAGS) $(VERSION) -o $@

obj/shards_test.o: src/shards_test.c include/SHARDS.h
	$(CC) -g -c  src/shards_test.c $(CFLAGS)  $(VERSION) -o $@

dist: dist.c
	$(CC) -g  dist.c splay.c $(CFLAGS) -o dist

obj/SHARDS.o : src/SHARDS.c ./include/SHARDS.h
	$(CC) -g -c  $(CFLAGS) $(LFLAGS)  src/SHARDS.c -o $@

lib/libshardsc.a: obj/SHARDS.o
	$(AR) $(ARFLAGS) $@ $^ 
	
obj/splay.o: src/splay.c include/splay.h
	$(CC) -g -c $(CFLAGS) $(LFLAGS)  src/splay.c -o $@

obj/murmurhash3.o: src/murmurhash3.c include/murmurhash3.h
	$(CC) -g -c $(CFLAGS) $(LFLAGS)  src/murmurhash3.c -o $@

obj/jenkins_hash.o: src/jenkins_hash.c include/jenkins_hash.h
	$(CC) -g -c $(CFLAGS) $(LFLAGS)  src/jenkins_hash.c -o $@
