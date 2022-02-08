#include <stdio.h>
#include <stdlib.h>

#include "io.h"

/*
 *
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
            fprintf(stderr, "Lexer error in file %s line %d near text %c\n\t%s\n", 
                filename, line_num, c, msg);
        }
        break;
        case LEXER_WRN:
        {
            fprintf(stderr, "Lexer warning in file %s line %d near text %c\n\t%s\n", 
                filename, line_num, c, msg);
        }
        break;
        case PARSER_ERR:
        default:
        {
            fprintf(stderr, "Error in file %s line %d\n\t%s\n", 
                filename, line_num, msg);
        }
    }
}


/*
 * Opens filename, returns buffer of entire file. Must free the buffer that's returned after 
 * calling read_file. If filename is bigger than MAX_FILE_SIZE, return NULL.
 * Also replaces last element of buffer with 0 so that we know when to stop reading from file.
 *
 * filename: file to open and return contents of
 *
 * returns: pointer to buffer that holds filename contents
 */
char *
read_file(char *filename)
{
    FILE *fp;
    long file_size;
    char *buffer;
    size_t bytes_read;

    fp = fopen(filename, "r");
    if(fp == NULL) {
        print_msg(FILE_ERR, filename, 0, 0, "Could not open file");
        return NULL;
    }

    /* get file size */
    fseek (fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if(file_size > MAX_FILE_SIZE) {
        print_msg(FILE_ERR, filename, 0, 0, "File size too big");
        return NULL;
    }

    /* allocate memory to contain the whole file + '\0' at end of buffer */
    buffer = (char*) malloc((sizeof(char)*file_size)+1);
    if (buffer == NULL) {
        print_msg(FILE_ERR, filename, 0, 0, "Could not allocate memory for file");
        return NULL;
    }

    /* copy the file into the buffer */
    bytes_read = fread(buffer, 1, file_size, fp);
    if (bytes_read != file_size) {
        print_msg(FILE_ERR, filename, 0, 0, "Could not read entire file contents");
        return NULL;
    }

    /* set last char of buffer to 0 */
    buffer[file_size] = 0;

    fclose(fp);
    return buffer;
}
