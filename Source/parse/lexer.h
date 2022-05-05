#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stdio.h>

#include "../util/io.h"

#include "token.h"

typedef struct macro {
    struct macro *next;
    char *name;
    char *text;
    char *tmp; // for expansion -- where to set lexer back to afterwards
    struct macro *tmp_macro; // for expansion, when expanding to another macro
} macro_t;

typedef struct includes {
    struct includes *next;
    struct includes *prev;
    char *filename;
} includes_t;

typedef struct lexer {
    char *filename;
    FILE *infile;
    char buffer[BUFFER_SIZE];
    char *cur;
    int line_num;
    includes_t *includes;
    int include_depth; // include depth
    int macro_depth; // macro depth
    macro_t *macros; // list of currently defined macros
    macro_t *cur_macro;
} lexer_t;

int consume(lexer_t *, token_t *);
token_t * next_token(lexer_t *);

int add_macro(lexer_t *, char *, char *);
void free_macros(macro_t *);

lexer_t *init_lexer(char *, int, int, macro_t *, includes_t *);
void free_lexer(lexer_t *);

void tokenize(char *);

#endif