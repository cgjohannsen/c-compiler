#ifndef PARSE_H
#define PARSE_H

#include <stdlib.h>

#include "lexer.h"
#include "ast.h"

typedef struct parser {
    lexer_t lex;
    ast_t ast;
    token_t cur, next;
    int status;
} parser_t;

void parse(char *, FILE *);
void typecheck(void);

#endif