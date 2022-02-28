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
    char *cur;
    int line_num;
} lexer_t;

int consume(lexer_t *, token_t *);
void next_token(lexer_t *, token_t *);
void init_lexer(char *, lexer_t *);
void tokenize(char *, FILE *);

#endif