

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
 * return: an initilialized token_t struct
 */
token_t
init_token(char *filename, int line_num) 
{
    token_t tok = {
        // allocate 4 chars to start
        .text = (char *) malloc((sizeof(char) * MIN_LEXEME_SIZE) + 1), 
        .text_size = 0,
        .text_max_size = MIN_LEXEME_SIZE,
        .filename = filename,
        .line_num = line_num
    };
    tok.text[0] = '\0';
    return tok;
}

/*
 * Frees the memory used for the text attribute of the token.
 *
 * tok: token to be freed
 *
 * return: 1
 */
int
free_token(token_t *tok)
{
    free(tok->text);
    return 1;
}