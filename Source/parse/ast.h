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
    _FUN_PARAMS,
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
    _FOR_BODY,
    _WHILE_STATEMENT,
    _WHILE_COND,
    _WHILE_BODY,
    _DO_STATEMENT,
    _DO_COND,
    _DO_BODY,
    _STATEMENT_BLOCK,
    _EQ,
    _NEQ, 
    _GT,
    _LT,
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
    _ASSIGN,
    _ADD_ASSIGN,
    _SUB_ASSIGN,
    _MULT_ASSIGN,
    _DIV_ASSIGN,
    _FUN_ARGS,
    _FUN_CALL,
    _ADD,
    _SUB,
    _MULT,
    _DIV,
    _MOD,
    _DOT,
    _ITE,
    _ARR_ACCESS,
    _STRUCT_ACCESS,
    _ARR_DIM,
    _CHAR_LIT,
    _INT_LIT,
    _REAL_LIT,
    _STR_LIT,
    _VAR, 
    _TYPE
} asttype_t;

typedef enum exprtype {
    __CHAR,
    __INT,
    __REAL,
    __STRING,
    __STRUCT
} exprtype_t;

typedef union astval {
    char c;
    int i;
    float f;
} astval_t;

typedef struct astnode {
    struct astnode *left;
    struct astnode *right;
    char *filename;
    int line_num;
    asttype_t type;
    astval_t val;
    exprtype_t exprtype;
    char *text;
    int arr_dim;
    bool is_array;
    bool is_const;
    bool is_struct;
} astnode_t;

astnode_t *init_astnode(asttype_t asttype, token_t *tok);

void set_asttext(astnode_t *node, char *text);
void set_astchar(astnode_t *node, char c);
void set_astint(astnode_t *node, int i);
void set_astfloat(astnode_t *node, float f);
void set_aststring(astnode_t *node, char *s);

int add_astchild(astnode_t *parent, astnode_t *child);
int add_astchildleft(astnode_t *parent, astnode_t *child);
int add_astsibling(astnode_t *node, astnode_t *sibling);

void free_ast(astnode_t *root);

bool is_lvalue(astnode_t *node);

#endif