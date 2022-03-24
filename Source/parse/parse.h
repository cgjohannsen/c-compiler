#ifndef PARSE_H
#define PARSE_H

#include <stdlib.h>

#include "lexer.h"
#include "ast.h"
#include "symtable.h"

typedef struct parser {
    lexer_t lex;
    symtable_t table;
    ast_t ast;
    token_t cur, next;
    int status;
} parser_t;

void parse(char *, FILE *);
void typecheck(void);

#endif