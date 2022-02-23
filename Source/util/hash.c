#include "hash.h"

/*
 * Returns a hash of the inputted string. Taken from:
 * http://www.cse.yorku.ca/~oz/hash.html
 *
 * str: input string to be hashed
 *
 * return: hash of type uint64_t
 */
uint64_t hash(char *str) {
    uint64_t hash = 5381;
    uint32_t c;

    while((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
