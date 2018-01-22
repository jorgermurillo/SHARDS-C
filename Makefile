CC=gcc
CFLAGS= -Wall -I./include `pkg-config --cflags --libs glib-2.0`
VERSION=-std=c11
ODIR=obj
AR=ar
ARFLGAS=rcs

dynamic: lib/libSHARDS.so
	
lib/libSHARDS.so: obj/murmurhash3.o obj/shard_utils.o
	$(CC)  -g -fPIC  $(VERSION)  -Wextra -pedantic obj/murmurhash3.o obj/splay.o obj/SHARDS.o -shared $(CFLAGS) -o lib/libSHARDS.so

static: lib/libSHARDS.a
	
lib/libSHARDS.a: obj/SHARDS.o obj/shard_utils.o
	$(AR) $(ARFLAGS) lib/libSHARDS.a $^ 

# The -I./include from the CFLAGS is not really needed in thi step, as the header file for libSHARDS.so was already included en the shards_test.o .
# However it won't hurt the compiling process, which is why we leave that flag in the CFLAGS variable and use it here.

shards_test: obj/shards_test.o lib/libSHARDS.a
	$(CC) -g  $^ $(CFLAGS) $(VERSION) -o $@

shards_test2: obj/shards_test.o lib/libSHARDS.so
	$(CC) -g $^ $(CFLAGS) $(VERSION) -o $@

obj/shards_test.o: src/shards_test.c 
	$(CC) -g -c  src/shards_test.c $(CFLAGS)  $(VERSION) -o $@

obj/SHARDS.o : src/SHARDS.c 
	$(CC) -g -fPIC -c  $(CFLAGS) $(LFLAGS)  src/SHARDS.c -o $@

obj/shard_utils.o: src/shards_utils.c
	$(CC) -g  -fPIC -c $(CFLAGS) $(LFLAGS)  src/shards_utils.c -o $@
#obj/splay.o: src/splay.c 
#	$(CC) -g  -fPIC -c $(CFLAGS) $(LFLAGS)  src/splay.c -o $@

#obj/murmurhash3.o: src/murmurhash3.c 
#	$(CC) -g -fPIC -c  $(CFLAGS) $(LFLAGS)  src/murmurhash3.c -o $@

