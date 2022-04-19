#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LEXEME_SIZE   64
#define MIN_LEXEME_SIZE   4
#define MAX_STR_SIZE      1024

#define VOID_HASH       6385805911
#define CHAR_HASH       6385115235
#define INT_HASH        193495088
#define FLOAT_HASH      210712519067

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

typedef enum tokentype 
{
    END     = 0,
    COMMA   = ',',
    DOT     = '.',
    SEMI    = ';',
    LPAR    = '(',
    RPAR    = ')',
    LBRAK   = '[',
    RBRAK   = ']',
    LBRACE  = '{',
    RBRACE  = '}',
    GT      = '>',
    LT      = '<',
    ASSIGN  = '=',
    PLUS    = '+',
    MINUS   = '-',
    STAR    = '*',
    SLASH   = '/',
    MOD     = '%',
    COLON   = ':',
    QUEST   = '?',
    TILDE   = '~',
    PIPE    = '|',
    AMP     = '&',
    BANG    = '!',

    // Stuff with attributes

    TYPE      = 301,
    CHAR_LIT  = 302,
    INT_LIT   = 303,
    REAL_LIT  = 304,
    STR_LIT   = 305,
    IDENT     = 306,

    // Symbols

    EQ        = 351,
    NEQ       = 352,
    GEQ       = 353,
    LEQ       = 354,
    INCR      = 355,
    DECR      = 356,
    DPIPE     = 357,
    DAMP      = 358,

    PLUSASSIGN    = 361,
    MINUSASSIGN   = 362,
    STARASSIGN    = 363,
    SLASHASSIGN   = 364,

    // Keywords

    CONST     = 401,
    STRUCT    = 402,
    FOR       = 403,
    WHILE     = 404,
    DO        = 405,
    IF        = 406,
    ELSE      = 407,
    BREAK     = 408,
    CONTINUE  = 409,
    RETURN    = 410,
    SWITCH    = 411,
    CASE      = 412,
    DEFAULT   = 413

} tokentype_t;

typedef union tokenvalue 
{
    uint8_t c;
    uint32_t i;
    double d;
    char *s;
} tokenvalue_t;

typedef struct token 
{
    tokentype_t tok_type;
    char *text;
    int text_size;
    int text_max_size;
    tokenvalue_t value;
    int line_num;
    char *filename;
    FILE *file;
} token_t;

int is_unaryop(tokentype_t);
int is_binaryop(tokentype_t);
int is_literal(tokentype_t);
int is_assignop(tokentype_t);
int is_typeorqualifier(tokentype_t);

void print_token(token_t *);
token_t * init_token(char *, int); 
void free_token(token_t *);

#endif

