#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stdio.h>

#include "tokens.h"

#define MAX_LEXEME_SIZE 64
#define MAX_STR_SIZE    1024
#define BUFFER_SIZE     2048

#define CONST_HASH      210709068620
#define STRUCT_HASH     6954031505834
#define FOR_HASH        193491852
#define WHILE_HASH      210732529790
#define DO_HASH         5863320
#define IF_HASH         5863476
#define ELSE_HASH       6385192046
#define BREAK_HASH      210707980106
#define CONTINUE_HASH   7572251799911306
#define RETURN_HASH     6953974653989
#define SWITCH_HASH     6954034739063
#define CASE_HASH       6385108193
#define DEFAULT_HASH    229463065711754

typedef union tokenvalue 
{
    uint8_t c;
    uint32_t i;
    double d;
    char *s;
} tokenvalue_t;

typedef struct token 
{
    tokentype_t type;
    char text[MAX_LEXEME_SIZE];
    int text_size;
    tokenvalue_t value;
    int line_num;
    char *filename;
} token_t;

typedef struct lexer {
    char *filename;
    char *buffer;
    char *cur;
    int line_num;
} lexer_t;

int consume(lexer_t *, token_t *);
token_t next_token(lexer_t *);
void tokenize(char *, FILE *);

#endif