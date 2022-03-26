#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

#include "ast.h"

#define MIN_SYMTABLE_SIZE 256

typedef enum symtype {
    __CHAR,
    __INT,
    __FLOAT,
    __STRING,
    __STRUCT
} symtype_t;

typedef struct symparam {
    struct symparam *next;
    char *ident;
    char *type;
} symparam_t;

typedef struct symentry {
    struct symentry *next;
    char *ident;
    char *type;
    bool is_array;
    symparam_t *param;
} symentry_t;

typedef struct symtable {
    symentry_t *head; // use linked list
} symtable_t;

void init_table(symtable_t *);
void add_entry(symtable_t *, symentry_t *);
void remove_entry(symtable_t *, char *);
symentry_t *get_entry(symtable_t *, char *);

#endif