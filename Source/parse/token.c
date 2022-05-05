#include "../util/io.h"
#include "token.h"

int 
is_unaryop(tokentype_t type)
{
    return type == MINUS || type == BANG || type == TILDE || type == INCR || type == DECR;
}


int 
is_binaryop(tokentype_t type)
{
    return type == EQ || type == NEQ || type == GT || type == GEQ || 
        type == LT || type == LEQ || type == PLUS || type == MINUS || 
        type == STAR || type == SLASH || type == MOD || type == PIPE ||
        type == AMP || type == DPIPE || type == DAMP;
}

int 
is_literal(tokentype_t type)
{
    return type == CHAR_LIT || type == INT_LIT || type == REAL_LIT ||
        type == STR_LIT;
}

int 
is_assignop(tokentype_t type)
{
    return type == ASSIGN || type == PLUSASSIGN || type == MINUSASSIGN ||
        type == STARASSIGN || type == SLASHASSIGN;
}

int 
is_typeorqualifier(tokentype_t type)
{
    return type == CONST || type == STRUCT || type == TYPE;
}



/**
 * Prints token to screen in the form seen in function
 * 
 * @param tok     token to print relevant contents of
 *
 * @return  void
 */
void 
print_token(token_t *tok) 
{
    int i = 0;
    while(i < tok->include_depth+tok->macro_depth) {
        fprintf(outfile, "..");
        i++;
    }

    if(tok->tok_type == MACRO && tok->macro_name != NULL) {
        fprintf(outfile, "Macro %s macro expansion\n", tok->macro_name);
    } else if(tok->tok_type == MACRO) {
        fprintf(outfile, "File %s Line %*d macro expansion\n", 
            tok->filename, 5, tok->line_num);
    } else if(tok->macro_name != NULL) {
        fprintf(outfile, "Macro %s Token %*d Text %s\n",
            tok->macro_name, 3, tok->tok_type, tok->text);
    } else {
        fprintf(outfile, "File %s Line %*d Token %*d Text %s\n", 
            tok->filename, 5, tok->line_num, 3, tok->tok_type, tok->text);
    }
}

/**
 * Initializes values of a token_t struct and returns said token_t. Must call 
 * free_token subsequently to free text memory.
 *
 * @param filename name of file currently being processed 
 * @param line_num current line number within file being processed
 *
 * @return 
 */
token_t * 
init_token(char *filename, int line_num, int include_depth, int macro_depth) 
{
    token_t *tok = (token_t *) malloc(sizeof(token_t));

    // allocate 4 chars to start
    tok->text = (char *) malloc((sizeof(char) * MIN_LEXEME_SIZE) + 1);
    tok->text[0] = '\0';
    tok->text_size = 0;
    tok->text_max_size = MIN_LEXEME_SIZE;
    tok->filename = filename;
    tok->line_num = line_num;
    tok->include_depth = include_depth;
    tok->macro_depth = macro_depth;
    tok->macro_name = NULL;

    return tok;
}

/**
 * Frees the memory used for the token.
 *
 * @param tok token to be freed
 *
 * @return void
 */
void
free_token(token_t *tok)
{
    free(tok->text);
    free(tok);
}