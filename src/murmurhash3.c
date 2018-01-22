#include "murmurhash3.h"
//Los sgtes son #include son para el main
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

//THIS COME FROM wolkykim/qlibc


bool qhashmurmur3_128(const void *data, size_t nbytes, void *retbuf);

/*
int main(){
	uint64_t buffer[2];
	
	char  *x = "874454515nbcfcvgjdtssryfyu68481455";
	bool one = qhashmurmur3_128( x , 1 ,buffer);
	uint64_t c0 = buffer[0];
	uint64_t c1 = buffer[1];
	printf("%6" PRIu64 " \n",c0);
	printf("%6" PRIu64 " \n",c1);
}
*/

bool qhashmurmur3_128(const void *data, size_t nbytes, void *retbuf) {
    if (data == NULL || nbytes == 0)
        return false;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    const int nblocks = nbytes / 16;
    const uint64_t *blocks = (const uint64_t *) (data);
    const uint8_t *tail = (const uint8_t *) (data + (nblocks * 16));

    uint64_t h1 = 0;
    uint64_t h2 = 0;

    int i;
    uint64_t k1, k2;
    for (i = 0; i < nblocks; i++) {
        k1 = blocks[i * 2 + 0];
        k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1 = (k1 << 31) | (k1 >> (64 - 31));
        k1 *= c2;
        h1 ^= k1;

        h1 = (h1 << 27) | (h1 >> (64 - 27));
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = (k2 << 33) | (k2 >> (64 - 33));
        k2 *= c1;
        h2 ^= k2;

        h2 = (h2 << 31) | (h2 >> (64 - 31));
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    k1 = k2 = 0;
    switch (nbytes & 15) {
        case 15:
            k2 ^= (uint64_t)(tail[14]) << 48;
        case 14:
            k2 ^= (uint64_t)(tail[13]) << 40;
        case 13:
            k2 ^= (uint64_t)(tail[12]) << 32;
        case 12:
            k2 ^= (uint64_t)(tail[11]) << 24;
        case 11:
            k2 ^= (uint64_t)(tail[10]) << 16;
        case 10:
            k2 ^= (uint64_t)(tail[9]) << 8;
        case 9:
            k2 ^= (uint64_t)(tail[8]) << 0;
            k2 *= c2;
            k2 = (k2 << 33) | (k2 >> (64 - 33));
            k2 *= c1;
            h2 ^= k2;

        case 8:
            k1 ^= (uint64_t)(tail[7]) << 56;
        case 7:
            k1 ^= (uint64_t)(tail[6]) << 48;
        case 6:
            k1 ^= (uint64_t)(tail[5]) << 40;
        case 5:
            k1 ^= (uint64_t)(tail[4]) << 32;
        case 4:
            k1 ^= (uint64_t)(tail[3]) << 24;
        case 3:
            k1 ^= (uint64_t)(tail[2]) << 16;
        case 2:
            k1 ^= (uint64_t)(tail[1]) << 8;
        case 1:
            k1 ^= (uint64_t)(tail[0]) << 0;
            k1 *= c1;
            k1 = (k1 << 31) | (k1 >> (64 - 31));
            k1 *= c2;
            h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= nbytes;
    h2 ^= nbytes;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 33;
    h1 *= 0xff51afd7ed558ccdULL;
    h1 ^= h1 >> 33;
    h1 *= 0xc4ceb9fe1a85ec53ULL;
    h1 ^= h1 >> 33;

    h2 ^= h2 >> 33;
    h2 *= 0xff51afd7ed558ccdULL;
    h2 ^= h2 >> 33;
    h2 *= 0xc4ceb9fe1a85ec53ULL;
    h2 ^= h2 >> 33;

    h1 += h2;
    h2 += h1;

    ((uint64_t *) retbuf)[0] = h1;
    ((uint64_t *) retbuf)[1] = h2;

    return true;
}
