#include "lexer.h"
#include "parse.h"

int parse_program(lexer_t *lex);
int parse_vardecl();
int parse_funproto();
int parse_fundecl();
int parse_funparam();
int parse_fundef();

int 
parse_program(lexer_t *lex)
{
    token_t cur;

    next_token(lex);
    /*

    if(parse_vardecl()) {

    } else if(parse_funproto()) {

    } else if(parse_fundef()) {

    } else {
        // error
    }
    */

    return 1;
}


void 
parse(char *infilename, FILE *outfile)
{
    lexer_t lex;
    
    lex = init_lexer(infilename);
    lex.cur_char = lex.buffer; // set cur to beginning of buffer

    parse_program(&lex);
    

}