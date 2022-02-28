

#include "token.h"

/*
 * Prints token to screen in the form seen in function
 * 
 * outfile: file pointer to print the output to
 * tok:     token to print relevant contents of
 *
 * return:  void
 */
void 
print_token(FILE *outfile, token_t *tok) 
{
    fprintf(outfile,"File %s Line %*d Token %*d Text %s\n", 
        tok->filename, 5, tok->line_num, 3, tok->type, tok->text);
}

/*
 * Initializes values of a token_t struct and returns said token_t. Must call 
 * free_token subsequently to free text memory.
 *
 * filename: name of file currently being processed 
 * line_num: current line number within file being processed
 *
 * return: void
 */
void 
init_token(char *filename, int line_num, token_t *tok) 
{
    // allocate 4 chars to start
    tok->text = (char *) malloc((sizeof(char) * MIN_LEXEME_SIZE) + 1);
    tok->text[0] = '\0';
    tok->text_size = 0;
    tok->text_max_size = MIN_LEXEME_SIZE;
    tok->filename = filename;
    tok->line_num = line_num;
}

/*
 * Frees the memory used for the token.
 *
 * tok: token to be freed
 *
 * return: void
 */
void
free_token(token_t *tok)
{
    free(tok->text);
}