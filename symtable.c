#include <stdlib.h>

#include "symtable.h"

symtype_t *
init_symtype(basicsymtype_t type, char *struct_name, bool is_const, bool is_array, int array_size)
{
    symtype_t *symtype = (symtype_t *) malloc(sizeof(symtype_t));

    symtype->type = type;
    
    if(type == __STRUCT) {
        symtype->struct_name = (char *) malloc(sizeof(char) * strlen(struct_name) + 1);
        strcpy(symtype->struct_name, struct_name);
    }

    symtype->is_const = is_const;
    symtype->is_array = is_array;
    symtype->array_size = array_size;

    return symtype;
}

void 
add_sym(sym_t *sym, char *name, symtype_t *type)
{
    sym_t *new = (sym_t *) malloc(sizeof(sym_t));

    new->name = (char *) malloc(sizeof(char) * strlen(name) + 1);
    strcpy(new->name, name);

    new->type = type;

    if(sym == NULL) {
        sym = new;
    }

    sym_t *cur = sym;
    while(cur->right != NULL) {
        cur = cur->right
    }

    cur->right = new;
}


void 
add_funsym(funsym_t *sym, char *name, symtype_t *ret_type);


void 
add_structsym(structsym_t *sym, char *name);


void 
add_funsymparam(funsym_t *sym, char *name, symtype_t *type);


void 
add_structsymmember(structsym_t *sym, char *name, symtype_t *type);



void *
get_sym(symtable_t *table, char *name);