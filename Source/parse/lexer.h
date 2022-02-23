#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stdio.h>

#include "../util/io.h"

#include "token.h"

typedef struct lexer {
    char *filename;
    FILE *infile;
    char buffer[BUFFER_SIZE];
    char *cur_char;
    token_t cur_tok;
    int line_num;
} lexer_t;

int consume(lexer_t *, token_t *);
void next_token(lexer_t *);
lexer_t init_lexer(char *);
void tokenize(char *, FILE *);

#endif