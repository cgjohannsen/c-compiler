#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../util/io.h"
#include "ast.h"

astnode_t * 
init_astnode(nodetype_t asttype, token_t *tok)
{
    astnode_t *node = (astnode_t *) malloc(sizeof(astnode_t));

    node->text = (char *) malloc(strlen(tok->text) + 1);
    node->ctype.name = (char *) malloc(strlen(tok->text) + 1);

    strcpy(node->text, tok->text);

    node->filename = tok->filename;
    node->line_num = tok->line_num;
    node->left = NULL;
    node->right = NULL;
    node->node_type = asttype;
    node->ctype.is_array = false;
    node->ctype.is_const = false;
    node->ctype.is_struct = false;
    node->arr_dim = -1;

    return node;
}



void 
set_asttext(astnode_t *node, char *text)
{
    node->text = (char *) realloc(node->text, strlen(text) + 1);
    strcpy(node->text, text);
}


void 
set_ctypename(astnode_t *node, char *name)
{
    if(node == NULL) {
        return;
    }

    node->ctype.name = (char *) realloc(node->ctype.name, strlen(name) + 1);
    strcpy(node->ctype.name, name);
}


int 
add_astchild(astnode_t *parent, astnode_t *child)
{
    if(child == NULL) {
        return 0;
    }

    astnode_t *cur;
    cur = parent->left;

    if(cur == NULL) { // case when parent has no children
        parent->left = child;
        return 1;
    }

    while(cur->right != NULL) {
        cur = cur->right;
    }

    cur->right = child;

    return 1;
}



int 
add_astchildleft(astnode_t *parent, astnode_t *child)
{
    if(child == NULL) {
        return 0;
    }

    if(parent->left == NULL) { // case when parent has no children
        parent->left = child;
        return 1;
    }

    astnode_t *tmp;
    tmp = parent->left;
    parent->left = child;
    child->right = tmp;

    return 1;
}



int 
add_astsibling(astnode_t *node, astnode_t *sibling)
{
    if(sibling == NULL) {
        return 0;
    }

    astnode_t *cur;
    cur = node->right;

    if(cur == NULL) { // case where node is only child
        node->right = sibling;
        return 1;
    }

    while(cur->right != NULL) {
        cur = cur->right;
    }

    cur->right = sibling;

    return 1;
}



void 
free_ast(astnode_t *node)
{
    if(node == NULL) {
        return;
    }

    if(node->ctype.name != NULL) {
        free(node->ctype.name);
    }

    free_ast(node->left);
    free_ast(node->right);
    free(node->text);
    free(node);
}



bool 
is_lvalue(astnode_t *node)
{  
    if(node == NULL) {
        return true;
    }

    if(node->node_type != _ARR_ACCESS && node->node_type != _STRUCT_ACCESS && 
        node->node_type != _VAR || node->right != NULL) {
        return false;
    }

    if(node->node_type == _VAR) { 
        return true;
    }

    if(node->node_type == _ARR_ACCESS) {
        return is_lvalue(node->right);
    } 
    return is_lvalue(node->left);
}


bool
is_samectype(astnode_t *type1, astnode_t *type2)
{
    return type1->ctype.is_const == type2->ctype.is_const &&
           type1->ctype.is_array == type2->ctype.is_array && 
           !strcmp(type1->ctype.name, type2->ctype.name);
}


bool
is_numctype(astnode_t *type)
{
    return (!(strcmp(type->ctype.name, "char")) ||
            !(strcmp(type->ctype.name, "int")) ||
            !(strcmp(type->ctype.name, "float"))) &&
            !type->ctype.is_array;
}


bool
is_intctype(astnode_t *type)
{
    return (!(strcmp(type->ctype.name, "char")) ||
            !(strcmp(type->ctype.name, "int"))) &&
            !type->ctype.is_array;
}


void 
copy_ctype(astnode_t *source, astnode_t *result)
{
    set_ctypename(result, source->ctype.name);
    result->ctype.is_array = source->ctype.is_array;
    result->ctype.is_const = source->ctype.is_const;
}


astnode_t *
copy_astnode(astnode_t *node, bool is_top)
{
    if(node == NULL) {
        return NULL;
    }

    astnode_t *copy = (astnode_t *) malloc(sizeof(astnode_t));

    copy->text = (char *) malloc(strlen(node->text) + 1);
    copy->ctype.name = (char *) malloc(strlen(node->text) + 1);

    strcpy(copy->text, node->text);

    copy->filename = node->filename;
    copy->line_num = node->line_num;
    copy->node_type = node->node_type;
    copy->ctype.is_array = node->ctype.is_array;
    copy->ctype.is_const = node->ctype.is_const;
    copy->ctype.is_struct = node->ctype.is_struct;
    copy->arr_dim = node->arr_dim;

    // copy children/siblings
    copy->left = copy_astnode(node->left, false);    
    if(!is_top) {
        copy->right = copy_astnode(node->right, false);    
    }

    return copy;
}


void 
print_ctype(astnode_t *node)
{
    if(node->ctype.is_const) {
        fprintf(outfile, "const ");
    }
    fprintf(outfile, "%s", node->ctype.name);
    if(node->ctype.is_array) {
        fprintf(outfile, "[]");
    }
}



void
print_astrec(astnode_t *node, int depth) 
{
    if(node == NULL) {
        return;
    }

    int i = 0;

    while(i < depth) {
        fprintf(stderr, "\t");
        i++;
    }

    fprintf(stderr, "%s\n", node->text);

    astnode_t *child;
    child = node->left;

    while(child != NULL) {
        print_astrec(child, depth+1);
        child = child->right;
    }
}



void
print_ast(astnode_t *node)
{
    print_astrec(node, 0);
}