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
    _FUN_PARAM,
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
    _EQ,
    _NEQ, 
    _GEQ, 
    _LEQ, 
    _INCR,
    _DECR,
    _LOG_OR,
    _LOG_AND,
    _BIT_OR,
    _BIT_AND,
    _LOG_NEG,
    _ARITH_NEG,
    _BIT_NEG,
    _ADD_ASSIGN,
    _SUB_ASSIGN,
    _MULT_ASSIGN,
    _DIV_ASSIGN,
    _ADD,
    _SUB,
    _MULT,
    _DIV,
    _DOT,
    _TERNARY,
    _ARR_ACCESS,
    _ARR_DIM,
    _CHAR_LIT,
    _INT_LIT,
    _REAL_LIT,
    _STRING_LIT,
    _VAR
} asttype_t;

typedef union astval {
    char c;
    int i;
    float f;
} astval_t;

typedef struct astnode {
    struct astnode *left;
    struct astnode *right;
    asttype_t type;
    astval_t val;
    char *text;
    bool is_array;
    bool is_const;
    bool is_struct;
} astnode_t;

void init_astnode(astnode_t *node, asttype_t asttype, char *text);

void set_asttext(astnode_t *node, char *text);
void set_astchar(astnode_t *node, char c);
void set_astint(astnode_t *node, int i);
void set_astfloat(astnode_t *node, float f);
void set_aststring(astnode_t *node, char *s);

int add_astchild(astnode_t *parent, astnode_t *child);
int add_astchildleft(astnode_t *parent, astnode_t *child);
int add_astsibling(astnode_t *node, astnode_t *sibling);

void free_ast(astnode_t *root);

int is_inttype(asttype_t asttype);
int is_numerictype(asttype_t asttype);
int is_vartype(asttype_t asttype);

#endif