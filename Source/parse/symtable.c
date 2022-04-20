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
    funsym_t *new;

    new = (funsym_t *) malloc(sizeof(funsym_t));

    astnode_t *ret_type, *ident, *args;

    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;

    new->is_def = is_def;
    new->ret_type = ret_type;
    new->param = NULL;
    new->next = NULL;

    new->name = (char *) malloc(sizeof(char) * strlen(ident->text) + 1);
    strcpy(new->name, ident->text);

    astnode_t *param;
    varsym_t *new_param, *cur;

    param = args->left;

    while(param != NULL) {
        new_param = init_varsym(param, param->right);

        if(new->param == NULL) {
            new->param = new_param;
        } else {
            cur = new->param;
            while(cur->next != NULL) {
                cur = cur->next;
            }
            cur->next = new_param;
        }

        param = param->right->right;
    }

    return new;
}

bool
add_localvar(symtable_t *table, astnode_t *type, astnode_t *var)
{
    varsym_t *new, *cur;
    
    new = init_varsym(type, var);

    if(table->local_vars == NULL) {
        table->local_vars = new;
        new->idx = 0;
        table->num_locals += 1;
        return true;
    }

    cur = table->local_vars;
    while(cur->next != NULL) {
        if(!strcmp(cur->name, var->text)) {
            // failed -- variable declaration already in table
            return false;
        }
        cur = cur->next;
    }
    cur->next = new;

    new->idx = table->num_locals;
    table->num_locals += 1;


    return true;
}

bool
add_globalvar(symtable_t *table, astnode_t *type, astnode_t *var)
{
    varsym_t *new, *cur;
    
    new = init_varsym(type, var);

    if(table->global_vars == NULL) {
        table->global_vars = new;
        return true;
    }

    cur = table->global_vars;
    while(cur->next != NULL) {
        if(!strcmp(cur->name, var->text)) {
            // failed -- variable declaration already in table
            return false;
        }
        cur = cur->next;
    }
    cur->next = new;

    return true;
}


bool
add_localstruct(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *new, *cur;

    new = init_structsym(type_decl);

    if(table->local_structs == NULL) {
        table->local_structs = new;
        return true;
    }

    cur = table->local_structs;
    while(cur->next != NULL) {
        if(!strcmp(cur->name, type_decl->text)) {
            // failed -- type declaration already in table
            return false;
        }
        cur = cur->next;
    }
    cur->next = new;

    return true;
}


bool
add_globalstruct(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *new, *cur;

    new = init_structsym(type_decl);

    if(table->global_structs == NULL) {
        table->global_structs = new;
        return true;
    }

    cur = table->global_structs;
    while(cur->next != NULL) {
        if(!strcmp(cur->name, type_decl->text)) {
            // failed -- type declaration already in table
            return false;
        }
        cur = cur->next;
    }
    cur->next = new;

    return true;
}


bool
add_function(symtable_t *table, astnode_t *fun_decl, bool is_def)
{
    funsym_t *new, *cur;

    new = init_funsym(fun_decl, is_def);

    if(table->functions == NULL) {
        table->functions = new;
        return true;
    }

    cur = table->functions;
    while(cur->next != NULL) {
        if(!strcmp(cur->name, fun_decl->left->right->text)) {
            // failed -- function already in table
            return false;
        }
        cur = cur->next;
    }
    cur->next = new;

    return true;
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

    table->num_locals = 0;
    table->global_vars = NULL;
    table->local_vars = NULL;
    table->global_structs = NULL;
    table->local_structs = NULL;
    table->functions = NULL;
}


void
free_structsym(structsym_t *sym)
{
    free(sym->name);

    varsym_t *cur, *next;

    cur = sym->member;
    while(cur != NULL) {
        next = cur->next;
        free(cur->name);
        free(cur);
        cur = next;
    }

    free(sym);
}


void 
free_locals(symtable_t *table)
{
    varsym_t *cur1, *next1;

    cur1 = table->local_vars;
    while(cur1 != NULL) {
        next1 = cur1->next;
        free(cur1->name);
        free(cur1);
        cur1 = next1;
    }

    structsym_t *cur2, *next2;
    cur2 = table->local_structs;
    while(cur2 != NULL) {
        next2 = cur2->next;
        free_structsym(cur2);
        cur2 = next2;
    }

    table->local_vars = NULL;
    table->num_locals = 0;
}


void 
free_symtable(symtable_t *table)
{
    free(table);
}