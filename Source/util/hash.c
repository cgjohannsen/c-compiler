#include "hash.h"

// function taken from:
// http://www.cse.yorku.ca/~oz/hash.html
uint64_t hash(unsigned char *str) {
    uint64_t hash = 5381;
    uint32_t c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
