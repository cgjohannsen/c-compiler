#ifndef PARSE_H
#define PARSE_H

#include "lexer.h"
#include "ast.h"

typedef struct parser {
    lexer_t lex;
    ast_t ast;
} parser_t;

void parse(char *, FILE *);

#endif