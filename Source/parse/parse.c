#include "parse.h"

void 
update_parser(parser_t *parser)
{
    token_t tmp;
    
    free_token(&parser->cur);

    if(parser->next.type == END) {
        return;
    }

    tmp = parser->next;
    next_token(&parser->lex, &parser->next);
    parser->cur = tmp;

}  


void
parse_paramlist(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) {
            update_parser(parser);
            if(parser->cur.type == RBRAK) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ']'");
                parser->status = 0;
                return;
            }
        }
        if(parser->cur.type == COMMA) {
            update_parser(parser);
            parse_paramlist(parser);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, 
            parser->lex.line_num, 0, parser->cur.text, "Expected IDENT");
        parser->status = 0;
        return;
    }
}


void
parse_varlist(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) {
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                update_parser(parser);
                if(parser->cur.type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ']'");
                    parser->status = 0;
                    return;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected INT_LIT");
                parser->status = 0;
                return;
            }
        }
        if(parser->cur.type == COMMA) {
            update_parser(parser);
            parse_varlist(parser);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected IDENT");
        parser->status = 0;
        return;
    }
}


void
parse_expr(parser_t *parser)
{
    if(parser->cur.type == INT_LIT) {
        update_parser(parser);
        return;
    }
}


void 
parse_block(parser_t *parser)
{
    return;
}



void
parse_statement(parser_t *parser)
{
    if(parser->cur.type == SEMI) {
        update_parser(parser);
        return;
    } else if(parser->cur.type == BREAK) {
        update_parser(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return;
        }
    } else if(parser->cur.type == CONTINUE) {
        update_parser(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return;
        }
    } else if(parser->cur.type == RETURN) {
        update_parser(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return;
        } else {
            parse_expr(parser);
            return;
        }
    } else if(parser->cur.type == IF) {
        update_parser(parser);
        if(parser->cur.type == LPAR) {
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RPAR) {
                update_parser(parser);
                if(parser->cur.type == LBRAK) {
                    update_parser(parser);
                    parse_block(parser);
                } else {

                }
                if(parser->cur.type == ELSE) {
                    update_parser(parser);
                    if(parser->cur.type == LBRAK) {
                        update_parser(parser);
                        parse_block(parser);
                    } else {

                    }
                }
            } else {
                // error -- expected ')'
            }
        } else {
            // error -- expecgted '('
        }
    } else if(parser->cur.type == FOR) {
        
    } else if(parser->cur.type == WHILE) {
        
    } else if(parser->cur.type == DO) {
        
    } else {
        parse_expr(parser);
    }
}


void
parse_fundecl(parser_t *parser)
{
    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return;
    }

    if(parser->cur.type == TYPE) {
        update_parser(parser);
        parse_varlist(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return;
        }
    } else {
        parse_statement(parser);
    }

    parse_fundecl(parser);
}


void 
parse_global(parser_t *parser) 
{
    if(parser->cur.type == CONST) { // var decl
        update_parser(parser);
        if(parser->cur.type == TYPE) {
            update_parser(parser);
            parse_varlist(parser);
            if(parser->cur.type == SEMI) {
                update_parser(parser);
                return;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, parser->lex.
                    line_num, 0, parser->cur.text, "Expected ';'");
                parser->status = 0;
                return;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected TYPE");
            parser->status = 0;
            return;
        }
    }

    if(parser->cur.type == TYPE) {
        update_parser(parser);
        if(parser->next.type == IDENT) {
            if(parser->next.type == LPAR) { // function decl/proto
                update_parser(parser);
                update_parser(parser);
                parse_paramlist(parser);
                if(parser->cur.type == SEMI) { // function prototype
                    update_parser(parser);
                    return;
                } else if(parser->cur.type == LBRACE) { // function decl
                    update_parser(parser);
                    parse_fundecl(parser);
                    return;
                }
            } else { // var decl
                parse_varlist(parser);
                if(parser->cur.type == SEMI) {
                    update_parser(parser);
                    return;
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ';'");
                    parser->status = 0;
                    return;
                }
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected IDENT");
            parser->status = 0;
            return;
        }   
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected TYPE");
        parser->status = 0;
        return;
    }
}


void 
parse_program(parser_t *parser)
{
    while(parser->cur.type != END && parser->next.type != END) {
        parse_global(parser);
    }
}


void
init_parser(char *filename, parser_t *parser)
{
    init_lexer(filename, &parser->lex);
    
    parser->status = 1;

    next_token(&parser->lex, &parser->cur);
    if(parser->cur.type != END) {
        next_token(&parser->lex, &parser->next);
    }
}

void 
parse(char *infilename, FILE *outfile)
{
    parser_t parser;
    init_parser(infilename, &parser);
    parse_program(&parser);
    if(parser.status > 0) {
        fprintf(outfile,"File %s is syntactically correct.\n",infilename);
    }
}