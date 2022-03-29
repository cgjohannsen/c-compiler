#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"

astnode_t * 
init_astnode(asttype_t asttype, char *text)
{
    astnode_t *node = (astnode_t *) malloc(sizeof(astnode_t));

    node->text = (char *) malloc(sizeof(char) * (strlen(text) + 1));
    strcpy(node->text, text);

    node->left = NULL;
    node->right = NULL;
    node->type = asttype;
    node->is_array = false;
    node->is_const = false;
    node->is_struct = false;

    switch(asttype) {
        case _CHAR_LIT: 
            node->val.c = *text;
            break;
        case _INT_LIT:
            node->val.i = atoi(text);
            break;
        case _REAL_LIT:
            node->val.f = atof(text);
            break;
        default:
            break;
    }

    return node;
}



void 
set_asttext(astnode_t *node, char *text)
{
    node->text = (char *) realloc(node->text,sizeof(char) * (strlen(text) + 1));
    strcpy(node->text, text);
}



void 
set_astchar(astnode_t *node, char c)
{
    node->val.c = c;
}



void 
set_astint(astnode_t *node, int i)
{
    node->val.i = i;
}



void 
set_astfloat(astnode_t *node, float f)
{
    node->val.f = f;
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

    free_ast(node->left);
    free_ast(node->right);
    free(node->text);
    free(node);
}



int 
is_inttype(asttype_t asttype)
{
    return asttype == _CHAR_LIT || asttype == _INT_LIT;
}



int is_numerictype(asttype_t asttype)
{   
    return asttype == _CHAR_LIT || asttype == _INT_LIT || asttype == _REAL_LIT;
}

