#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>
#include <stdarg.h>

#include "ast.h"

#define MIN_SYMTABLE_SIZE 256

typedef struct varsym {
    struct varsym *next;
    char *name;
    astnode_t *type;
    astnode_t *var;
    bool is_init;
    int idx;
} varsym_t;

typedef struct funsym {
    struct funsym *next;
    char *name;
    bool is_def;
    astnode_t *ret_type;
    varsym_t *param;
} funsym_t;

typedef struct structsym {
    struct structsym *next;
    char *name;
    varsym_t *member;
} structsym_t;

typedef struct symtable {
    varsym_t *global_vars;
    varsym_t *local_vars;
    structsym_t *global_structs;
    structsym_t *local_structs;
    funsym_t *functions;
    astnode_t *ret_type;
} symtable_t;

symtable_t *init_symtable(void);

bool add_localvar(symtable_t *table, astnode_t *type, astnode_t *var);
bool add_globalvar(symtable_t *table, astnode_t *type, astnode_t *var);
bool add_localstruct(symtable_t *table, astnode_t *type_decl);
bool add_globalstruct(symtable_t *table, astnode_t *type_decl);
bool add_function(symtable_t *table, astnode_t *fun_decl, bool is_def);

void free_locals(symtable_t *table);
void free_symtable(symtable_t *table);

varsym_t *get_globalvar(symtable_t *table, char *name);
varsym_t *get_localvar(symtable_t *table, char *name);
structsym_t *get_globalstruct(symtable_t *table, char *name);
structsym_t *get_localstruct(symtable_t *table, char *name);
funsym_t *get_function(symtable_t *table, char *name);

bool is_var(symtable_t *table, char *name);
bool is_struct(symtable_t *table, char *name);
bool is_function(symtable_t *table, char *name);

#endif