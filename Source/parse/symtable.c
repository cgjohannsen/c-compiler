#include <stdlib.h>
#include <string.h>

#include "symtable.h"


varsym_t * 
init_varsym(astnode_t *type, astnode_t *var)
{
    varsym_t *new = (varsym_t *) malloc(sizeof(varsym_t));

    new->name = (char *) malloc(sizeof(char) * strlen(var->text) + 1);
    strcpy(new->name, var->text);

    new->type = type;
    new->var = var;
    new->next = NULL;

    return new;
}

structsym_t *
init_structsym(astnode_t *type_decl)
{
    structsym_t *new = (structsym_t *) malloc(sizeof(structsym_t));

    new->name = (char *) malloc(sizeof(char) * strlen(type_decl->text) + 1);
    strcpy(new->name, type_decl->text);

    astnode_t *var_decl, *type, *var;
    varsym_t *varsym, *cur;

    var_decl = type_decl->left;
    while(var_decl != NULL) {
        type = var_decl->left;
        var = type->right;
        
        while(var != NULL) {
            varsym = init_varsym(type, var);

            if(new->member == NULL) { // first member
                new->member = varsym;
            } else { // find end of linked list
                cur = new->member;
                while(cur->next != NULL) {
                    cur = cur->next;
                }
                cur->next = varsym;
            }

            var = var->right;
        }

        var_decl = var_decl->right;
    }

    new->next = NULL;

    return new;
}


funsym_t * 
init_funsym(astnode_t *fun_decl, bool is_def)
{
    return NULL;
}

void
add_localvar(symtable_t *table, astnode_t *type, astnode_t *var)
{
    varsym_t *new, *cur;
    
    new = init_varsym(type, var);

    if(table->local_vars == NULL) {
        table->local_vars = new;
        return;
    }

    cur = table->local_vars;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new;
}

void
add_globalvar(symtable_t *table, astnode_t *type, astnode_t *var)
{
    varsym_t *new, *cur;
    
    new = init_varsym(type, var);

    if(table->local_vars == NULL) {
        table->local_vars = new;
        return;
    }

    cur = table->local_vars;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new;
}


void
add_localstruct(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *new, *cur;

    new = init_structsym(type_decl);

    if(table->local_structs == NULL) {
        table->local_structs = new;
        return;
    }

    cur = table->local_structs;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new;
}


void
add_globalstruct(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *new, *cur;

    new = init_structsym(type_decl);

    if(table->global_structs == NULL) {
        table->global_structs = new;
        return;
    }

    cur = table->global_structs;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new;
}


void
add_function(symtable_t *table, astnode_t *fun_decl, bool is_def)
{
    funsym_t *new, *cur;

    new = init_funsym(fun_decl, is_def);

    if(table->functions == NULL) {
        table->functions = new;
        return;
    }

    cur = table->functions;
    while(cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new;
}


varsym_t *
get_globalvar(symtable_t *table, char *name)
{
    varsym_t *cur;

    cur = table->global_vars;
    while(cur != NULL) {
        if(!strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

varsym_t *
get_localvar(symtable_t *table, char *name)
{
    varsym_t *cur;

    cur = table->local_vars;
    while(cur != NULL) {
        if(!strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}


structsym_t *
get_globalstruct(symtable_t *table, char *name)
{
    structsym_t *cur;

    cur = table->global_structs;
    while(cur != NULL) {
        if(!strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

structsym_t *
get_localstruct(symtable_t *table, char *name)
{
    structsym_t *cur;

    cur = table->local_structs;
    while(cur != NULL) {
        if(!strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}


funsym_t *
get_function(symtable_t *table, char *name)
{
    funsym_t *cur;

    cur = table->functions;
    while(cur != NULL) {
        if(!strcmp(cur->name, name)) {
            return cur;
        }
        cur = cur->next;
    }

    return NULL;
}


symtable_t *
init_symtable()
{
    symtable_t *table;

    table = (symtable_t *) malloc(sizeof(symtable_t));

    table->global_vars = NULL;
    table->local_vars = NULL;
    table->global_structs = NULL;
    table->local_structs = NULL;
    table->functions = NULL;
}


void 
free_locals(symtable_t *table)
{

}


void 
free_symtable(symtable_t *table)
{
    free(table);
}