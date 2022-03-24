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
}  


// forward declarations
astnode_t parse_statement(parser_t *parser);
astnode_t parse_expr(parser_t *parser);


astnode_t
parse_type(parser_t *parser)
{
    if(parser->cur.type == CONST) { // 'const'
        update_parser(parser);
        if(parser->cur.type == TYPE) { // 'const' type
            update_parser(parser);
            return;
        } else if(parser->cur.type == STRUCT) { // 'const' 'struct'
            update_parser(parser);
            if(parser->cur.type == IDENT) { // 'const' 'struct' ident
                update_parser(parser);
                return;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected identifier.");
                parser->status = 0;
                return;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected type or 'struct'.");
            parser->status = 0;
            return;
        }
    }   

    if(parser->cur.type == TYPE) { // type
        update_parser(parser);
        if(parser->cur.type == CONST) { // type 'const'
            update_parser(parser);
            return;
        }
        return;
    }

    if(parser->cur.type == STRUCT) { // 'struct'
        update_parser(parser);
        if(parser->cur.type == IDENT) { // 'struct' ident
            update_parser(parser);
            if(parser->cur.type == CONST) { // 'struct' ident 'const'
                update_parser(parser);
                return;
            }
            return;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, 
                parser->lex.line_num, 0, parser->cur.text, 
                "Expected identifier.");
            parser->status = 0;
            return;
        }
    }
}


astnode_t
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

    if(is_typeorqualifier(parser->cur.type)) {
        parse_type(parser);
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


astnode_t
parse_varlist(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
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
        } else if(parser->cur.type == ASSIGN) { // initialization
            update_parser(parser);
            parse_expr(parser);
        }
        if(parser->cur.type == COMMA) { // continue declaring vars
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

astnode_t
parse_varlistnoinit(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
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
        if(parser->cur.type == COMMA) { // continue declaring vars
            update_parser(parser);
            parse_varlistnoinit(parser);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        return;
    }
}


astnode_t
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


astnode_t
parse_argslist(parser_t *parser)
{
    parse_expr(parser);
    if(parser->cur.type == COMMA) {
        update_parser(parser);
        parse_argslist(parser);
    }
}


astnode_t
parse_lvalue(parser_t *parser)
{
    if(parser->cur.type == IDENT) {
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // with array access
            update_parser(parser);
            parse_expr(parser);
            if(parser->cur.type == RBRAK) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ']'");
                parser->status = 0;
                return;
            }
        }
        if(parser->cur.type == DOT) { // struct access
            update_parser(parser);
            parse_lvalue(parser);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        return;
    }
}



astnode_t
parse_term(parser_t *parser)
{
    if(is_literal(parser->cur.type)) { // literal
        update_parser(parser);
        return;
    } else if(parser->cur.type == IDENT) {
        if(parser->next.type == LPAR) { // function call
            update_parser(parser);
            update_parser(parser);
            if(parser->cur.type == RPAR) { // Ident '(' ')'
                update_parser(parser);
                return;
            } else { // Ident '(' args-list ')'
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
            if(is_assignop(parser->cur.type)) { // l-value AssignOp expr
                update_parser(parser);
                parse_expr(parser);
                return;
            } else if(parser->cur.type == INCR || parser->cur.type == DECR) {
                //  l-value ('++' | '--') 
                update_parser(parser);
                return;
            }
        }
    } else if(parser->cur.type == INCR || parser->cur.type == DECR) { 
        // ('++' | '--') l-value    
        update_parser(parser);
        parse_lvalue(parser);
        return;
    } else if(is_unaryop(parser->cur.type)) { // UnaryOp expr 
        update_parser(parser);
        parse_expr(parser);
        return;
    } else if(parser->cur.type == LPAR) {
        if(parser->next.type == TYPE || parser->next.type == CONST) { 
            // '(' type ')' expr 
            parse_type(parser);
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
        } else { // '(' expr ')' 
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


astnode_t
parse_exprprime(parser_t *parser)
{
    if(is_binaryop(parser->cur.type)) { // BinaryOp expr
        update_parser(parser);
        parse_expr(parser);
        return;
    } else if(parser->cur.type == QUEST) { // '?' expr ':' expr
        update_parser(parser);
        parse_expr(parser);
        if(parser->cur.type == COLON) { // ':' expr
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


astnode_t 
parse_expr(parser_t *parser)
{
    parse_term(parser);
    parse_exprprime(parser);
}




astnode_t 
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


astnode_t
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



astnode_t
parse_typedecl(parser_t *parser)
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

    parse_type(parser);
    parse_varlistnoinit(parser);

    if(parser->cur.type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
        parser->status = 0;
        return;
    }

    parse_typedecl(parser);

}


astnode_t
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

    if(parser->cur.type == STRUCT) { // 'struct' ident
        update_parser(parser);
        if(parser->cur.type == IDENT) {
            update_parser(parser);
            if(parser->cur.type == LBRACE) { // type-decl
                update_parser(parser);
                parse_typedecl(parser);
                if(parser->cur.type == SEMI) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ';'.");
                    parser->status = 0;
                    return;
                }
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, 
                parser->lex.line_num, 0, parser->cur.text, 
                "Expected identifier.");
            parser->status = 0;
            return;
        }
    }

    if(is_typeorqualifier(parser->cur.type)) {
        parse_type(parser);
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


astnode_t 
parse_global(parser_t *parser) 
{
    if(is_typeorqualifier(parser->cur.type)) {

        if(parser->cur.type == STRUCT) { // 'struct' ident
            update_parser(parser);
            if(parser->cur.type == IDENT) {
                update_parser(parser);
                if(parser->cur.type == LBRACE) { // type-decl
                    update_parser(parser);
                    parse_typedecl(parser);
                    if(parser->cur.type == SEMI) {
                        update_parser(parser);
                        return;
                    } else {
                        print_msg(PARSER_ERR, parser->lex.filename, 
                            parser->lex.line_num, 0, parser->cur.text, 
                            "Expected ';'.");
                        parser->status = 0;
                        return;
                    }
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected identifier.");
                parser->status = 0;
                return;
            }
        } else {
            parse_type(parser);
        }

        if(parser->cur.type == IDENT) {
            if(parser->next.type == LPAR) { // fun-decl or fun-proto
                update_parser(parser);
                update_parser(parser);
                parse_paramlist(parser);
                if(parser->cur.type == SEMI) { // fun-proto
                    update_parser(parser);
                    return;
                } else if(parser->cur.type == LBRACE) { // fun-decl
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
                0, parser->cur.text, "Expected identifier or type declaration.");
            parser->status = 0;
            return;
        }   
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected type name 1.");
        parser->status = 0;
        return;
    }
}


astnode_t 
parse_program(parser_t *parser)
{
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

astnode_t 
parse(char *infilename, FILE *outfile)
{
    parser_t parser;
    init_parser(infilename, &parser);
    parse_program(&parser);
    if(parser.status > 0) {
        fprintf(outfile,"File %s is syntactically correct.\n",infilename);
        exit(1);
    }
    return parser.ast;
}