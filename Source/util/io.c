#include <stdio.h>
#include <stdlib.h>

#include "io.h"

/*
 * Prints the content of the message to stderr along with filename, line number,
 * and optional character if relevant.
 *
 * code:     type of message i.e. warning/error and from what part of compiler
 * filename: name of relevant file
 * line_num: line number of relevant file
 * c:        relevant character (optional)
 * msg:      message to print
 *
 * return: void
 */
void 
print_msg(msgcode_t code, char *filename, int line_num, char c, char *msg)
{
    switch(code) {
        case FILE_ERR:
        {
            fprintf(stderr, "File error in file %s\n\t%s\n",
                filename, msg);
        }
        break;
        case LEXER_ERR:
        {
            fprintf(stderr, "Lexer error in file %s line %d near text" 
                "%c\n\t%s\n", filename, line_num, c, msg);
        }
        break;
        case LEXER_WRN:
        {
            fprintf(stderr, "Lexer warning in file %s line %d near text" 
                "%c\n\t%s\n", filename, line_num, c, msg);
        }
        break;
        case PARSER_ERR:
        {
            fprintf(stderr, "Parser error in file %s line %d near text" 
                "%c\n\t%s\n", filename, line_num, c, msg);
        }
        break;
        default:
        {
            fprintf(stderr, "Error in file %s line %d\n\t%s\n", 
                filename, line_num, msg);
        }
    }
}

/*
 * Opens filename, returns file ptr to said file. Prints error if file was not 
 * opened successfully
 *
 * filename: file to open and return file ptr to
 *
 * return: pointer to buffer that holds filename contents
 *         NULL if file failed to open
 */
FILE *
open_file(char *filename)
{
    FILE *fp;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        print_msg(FILE_ERR, filename, 0, 0, "Could not open file");
        return NULL;
    }

    return fp;
}

/*
 * Reads the contents of the file at fp into the buffer. Places the value 0 at
 * the EOF character.
 *
 * fp:     file ptr to file to read from
 * buffer: buffer to fill
 *
 * return: 1
 */
int
refill_buffer(FILE *fp, char *buffer)
{
    size_t bytes_read;

    bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fp);

    if(bytes_read != BUFFER_SIZE) { // EOF in buffer now
        buffer[bytes_read] = 0; // set EOF char to 0
    }

    return 1;
}
