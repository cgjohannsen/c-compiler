#include "parse.h"

void 
update_parser(parser_t *parser)
{
    token_t tmp;
    
    if(parser->cur.type == END) {
        return;
    }

    free_token(&parser->cur);
    
    tmp = parser->next;
    next_token(&parser->lex, &parser->next);
    parser->cur = tmp;

    print_token(stderr,&parser->cur);
}  

void parse_expr(parser_t *parser);


void
parse_paramlist(parser_t *parser)
{
    if(parser->cur.type == RPAR) {
        update_parser(parser);
        return;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        return;
    }

    if(parser->cur.type == TYPE) {
        update_parser(parser);
        if(parser->cur.type == IDENT) {
            update_parser(parser);
            if(parser->cur.type == LBRAK) {
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
            }
            if(parser->cur.type == COMMA) {
                update_parser(parser);
            }
            parse_paramlist(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected identifier.");
            parser->status = 0;
            return;
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected type name.");
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
                    "Expected integer literal.");
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
            0, parser->cur.text, "Expected identifier");
        parser->status = 0;
        return;
    }
}


void
parse_forparams(parser_t *parser)
{

    if(parser->cur.type == SEMI) { // empty first param
        update_parser(parser);
    } else { // non-empty first param
        parse_expr(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return;
        }
    }

    if(parser->cur.type == SEMI) { // empty second param
        update_parser(parser);
    } else { // non-empty second param
        parse_expr(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return;
        }
    }

    if(parser->cur.type == RPAR) { // empty third param
        update_parser(parser);
        return;
    } else { // non-empty third param
        parse_expr(parser);
        if(parser->cur.type == RPAR) {
            update_parser(parser);
            return;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return;
        }
    }
}


void
parse_argslist(parser_t *parser)
{
    parse_expr(parser);
    if(parser->cur.type == COMMA) {
        update_parser(parser);
        parse_argslist(parser);
    }
}


void
parse_lvalue(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // with array access
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RBRAK) {
                update_parser(parser);
                return;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ']'");
                parser->status = 0;
                return;
            }
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        return;
    }
}



void
parse_term(parser_t *parser)
{
    if(is_literal(parser->cur.type)) { // literal
        update_parser(parser);
        return;
    } else if(parser->cur.type == IDENT) {
        if(parser->next.type == LPAR) { // function call
            update_parser(parser);
            update_parser(parser);
            if(parser->cur.type == RPAR) { // empty args-list
                update_parser(parser);
                return;
            } else { // non-empty args-list
                parse_argslist(parser);
                if(parser->cur.type == RPAR) {
                    update_parser(parser);
                    return;
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected ')'");
                    parser->status = 0;
                    return;
                }
            }
        } else { // l-value
            parse_lvalue(parser);
            if(is_assignop(parser->cur.type)) { // l-value assignment
                update_parser(parser);
                parse_expr(parser);
                return;
            } else if(parser->cur.type == INCR || parser->cur.type == DECR) {
                // proceeding incr/decr
                update_parser(parser);
                return;
            }
        }
    } else if(parser->cur.type == INCR || parser->cur.type == DECR) { 
        // preceding incr/decr  
        update_parser(parser);
        parse_lvalue(parser);
        return;
    } else if(is_unaryop(parser->cur.type)) { // unary op
        update_parser(parser);
        parse_expr(parser);
        return;
    } else if(parser->cur.type == LPAR) {
        if(parser->next.type == TYPE) { // type cast
            update_parser(parser);
            update_parser(parser);
            if(parser->cur.type == RPAR) {
                update_parser(parser);
                parse_expr(parser);
                return;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected ')'");
                parser->status = 0;
                return;
            }
        } else { // parens
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RPAR) {
                update_parser(parser);
                return;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ')'");
                parser->status = 0;
                return;
            }
        }
    }
}


void
parse_exprprime(parser_t *parser)
{
    if(is_binaryop(parser->cur.type)) {
        update_parser(parser);
        parse_expr(parser);
        return;
    } else if(parser->cur.type == QUEST) {
        update_parser(parser);
        parse_expr(parser);
        if(parser->cur.type == COLON) {
            update_parser(parser);
            parse_expr(parser);
            return;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ':'");
            parser->status = 0;
            return;
        }
    } 
}


void 
parse_expr(parser_t *parser)
{
    parse_term(parser);
    parse_exprprime(parser);
}



void parse_statement(parser_t *parser);

void 
parse_block(parser_t *parser)
{
    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        return;
    }

    parse_statement(parser);
    parse_block(parser);
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
        } else {
            parse_expr(parser);
            if(parser->cur.type == SEMI) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ';'");
                parser->status = 0;
            }
        }
        return;
    } else if(parser->cur.type == IF) {
        update_parser(parser);
        if(parser->cur.type == LPAR) {
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RPAR) {
                update_parser(parser);
                if(parser->cur.type == LBRACE) {
                    update_parser(parser);
                    parse_block(parser);
                } else {
                    parse_statement(parser);
                }
                if(parser->cur.type == ELSE) {
                    update_parser(parser);
                    if(parser->cur.type == LBRACE) {
                        update_parser(parser);
                        parse_block(parser);
                    } else {
                        parse_statement(parser);
                    }
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ')'");
                parser->status = 0;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
        }
        return;
    } else if(parser->cur.type == FOR) {
        update_parser(parser);
        if(parser->cur.type == LPAR) {
            update_parser(parser);
            parse_forparams(parser);
            if(parser->cur.type == LBRACE) {
                update_parser(parser);
                parse_block(parser);
                return;
            } else {
                parse_statement(parser);
                return;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
            return;
        }
    } else if(parser->cur.type == WHILE) {
        update_parser(parser);
        if(parser->cur.type == LPAR) {
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RPAR) {
                update_parser(parser);
                if(parser->cur.type == LBRACE) {
                    update_parser(parser);
                    parse_block(parser);
                } else {
                    parse_statement(parser);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ')'");
                parser->status = 0;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
        }
    } else if(parser->cur.type == DO) {
        update_parser(parser);
        if(parser->cur.type == LBRACE) {
            update_parser(parser);
            parse_block(parser); 
        } else {
            parse_statement(parser);
        }
        if(parser->cur.type == WHILE) {
            update_parser(parser);
            if(parser->cur.type == LPAR) {
                update_parser(parser);
                parse_expr(parser);
                if(parser->cur.type == RPAR) {
                    update_parser(parser);
                    if(parser->cur.type == SEMI) {
                        update_parser(parser);
                        return;
                    } else {
                        print_msg(PARSER_ERR, parser->lex.filename, 
                            parser->lex.line_num, 0, parser->cur.text, 
                            "Expected ';'");
                        parser->status = 0;
                    }
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ')'");
                    parser->status = 0;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected '('");
                parser->status = 0;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected while following do");
            parser->status = 0;
        }
    } else {
        parse_expr(parser);
        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return;
        }
    }
}


void
parse_fundecl(parser_t *parser)
{
    if(parser->status == 0) {
        return;
    }

    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
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
                0, parser->cur.text, "Expected type name.");
            parser->status = 0;
            return;
        }
    }

    if(parser->cur.type == TYPE) {
        update_parser(parser);
        if(parser->cur.type == IDENT) {
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
                0, parser->cur.text, "Expected identifier.");
            parser->status = 0;
            return;
        }   
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected type name.");
        parser->status = 0;
        return;
    }
}


void 
parse_program(parser_t *parser)
{
    print_token(stderr,&parser->cur);
    while(parser->cur.type != END && parser->status > 0) {
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