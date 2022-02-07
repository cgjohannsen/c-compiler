#ifndef TOKENS_H
#define TOKENS_H

typedef enum tokentype {
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

/* Stuff with attributes */

    TYPE      = 301,
    CHAR_LIT  = 302,
    INT_LIT   = 303,
    REAL_LIT  = 304,
    STR_LIT   = 305,
    IDENT     = 306,

/* Symbols */

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

/* Keywords */

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

#endif

