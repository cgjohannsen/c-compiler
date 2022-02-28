#ifndef IO_H
#define IO_H

#include <stdio.h>

#define MAX_FILE_SIZE 5000000 // 5MB
#define BUFFER_SIZE   1000    // 1kb 

typedef enum msgcode {
    FILE_ERR,
    LEXER_ERR,
    LEXER_WRN,
    PARSER_ERR,
    PARSER_WRN
} msgcode_t;

void print_msg(msgcode_t, char *, int, char, char *, char *);
FILE *open_file(char *);
int refill_buffer(FILE *, char *);

#endif