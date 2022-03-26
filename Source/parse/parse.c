#include "parse.h"

/**
 *
 */
void 
update_parser(parser_t *parser)
{
    token_t tmp;
    
    if(parser->cur.type == END) {
        return;
    }

    //free_token(&parser->cur);
    
    tmp = parser->next;
    next_token(&parser->lex, &parser->next);
    parser->cur = tmp;
}  


// forward declarations
astnode_t parse_statement(parser_t *parser);
astnode_t parse_expr(parser_t *parser);



/**
 *
 */
void
parse_type(parser_t *parser, astnode_t *node)
{
    node->is_const = false;
    node->is_struct = false;

    if(parser->cur.type == CONST) { // 'const'
        node->is_const = true;
        update_parser(parser);
        if(parser->cur.type == TYPE) { // 'const' type
            node->type = parser->cur.text;
            update_parser(parser);
            return;
        } else if(parser->cur.type == STRUCT) { // 'const' 'struct'
            update_parser(parser);
            if(parser->cur.type == IDENT) { // 'const' 'struct' ident
                node->is_struct = true;
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
        node->type = parser->cur.text;
        update_parser(parser);
        if(parser->cur.type == CONST) { // type 'const'
            node->is_const = true;
            update_parser(parser);
            return;
        }
        return;
    }

    if(parser->cur.type == STRUCT) { // 'struct'
        node->is_struct = true;
        update_parser(parser);
        if(parser->cur.type == IDENT) { // 'struct' ident
            node->type = parser->cur.text;
            update_parser(parser);
            if(parser->cur.type == CONST) { // 'struct' ident 'const'
                node->is_const = true;
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



/**
 *
 */
astnode_t *
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

    astnode_t var;

    if(is_typeorqualifier(parser->cur.type)) {
        parse_type(parser, &var);
        if(parser->cur.type == IDENT) {
            var.text = parser->cur.text;
            update_parser(parser);
            if(parser->cur.type == LBRAK) {
                var.is_array = true;
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
                if(!is_typeorqualifier(parser->cur.type)) {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                              parser->lex.line_num, 0, parser->cur.text, 
                              "Expected type name.");
                    parser->status = 0;
                    return;
                }
            }
            add_astchild(parent, var);
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



/**
 *
 */
astnode_t *
parse_varlist(parser_t *parser)
{
    astnode_t *var;
    init_astnode(var, _VAR);

    if(parser->cur.type == IDENT) {
        var->text = parser->cur.text;
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                astnode_t *arr_dim;
                init_astnode(arr_dim, _INT_LIT);
                arr_dim->text = parser->cur.text;
                add_astchild(var, arr_dim);

                update_parser(parser);
                if(parser->cur.type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ']'");
                    parser->status = 0;
                    return NULL;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected integer literal.");
                parser->status = 0;
                return NULL;
            }
        } else if(parser->cur.type == ASSIGN) { // initialization
            update_parser(parser);

            astnode_t *expr;
            expr = parse_expr(parser);
            add_astchild(var, expr);
        }
        if(parser->cur.type == COMMA) { // continue declaring vars
            update_parser(parser);

            astnode_t *var_next;
            var_next = parse_varlist(parser);
            add_astsibling(var, var_next);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier");
        parser->status = 0;
        return NULL;
    }

    return var;
}



/**
 *
 */
astnode_t *
parse_varlistnoinit(parser_t *parser)
{
    astnode_t *var;
    init_astnode(var, _VAR);

    if(parser->cur.type == IDENT) {
        var->text = parser->cur.text;
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                astnode_t *arr_dim;
                init_astnode(arr_dim, _INT_LIT);
                arr_dim->text = parser->cur.text;
                add_astchild(var, arr_dim);

                update_parser(parser);
                if(parser->cur.type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ']'");
                    parser->status = 0;
                    return NULL;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected integer literal.");
                parser->status = 0;
                return NULL;
            }
        }
        if(parser->cur.type == COMMA) { // continue declaring vars
            update_parser(parser);

            astnode_t *var_next;
            var_next = parse_varlistnoinit(parser);
            add_astsibling(var, var_next);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        return NULL;
    }

    return var;
}



/**
 *
 */
astnode_t *
parse_forparams(parser_t *parser)
{
    astnode_t *for_init, *for_exit, *for_update;
    init_astnode(for_init, _FOR_INIT);
    init_astnode(for_exit, _FOR_EXIT);
    init_astnode(for_update, _FOR_UPDATE);

    add_astsibling(for_init, for_exit);
    add_astsibling(for_exit, for_update);

    if(parser->cur.type == SEMI) { // empty first param
        update_parser(parser);
    } else { // non-empty first param
        astnode_t *init_expr;
        init_expr = parse_expr(parser);
        add_astchild(for_init, init_expr);
        
        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return NULL;
        }
    }

    if(parser->cur.type == SEMI) { // empty second param
        update_parser(parser);
    } else { // non-empty second param
        astnode_t *exit_expr;
        exit_expr = parse_expr(parser);
        add_astchild(for_exit, exit_expr);

        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return NULL;
        }
    }

    if(parser->cur.type == RPAR) { // empty third param
        update_parser(parser);
        return;
    } else { // non-empty third param
        astnode_t *update_expr;
        update_expr = parse_expr(parser);
        add_astchild(for_update, update_expr);

        if(parser->cur.type == RPAR) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ')'");
            parser->status = 0;
            return NULL;
        }
    }

    return for_init;
}



/**
 *
 */
astnode_t *
parse_argslist(parser_t *parser)
{
    if(parser->cur.type == RPAR) { // fn() i.e., empty argument list
        update_parser(parser);
        return NULL;
    }
    
    astnode_t *arg;
    arg = parse_expr(parser);

    if(parser->cur.type == COMMA) {
        update_parser(parser);

        astnode_t *next_arg;
        next_arg = parse_argslist(parser);
        add_astsibling(arg, next_arg);
    }

    return arg;
}



/**
 * TODO
 */
astnode_t *
parse_lvalue(parser_t *parser)
{
    astnode_t *var;
    init_astnode(var, _VAR);

    if(parser->cur.type == IDENT) {
        var->text = parser->cur.text;
        update_parser(parser);

        if(parser->cur.type == LBRAK) { // with array access
            update_parser(parser);

            astnode_t *arr_expr;
            arr_expr = parse_expr(parser);
            add_astchild(var, arr_expr);

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



/**
 * TODO
 */
astnode_t *
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



/**
 * TODO 
 */
astnode_t *
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



/**
 * TODO
 */
astnode_t * 
parse_expr(parser_t *parser, astnode_t *parent)
{
    parse_term(parser);
    parse_exprprime(parser);
}



/**
 *
 */
astnode_t *
parse_statementblock(parser_t *parser)
{
    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return NULL;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        return NULL;
    }

    astnode_t *statement, *next_statement;
    statement = parse_statement(parser);
    next_statement = parse_statementblock(parser);
    add_astsibling(statement, next_statement);

    return statement;
}



/**
 *
 */
astnode_t *
parse_statement(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement);

    if(parser->cur.type == SEMI) { 
        update_parser(parser);
        return NULL;
    } else if(parser->cur.type == BREAK) {
        statement->asttype = _BREAK;
        update_parser(parser);

        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                      0, parser->cur.text, "Expected ';'.");
            parser->status = 0;
            return NULL;
        }

        return statement;
    } else if(parser->cur.type == CONTINUE) {
        statement->asttype = _CONTINUE;
        update_parser(parser);
        
        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                      0, parser->cur.text, "Expected ';'.");
            parser->status = 0;
            return NULL;
        }

        return statement;
    } else if(parser->cur.type == RETURN) {
        statement->asttype = _RETURN;
        update_parser(parser);

        if(parser->cur.type == SEMI) {
            update_parser(parser);
        } else {
            astnode_t *expr;
            init_astnode(expr, _EXPR);
            expr = parse_expr(parser);
            add_astchild(statement, expr);

            if(parser->cur.type == SEMI) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ';'");
                parser->status = 0;
                return NULL;
            }
        }

        return statement;
    } else if(parser->cur.type == IF) {
        statement->asttype = _IF_STATEMENT;
        update_parser(parser);

        if(parser->cur.type == LPAR) {
            update_parser(parser);

            astnode_t *if_cond;
            init_astnode(if_cond, _IF_COND);
            add_astchild(statement, if_cond);

            astnode_t *if_expr;
            if_expr = parse_expr(parser);
            add_astchild(if_cond, if_expr);

            if(parser->cur.type == RPAR) {
                update_parser(parser);

                astnode_t *if_body;
                init_astnode(if_body, _IF_BODY);
                add_astchild(statement, if_body); 

                if(parser->cur.type == LBRACE) { // if(cond) { }
                    update_parser(parser);

                    astnode_t *if_statement_block;
                    if_statement_block = parse_statementblock(parser);
                    add_astchild(if_body, if_statement_block);
                    
                } else { // if(cond) statement
                    astnode_t *if_single_statement;
                    if_single_statement = parse_statement(parser);
                    add_astchild(if_body, if_single_statement);
                }

                if(parser->cur.type == ELSE) {
                    update_parser(parser);

                    astnode_t *else_body;
                    init_astnode(else_body, _ELSE_BODY);
                    add_astchild(statement, else_body);

                    if(parser->cur.type == LBRACE) { // else { }
                        update_parser(parser);

                        astnode_t *else_statement_block;
                        else_statement_block = parse_statementblock(parser);
                        add_astchild(else_body, else_statement_block);
                    } else { // else statement
                        astnode_t *else_single_statement;
                        else_single_statement = parse_statement(parser);
                        add_astchild(else_body, else_single_statement);
                    }
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ')'");
                parser->status = 0;
                return NULL
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
            return NULL;
        }

        return statement;
    } else if(parser->cur.type == FOR) {
        statement->asttype = _FOR_STATEMENT;
        update_parser(parser);

        if(parser->cur.type == LPAR) {
            update_parser(parser);

            astnode_t *for_params;
            for_params = parse_forparams(parser);
            add_astchild(statement,for_params);

            astnode_t *for_body;
            init_astnode(for_body, _FOR_BODY);
            add_astchild(statement, for_body);

            if(parser->cur.type == LBRACE) { // for() { }
                update_parser(parser);

                astnode_t *for_statement_block;
                for_statement_block = parse_statementblock(parser);
                add_astchild(for_body, for_statement_block);
                
            } else { // for() statement
                astnode_t *for_single_statement;
                for_single_statement = parse_statement(parser);
                add_astchild(for_body, for_single_statement);
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
            return NULL;
        }

        return statement;
    } else if(parser->cur.type == WHILE) {
        statement->asttype = _WHILE_STATEMENT;
        update_parser(parser);

        if(parser->cur.type == LPAR) {
            update_parser(parser);

            astnode_t *while_cond;
            init_astnode(while_cond, _WHILE_COND);
            add_astchild(statement, while_cond);

            astnode_t *while_expr;
            while_expr = parse_expr(parser);
            add_astchild(while_cond, while_expr);

            if(parser->cur.type == RPAR) {
                update_parser(parser);

                astnode_t *while_body;
                init_astnode(while_body, _WHILE_BODY);
                add_astchild(statement, while_body);

                if(parser->cur.type == LBRACE) {
                    update_parser(parser);

                    astnode_t *while_statement_block;
                    while_statement_block = parse_statementblock(parser);
                    add_astchild(while_body, while_statement_block);

                } else {
                    astnode_t *while_single_statement;
                    while_single_statement = parse_statement(parser);
                    add_astchild(while_body, while_single_statement);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ')'");
                parser->status = 0;
                return NULL;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '('");
            parser->status = 0;
            return NULL;
        }
    } else if(parser->cur.type == DO) {
        statement->asttype = _DO_STATEMENT;
        update_parser(parser);

        astnode_t *do_body;
        init_astnode(do_body, _DO_BODY);
        add_astchild(statement, do_body);

        if(parser->cur.type == LBRACE) {
            update_parser(parser);
            
            astnode_t *do_statement_block;
            do_statement_block = parse_statementblock(parser);
            add_astchild(do_body, do_statement_block); 
        } else {
            astnode_t *do_single_statement;
            do_single_statement = parse_statement(parser);
            add_astchild(do_body, do_single_statement); 
        }

        if(parser->cur.type == WHILE) {
            update_parser(parser);
            if(parser->cur.type == LPAR) {
                update_parser(parser);

                astnode_t *do_cond;
                init_astnode(do_cond, _DO_COND);
                add_astchild(statement, do_cond);
                
                astnode_t *do_expr
                do_expr = parse_expr(parser);
                add_astchild(do_cond, do_expr);

                if(parser->cur.type == RPAR) {
                    update_parser(parser);

                    if(parser->cur.type == SEMI) {
                        update_parser(parser);
                        return statement;
                    } else {
                        print_msg(PARSER_ERR, parser->lex.filename, 
                            parser->lex.line_num, 0, parser->cur.text, 
                            "Expected ';'.");
                        parser->status = 0;
                        return NULL;
                    }
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ')'.");
                    parser->status = 0;
                    return NULL;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected '('.");
                parser->status = 0;
                return NULL;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected while following do.");
            parser->status = 0;
            return NULL;
        }
    } else {
        astnode_t *statement_expr;
        statement_expr = parse_expr(parser);

        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return statement_expr;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return NULL;
        }
    }

    return NULL;
}


/**
 *
 */
astnode_t *
parse_typedeclbody(parser_t *parser)
{
    astnode_t *var_decl, *var, *sibling;
    init_astnode(var_decl, _VAR_DECL);

    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return NULL;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        return NULL;
    }

    astnode_t *var;
    parse_type(parser, var_decl);
    var = parse_varlistnoinit(parser, var_decl);
    add_astchild(var_decl, var);

    sibling = parse_typedeclbody(parser);
    add_astsibling(var_decl, sibling);

    if(parser->cur.type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                  0, parser->cur.text, "Expected ';'");
        parser->status = 0;
        return NULL;
    }

    return var_decl;
}



/**
 *
 */
astnode_t *
parse_typedecl(parser_t *parser)
{
    astnode_t *type_decl, *var_decl;
    init_astnode(type_decl, _TYPE_DECL);

    var_decl = parse_typedeclbody(parser);
    add_astchild(type_decl, var_decl);

    return type_decl;
}



/**
 *
 */
astnode_t *
parse_funbody(parser_t *parser)
{
    astnode_t *fun_statement;
    init_astnode(fun_statement);

    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return NULL;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        return NULL;
    }

    if(parser->cur.type == STRUCT) { // 'struct' ident
        update_parser(parser);
        if(parser->cur.type == IDENT) {
            update_parser(parser);
            if(parser->cur.type == LBRACE) { // type-decl
                fun_statement->asttype = _TYPE_DECL;
                update_parser(parser);

                fun_statement = parse_typedecl(parser);

                if(parser->cur.type == SEMI) {
                    update_parser(parser);

                    astnode_t *next_fun_statmement;
                    next_fun_statmement = parse_funstatement(parser);
                    add_astsibling(fun_statement, next_fun_statmement);

                    return fun_statement;
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ';'.");
                    parser->status = 0;
                    return NULL;
                }
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, 
                parser->lex.line_num, 0, parser->cur.text, 
                "Expected identifier.");
            parser->status = 0;
            return NULL;
        }
    }

    if(is_typeorqualifier(parser->cur.type)) {
        fun_statement->asttype = _VAR_DECL;

        parse_type(parser, fun_statement);
        fun_statement = parse_varlist(parser);

        if(parser->cur.type == SEMI) {
            update_parser(parser);

            astnode_t *next_fun_statmement;
            next_fun_statmement = parse_funstatement(parser);
            add_astsibling(fun_statement, next_fun_statmement);

            return fun_statement;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            return NULL;
        }
    } else {
        fun_statement = parse_statement(parser);

        astnode_t *next_fun_statmement;
        next_fun_statmement = parse_funstatement(parser);
        add_astsibling(fun_statement, next_fun_statmement);

        return fun_statement;
    }
}


/**
 *
 */
astnode_t *
parse_funbody(parser_t *parser)
{
    astnode_t *fun_body;
    init_astnode(fun_body, _FUN_BODY);

    astnode_t *fun_statement;
    fun_statement = parse_funstatement(parser);
    add_astchild(fun_body, fun_statement);

    return fun_body;
}



/**
 * This function parses a global statement. These can be one of a type
 * declaration, function prototype, or function definition. This function also
 * returns the sub-AST rooted in the global node.
 */
astnode_t * 
parse_global(parser_t *parser) 
{
    astnode_t *global;
    init_astnode(global);

    if(is_typeorqualifier(parser->cur.type)) {

        if(parser->cur.type == STRUCT) { // 'struct' ident
            update_parser(parser);

            if(parser->cur.type == IDENT) {
                if(parser->cur.type == LBRACE) { // type-decl
                    global->asttype = _TYPE_DECL;
                    global->text = parser->cur.text;

                    update_parser(parser);
                    update_parser(parser);

                    global = parse_typedecl(parser, node);

                    if(parser->cur.type == SEMI) {
                        update_parser(parser);

                        astnode_t *next_global;
                        next_global = parse_global(parser);
                        add_astsibling(global, next_global);

                        return global;
                    } else {
                        print_msg(PARSER_ERR, parser->lex.filename, 
                            parser->lex.line_num, 0, parser->cur.text, 
                            "Expected ';'.");
                        parser->status = 0;
                        return NULL;
                    }
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected identifier.");
                parser->status = 0;
                return NULL;
            }
        } else {
            // assign return type to function or type to variable declaration
            parse_type(parser, global); 
        }

        if(parser->cur.type == IDENT) {
            if(parser->next.type == LPAR) { // fun-def or fun-proto
                global->text = parser->cur.text;

                update_parser(parser);
                update_parser(parser);

                astnode_t *var;
                var = parse_paramlist(parser);

                if(parser->cur.type == SEMI) { // fun-proto
                    global->type = _FUN_DECL;
                    add_astchild(global, var);

                    update_parser(parser);

                    astnode_t *next_global;
                    next_global = parse_global(parser);
                    add_astsibling(global, next_global);


                    return global;
                } else if(parser->cur.type == LBRACE) { // fun-def
                    global->type = _FUN_DEF;
                    update_parser(parser);

                    astnode_t *fun_decl;
                    init_astnode(fun_decl, _FUN_DECL);
                    fun_decl->text = global->text;
                    add_astchild(global, fun_decl);
                    add_astchild(fun_decl, var);

                    astnode_t *fun_body;
                    init_astnode(fun_body, _FUN_BODY);
                    fun_body = parse_funbody(parser);
                    add_astchild(global, fun_body);

                    astnode_t *next_global;
                    next_global = parse_global(parser);
                    add_astsibling(global, next_global);

                    return global;
                }
            } else { // var decl
                global->asttype = _VAR_DECL;

                astnode_t *var;
                var = parse_varlist(parser);
                add_astchild(global, var);

                if(parser->cur.type == SEMI) {
                    update_parser(parser);

                    astnode_t *next_global;
                    next_global = parse_global(parser);
                    add_astsibling(global, next_global);

                    return global;
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ';'");
                    parser->status = 0;
                    return NULL;
                }
            }

        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text,"Expected identifier or type declaration.");
            parser->status = 0;
            return NULL;
        }   
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected type name.");
        parser->status = 0;
        return NULL;
    }

    return NULL;
}



/**
 *
 */
astnode_t * 
parse_program(parser_t *parser)
{
    astnode_t *program;
    init_astnode(program, _PROGRAM);

    astnode_t *global;
    global = parse_global(parser);
    add_astchild(program, global);

    return program;
}



/**
 *
 */
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



/**
 *
 */
void 
parse(char *infilename, FILE *outfile)
{
    parser_t parser;
    init_parser(infilename, &parser);
    parse_program(&parser);
    if(parser.status > 0) {
        fprintf(outfile, "File %s is syntactically correct.\n", infilename);
        exit(1);
    }
}



/**
 *
 */
void
typecheck(astnode_t *root)
{
    // traverse tree via DFS (left-most childern first i.e., in order that
    // statements appear in the input)
    // update symbol table on-the-fly
    // if any type rules are violated, return

    return;
}