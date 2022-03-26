#ifndef AST_H
#define AST_H

#include "token.h"
#include "stdbool.h"

typedef enum asttype {
    _PROGRAM,
    _VAR_DECL,
    _TYPE_DECL,
    _FUN_DECL,
    _FUN_DEF,
    _FUN_BODY,
    _BREAK,
    _CONTINUE,
    _RETURN,
    _IF_STATEMENT,
    _IF_COND,
    _IF_BODY,
    _ELSE_BODY,
    _FOR_STATEMENT,
    _FOR_INIT,
    _FOR_EXIT,
    _FOR_UPDATE,
    _WHILE_STATEMENT,
    _WHILE_COND,
    _WHILE_BODY,
    _DO_STATEMENT,
    _DO_COND,
    _DO_BODY,
    _STATEMENT_BLOCK,
    _ASSIGNOP,
    _UNARYOP,
    _BINARYOP,
    _DOT,
    _TERNARY,
    _COLON,
    _CHAR_LIT,
    _INT_LIT,
    _FLOAT_LIT,
    _STRING_LIT,
    _VAR
} asttype_t;

typedef struct astnode {
    struct astnode *left;
    struct astnode *right;
    asttype_t asttype;
    char *text;
    char *type;
    bool is_array;
    bool is_const;
    bool is_struct;
} astnode_t;

void init_astnode(astnode_t *node, asttype_t asttype);
void init_astnode(astnode_t *node);
void add_astchild(astnode_t *parent, astnode_t *child);
void add_astsibling(astnode_t *node, astnode_t *sibling);
void free_ast(astnode_t *root);

int is_inttype(asttype_t asttype);
int is_numerictype(asttype_t asttype);
int is_vartype(asttype_t asttype);

#endif