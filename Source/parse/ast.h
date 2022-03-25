#ifndef AST_H
#define AST_H

#include "token.h"
#include "stdbool.h"

typedef enum asttype {
    _NONE,
    _PROGRAM,
    _VAR_DECL,
    _VAR_LIST,
    _TYPE_DECL,
    _FUN_PROTO,
    _FUN_DECL,
    _PARAM_LIST,
    _FUN_DEF,
    _EXPR,
    _BREAK,
    _CONTINUE,
    _RETURN,
    _IF_STATEMENT,
    _FOR_STATEMENT,
    _FOR_INIT,
    _FOR_EXIT,
    _FOR_UPDATE,
    _WHILE_STATEMENT,
    _DO_STATEMENT,
    _STATEMENT_BLOCK,
    _LPAR,
    _RPAR,
    _LBRAK,
    _RBRAK,
    _LBRACE,
    _RBRACE,
    _ASSIGNOP,
    _UNARYOP,
    _BINARYOP,
    _DOT,
    _TERNARY,
    _COLON,
    _TYPE,
    _SEMI,
    _LITERAL,
    _STRUCT
} asttype_t;

typedef struct astnode {
    struct astnode *left;
    struct astnode *right;
    asttype_t asttype;
    char *text;
    char *type;
    int array_param;
    bool is_const;
    bool is_struct;
    bool is_array;
} astnode_t;

void init_astnode(astnode_t *node, asttype_t asttype);
void init_astnode(astnode_t *node);
void add_astchild(astnode_t *parent, astnode_t *child);
void add_astsibling(astnode_t *node, astnode_t *sibling);
void free_ast(astnode_t *root);

#endif