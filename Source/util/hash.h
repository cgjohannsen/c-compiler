#ifndef HASH_H
#define HASH_H

#include <stdint.h>

typedef struct item {
    char *key;
    uint64_t value;
} item_t;

uint64_t hash(unsigned char *str);

#endif