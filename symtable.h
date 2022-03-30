#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include <stdarg.h>

#include "ast.h"

#define MIN_SYMTABLE_SIZE 256

typedef enum basicsymtype {
    __CHAR,
    __INT,
    __REAL,
    __STRING,
    __STRUCT
} basicsymtype_t;

typedef struct symtype {
    basicsymtype_t type;
    char *struct_name;
    bool is_const;
    bool is_array;
    int array_size;    
} symtype_t;

typedef struct sym {
    struct sym *next;
    char *name;
    symtype_t type;
} sym_t;

typedef struct funsym {
    struct funsym *next;
    char *name;
    symtype_t ret_type;
    sym_t *param;
} funsym_t;

typedef struct structsym {
    struct structsym *next;
    char *name;
    sym_t *member;
} structsym_t;

typedef struct symtable {
    sym_t *global_vars;
    sym_t *local_vars;
    structsym_t *global_structs;
    structsym_t *local_structs;
    funsym_t *functions;
} symtable_t;

symtype_t *init_symtype(basicsymtype_t type, char *struct_name, bool is_const, 
                        bool is_array, int array_size);

void add_sym(sym_t *sym, char *name, symtype_t *type);
void add_funsym(funsym_t *sym, char *name, symtype_t *ret_type);
void add_structsym(structsym_t *sym, char *name);

void add_funsymparam(funsym_t *sym, char *name, symtype_t *type);
void add_structsymmember(structsym_t *sym, char *name, symtype_t *type);

void *get_sym(symtable_t *table, char *name);

#endif