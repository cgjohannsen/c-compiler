#include "parse.h"

void 
update_parser(parser_t *parser)
{
    token_t *tmp;

    tmp = &parser->next;
    parser->next = next_token(&parser->lexer);
    parser->cur = tmp;
}   


int
check_type(parser_t *parser, tokentype_t type)
{
    return parser->status && parser->cur->type == type;
}


void
parse_paramlist(parser_t *parser)
{
    if(parser->cur->type == IDENT) {
        update_parser(parser);
        if(parser->cur->type == LBRAK) {
            update_parser(parser);
            if(parser->cur->type == RBRAK) {
                update_parser(parser);
            } else {
                // error -- expected ']'
            }
        }
        if(parser->cur->type == COMMA) {
            update_parser(parser);
            return parse_paramlist(parser);
        }
    } else {
        // error -- expected ident
    }
}


void
parse_varlist(parser_t *parser)
{
    if(parser->cur->type == IDENT) {
        update_parser(parser);
        if(parser->cur->type == LBRAK) {
            update_parser(parser);
            if(parser->cur->type == INT_LIT) {
                update_parser(parser);
                if(parser->cur->type == RBRAK) {
                    update_parser(parser);
                } else {
                    // error -- expected ']'
                }
            } else {
                // error -- expected int
            }
        }
        if(parser->cur->type == COMMA) {
            update_parser(parser);
            return parse_varlist(parser);
        }
    } else {
        // error -- expected ident
    }
}


void
parse_expr(parser_t parser)
{
    
}


void
parse_statement(parser_t parser)
{
    if(parser->cur->type == SEMI) {
        update_parser(parser);
        return;
    } else if(parser->cur->type == BREAK) {
        update_parser(parser);
        if(parser->cur->type == SEMI) {
            update_parser(parser);
            return;
        }
    } else if(parser->cur->type == CONTINUE) {
        update_parser(parser);
        if(parser->cur->type == SEMI) {
            update_parser(parser);
            return;
        }
    } else if(parser->cur->type == RETURN) {
        update_parser(parser);
        if(parser->cur->type == SEMI) {
            update_parser(parser);
            return;
        } else {
            parse_expr(parser);
            return;
        }
    } else if(parser->cur->type == IF) {
        update_parser(parser);
        if(parser->cur->type == LPAR) {
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur->type == RPAR) {
                
            } else {
                // error -- expected ')'
            }
        }
    } else if(parser->cur->type == FOR) {
        
    } else if(parser->cur->type == WHILE) {
        
    } else if(parser->cur->type == DO) {
        
    } else {
        parse_expr(parser);
    }
}


void
parse_fundecl(parser_t parser)
{
    if(parser->cur->type == RBRACE) {
        update_parser(parser);
        return;
    }

    if(parser->cur->type == TYPE) {
        update_parser(parser);
        parse_varlist(parser);
        if(parser->cur->type == SEMI) {
            update_parser(parser);
        } else {
            // error -- expected ';'
        }
    } else {
        parse_statement(parser);
    }

    parse_fundecl(parser);
}


void 
parse_global(parser_t *parser)
{
    if(parser->cur->type == CONST) {
        // var decl
        update_parser(parser);
        if(parser->cur->type == TYPE) {
            update_parser(parser);
            parse_varlist()
            if(parser->cur->type == SEMI) {
                update_parser(parser);
                return;
            } else {
                // error -- expected semi
            }
        } else {
            // error -- expected type
        }
    }

    if(parser->cur->type == TYPE) {
        update_parser(parser);
        if(parser->cur->type == IDENT) {
            update_parser(parser);
            if(parser->cur->type == LPAR) {
                // function decl/proto
                update_parser(parser);
                parse_paramlist(parser);
                if(parser->cur->type == SEMI) {
                    // function prototype
                    update_parser(parser);
                    return;
                } else if(parser->cur->type == LBRACE) {
                    // function decl
                    update_parser(parser);
                    parse_fundecl(parser);
                    return;
                }
            } else {
                // var decl
                parse_varlist()
                if(parser->cur->type == SEMI) {
                    update_parser(parser);
                    return;
                } else {
                    // error -- expected semi
                }
            }
        } else {
            // error -- expected ident
        }   
    } else {
        // error -- expected type 
    }

    return 1;
}


void 
parse_program(parser_t *parser)
{
    while(parser->next->type != END) {
        parse_global(parser);
    }
}


parser_t
init_parser(char *filename)
{
    lexer_t lexer;
    ast_t ast;

    lexer = init_lexer(filename);
    lexer.cur = lexer.buffer; // set cur to beginning of buffer

    parser_t parser = {
        .lex = &lexer,
        .ast = &ast,
        .status = 1
    }

    parser.cur = &next_token(parser.lex);
    if(parser.cur->type != END) {
        parser.next = &next_token(parser.lex);
    }

    return parser;
}

void 
parse(char *infilename, FILE *outfile)
{
    parser_t parser = init_parser(infilename);
    parse_program(&parser);
}