#ifndef IO_H
#define IO_H

#define MAX_FILE_SIZE 5000000 /* 5MB */

typedef enum msgcode {
    FILE_ERR,
    LEXER_ERR,
    LEXER_WRN,
    PARSER_ERR,
    PARSER_WRN
} msgcode_t;

void print_msg(msgcode_t, char *, int, char, char *);
char *read_file(char *);
void delete_buffer(char *);

#endif