#ifndef AST_H
#define AST_H

#include "token.h"

typedef struct astnode {
    token_t *val;
    struct astnode *parent;
    struct astnode **children;
} astnode_t;

typedef struct ast {
    astnode_t *root;
} ast_t;

#endif