#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum asttype {
    _PROGRAM,
    _GLOBAL,
    _VAR_DECL,
    _VAR_LIST,
    _TYPE_DECL,
    _FUN_PROTO,
    _FUN_DECL,
    _PARAM_LIST,
    _FUN_DEF,
    _STATEMENT,
    _EXPR,
    _BREAK,
    _CONTINUE,
    _RETURN,
    _IF_STATEMENT,
    _FOR_STATEMENT,
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
    _QUEST,
    _COLON,
    _TYPE,
    _SEMI,
    _VOID,
    _CHAR,
    _INTEGER,
    _FLOAT,
    _STRING
} asttype_t;

typedef struct astnode {
    asttype_t type;
    token_t *tok;
    struct astnode *left;
    struct astnode *right;
} astnode_t;

#endif