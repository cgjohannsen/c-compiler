#include "parse.h"

int parse_program(lexer_t *lex, ast_t *ast);
int parse_vardecl(lexer_t *lex, ast_t *ast);
int parse_funproto(lexer_t *lex, ast_t *ast);
int parse_fundecl(lexer_t *lex, ast_t *ast);
int parse_funparam(lexer_t *lex, ast_t *ast);
int parse_fundef(lexer_t *lex, ast_t *ast);

int 
parse_program(lexer_t *lex, ast_t *ast)
{
    token_t cur;

    next_token(lex);

    if(parse_vardecl(lex,ast)) {

    } else if(parse_funproto(lex,ast)) {

    } else if(parse_fundef(lex,ast)) {

    } else if(lex->cur_tok.type == END) {
    
    } else {
        // error
    }

    return 1;
}


void 
parse(char *infilename, FILE *outfile)
{
    lexer_t lex;
    ast_t ast;
    
    lex = init_lexer(infilename);
    lex.cur_char = lex.buffer; // set cur to beginning of buffer

    parse_program(&lex,&ast);
    

}