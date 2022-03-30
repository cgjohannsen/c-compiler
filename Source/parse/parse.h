#ifndef PARSE_H
#define PARSE_H

#include <stdlib.h>

#include "lexer.h"
#include "ast.h"

typedef struct parser {
    lexer_t *lex;
    token_t *cur, *next, *nextnext;
    int status;
} parser_t;

astnode_t *parse_program(parser_t *parser);
void init_parser(char *filename, parser_t *parser);
void parse(char *filename);

#endif