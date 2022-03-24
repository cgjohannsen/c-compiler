#ifndef AST_H
#define AST_H

#include "token.h"

typedef struct astnode {
    token_t *tok;
    struct astnode *left;
    struct astnode *right;
} astnode_t;

#endif