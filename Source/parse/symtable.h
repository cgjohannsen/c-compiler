#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

#define MIN_SYMTABLE_SIZE 256

typedef struct symentry {
    char *ident;
    asttype_t type;
    int size;
    int max_size;
} symentry_t;

typedef symentry_t *symtable symtable_t;

void init_table(symtable_t *table);
void add_entry(symtable_t *table, symentry_t *entry);

#endif