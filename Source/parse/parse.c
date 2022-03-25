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
void
parse_paramlist(parser_t *parser. astnode_t *parent)
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
        parse_type(parser,&var);
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
            add_astchild(parent,var);
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
    init_astnode(var,_VAR);

    if(parser->cur.type == IDENT) {
        var->text = parser->cur.text;
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
            var->is_array = true;
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                var->array_param = atoi(parser->cur.text);
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
            add_astchild(var,expr);
        }
        if(parser->cur.type == COMMA) { // continue declaring vars
            update_parser(parser);

            astnode_t *var_next;
            var_next = parse_varlist(parser);
            add_astsibling(var,var_next);
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
    init_astnode(var,_VAR);

    if(parser->cur.type == IDENT) {
        var->text = parser->cur.text;
        update_parser(parser);
        if(parser->cur.type == LBRAK) { // array
            var->is_array = true;
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                var->array_param = atoi(parser->cur.text);
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
            add_astsibling(var,var_next);
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
parse_vardecl(parser_t *parser)
{
    astnode_t *var_decl, *var;
    init_astnode(var_decl,_VAR_DECL);

    parse_type(parser,var_decl);
    var = parse_varlist(parser);
    add_astchild(var_decl,var);

    return var_decl;
}



/**
 *
 */
void
parse_forparams(parser_t *parser, astnode_t *parent)
{
    astnode_t *init_node, *exit_node, *update_node;

    init_astnode(init_node);
    init_astnode(exit_node);
    init_astnode(update_node);

    init_node->asttype = _FOR_INIT;
    exit_node->asttype = _FOR_EXIT;
    update_node->asttype = _FOR_UPDATE;

    add_astchild(parent,init_node);
    add_astchild(parent,exit_node);
    add_astchild(parent,update_node);

    if(parser->cur.type == SEMI) { // empty first param
        update_parser(parser);
    } else { // non-empty first param
        parse_expr(parser,init_node);
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
        parse_expr(parser,exit_node);
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
        parse_expr(parser,update_node);
        if(parser->cur.type == RPAR) {
            update_parser(parser);
            return;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ')'");
            parser->status = 0;
            return;
        }
    }
}



/**
 *
 */
void
parse_argslist(parser_t *parser, astnode_t *parent)
{
    parse_expr(parser,parent);
    if(parser->cur.type == COMMA) {
        update_parser(parser);
        parse_argslist(parser);
    }
}



/**
 *
 */
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
 *
 */
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



/**
 *
 */
void
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
 *
 */
void 
parse_expr(parser_t *parser, astnode_t *parent)
{
    parse_term(parser);
    parse_exprprime(parser);
}



/**
 *
 */
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
            init_astnode(expr,_EXPR);
            expr = parse_expr(parser,ret_node);
            add_astchild(statement,expr);

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

            astnode_t *expr;
            expr = parse_expr(parser);
            add_astchild(statement,expr);

            if(parser->cur.type == RPAR) {
                update_parser(parser);

                if(parser->cur.type == LBRACE) {
                    update_parser(parser);

                    astnode_t *statement_block;
                    statement_block = parse_block(parser,block_node);
                    add_astchild(statement,statement_block);
                    
                } else {
                    astnode_t *single_statement;
                    single_statement = parse_statement(parser);
                    add_astchild(statement,single_statement);
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
        statement->asttype = _EXPR;
        parse_expr(parser,statement);
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


/**
 *
 */
astnode_t *
parse_typedeclbody(parser_t *parser)
{
    astnode_t *var_decl, *var, *sibling;
    init_astnode(var_decl,_VAR_DECL);

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
    parse_type(parser,var_decl);
    var = parse_varlistnoinit(parser,var_decl);
    add_astchild(var_decl,var);

    sibling = parse_typedeclbody(parser);
    add_astsibling(var_decl,sibling);

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
    init_astnode(type_decl,_TYPE_DECL);

    var_decl = parse_typedeclbody(parser);
    add_astchild(type_decl,var_decl);

    return type_decl;
}



/**
 *
 */
void
parse_fundef(parser_t *parser, astnode_t *parent)
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

    parse_fundef(parser);
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

                    global = parse_typedecl(parser,node);

                    if(parser->cur.type == SEMI) {
                        update_parser(parser);
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
            parse_type(parser,global); 
        }

        if(parser->cur.type == IDENT) {
            if(parser->next.type == LPAR) { // fun-def or fun-proto
                global->text = parser->cur.text;

                update_parser(parser);
                update_parser(parser);

                astnode_t *var;
                var = parse_paramlist(parser);
                add_astchild(global,var);

                if(parser->cur.type == SEMI) { // fun-proto
                    global->type = _FUN_DECL;

                    update_parser(parser);

                    return global;
                } else if(parser->cur.type == LBRACE) { // fun-def
                    global->type = _FUN_DEF;

                    update_parser(parser);

                    parse_fundef(parser);

                    return global;
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
                0, parser->cur.text,"Expected identifier or type declaration.");
            parser->status = 0;
            return;
        }   
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected type name.");
        parser->status = 0;
        return;
    }

    return global;
}



/**
 *
 */
astnode_t * 
parse_program(parser_t *parser)
{
    astnode_t *program, *global;
    init_astnode(program,_PROGRAM);

    while(parser->cur.type != END && parser->status > 0) {
        global = parse_global(parser);
        add_astchild(program,global);
    }

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
        fprintf(outfile,"File %s is syntactically correct.\n",infilename);
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