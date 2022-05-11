#ifndef AST_H
#define AST_H

#include "token.h"
#include "stdbool.h"

typedef enum nodetype {
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
    _PRE_INCR,
    _PRE_DECR,
    _POST_INCR,
    _POST_DECR,
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
} nodetype_t;

typedef struct ctype {
    char *name;
    bool is_array;
    bool is_const;
    bool is_struct;
} ctype_t;

typedef struct astnode {
    struct astnode *left;
    struct astnode *right;
    char *filename;
    int line_num;
    nodetype_t node_type;
    ctype_t ctype;
    char *text;
    int arr_dim;
    int enter_label;
    int exit_label;
} astnode_t;

astnode_t *init_astnode(nodetype_t asttype, token_t *tok);

void set_asttext(astnode_t *node, char *text);
void set_ctypename(astnode_t *node, char *name);

int add_astchild(astnode_t *parent, astnode_t *child);
int add_astchildleft(astnode_t *parent, astnode_t *child);
int add_astsibling(astnode_t *node, astnode_t *sibling);

void free_ast(astnode_t *root);

bool is_lvalue(astnode_t *node);
bool is_samectype(astnode_t *type1, astnode_t *type2);
bool is_numctype(astnode_t *type);
bool is_intctype(astnode_t *type);

void copy_ctype(astnode_t *source, astnode_t *result);
astnode_t *copy_astnode(astnode_t *node, bool is_top);

void print_ctype(astnode_t *node);
void print_ast(astnode_t *node);

#endif