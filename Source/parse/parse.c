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

    tmp1 = parser->nextnext;
    tmp2 = parser->next;
    next_token(&parser->lex, &parser->nextnext);
    parser->next = tmp1;
    parser->cur = tmp2;
}  


// forward declarations
astnode_t parse_statement(parser_t *parser);
astnode_t parse_expr(parser_t *parser);



/**
 *
 */
astnode_t *
parse_type(parser_t *parser)
{
    astnode_t *type;
    init_astnode(type, _TYPE, "");

    if(parser->cur.type == CONST) { // 'const'
        type->is_const = true;
        update_parser(parser);
        if(parser->cur.type == TYPE) { // 'const' type
            set_asttext(node, parser->cur.text);
            update_parser(parser);
            return type;
        } else if(parser->cur.type == STRUCT) { // 'const' 'struct'
            update_parser(parser);
            if(parser->cur.type == IDENT) { // 'const' 'struct' ident
                type->is_struct = true;
                set_asttext(node, parser->cur.text);
                update_parser(parser);
                return type;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected identifier.");
                parser->status = 0;
                return NULL;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected type or 'struct'.");
            parser->status = 0;
            return NULL;
        }
    }   

    if(parser->cur.type == TYPE) { // type
        set_asttext(node, parser->cur.text);
        update_parser(parser);
        if(parser->cur.type == CONST) { // type 'const'
            type->is_const = true;
            update_parser(parser);
            return type;
        }
        return type;
    }

    if(parser->cur.type == STRUCT) { // 'struct'
        type->is_struct = true;
        update_parser(parser);
        if(parser->cur.type == IDENT) { // 'struct' ident
            set_asttext(node, parser->cur.text);
            update_parser(parser);
            if(parser->cur.type == CONST) { // 'struct' ident 'const'
                type->is_const = true;
                update_parser(parser);
                return type;
            }
            return type;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected identifier.");
            parser->status = 0;
            return NULL;
        }
    }
}



/**
 *
 */
astnode_t *
parse_forparams(parser_t *parser)
{
    astnode_t *for_init, *for_exit, *for_update;

    if(parser->cur.type == SEMI) { // empty first param
        init_astnode(for_init, _FOR_INIT, "");
        update_parser(parser);
    } else { // non-empty first param
        astnode_t *init_expr;
        init_astnode(for_init, _FOR_INIT, parser->cur.text);
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
        init_astnode(for_exit, _FOR_EXIT, "");
        update_parser(parser);
    } else { // non-empty second param
        astnode_t *exit_expr;
        init_astnode(for_exit, _FOR_EXIT, parser->cur.text);
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
        init_astnode(for_exit, _FOR_UPDATE, "");
        update_parser(parser);
    } else { // non-empty third param
        astnode_t *update_expr;
        init_astnode(for_exit, _FOR_UPDATE, parser->cur.text);
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

    add_astsibling(for_init, for_exit);
    add_astsibling(for_exit, for_update);

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
 *
 */
astnode_t *
parse_literal(parser_t *parser)
{
    astnode_t *literal;

    switch(parser->cur.type) {
        case CHAR_LIT: 
            init_astnode(literal, _CHAR_LIT, parser->cur.text);
            break;
        case INT_LIT:
            init_astnode(literal, _INT_LIT, parser->cur.text);
            break;
        case REAL_LIT:
            init_astnode(literal, _REAL_LIT, parser->cur.text);
            break;
        case STR_LIT:
            init_astnode(literal, _STR_LIT, parser->cur.text);
            break;
        default:
            literal = NULL;
            break;
    }

    update_parser(parser);
    
    return literal;
}



/**
 *
 */
astnode_t *
parse_lvalue(parser_t *parser)
{
    astnode_t *var;
    init_astnode(var, _VAR, parser->cur.text);

    if(parser->cur.type == IDENT) {
        lvalue->text = parser->cur.text;
        update_parser(parser);

        if(parser->cur.type == LBRAK) { // array access
            update_parser(parser);

            astnode_t *arr_access;
            init_astnode(arr_access, _ARR_ACCESS, parser->cur.text);

            astnode_t *arr_expr;
            arr_expr = parse_expr(parser);
            add_astchild(arr_access, arr_expr);
            add_astchild(arr_access, var);

            if(parser->cur.type == RBRAK) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ']'");
                parser->status = 0;
                return NULL;
            }

            if(parser->cur.type == DOT) { // array and struct access
                update_parser(parser);

                astnode_t *struct_access;
                init_astnode(struct_access, _STRUCT_ACCESS, parser->cur.text);

                astnode_t *struct_member
                struct_member = parse_lvalue(parser);
                add_astchild(struct_access, arr_access);
                add_astchild(struct_access, struct_member);

                return struct_access;
            } else { // only array access
                return arr_access;
            }
        }
        if(parser->cur.type == DOT) { // struct access
            update_parser(parser);

            astnode_t *struct_access;
            init_astnode(struct_access, _STRUCT_ACCESS, parser->cur.text);

            astnode_t *struct_member
            struct_member = parse_lvalue(parser);
            add_astchild(struct_access, var);
            add_astchild(struct_access, struct_member);

            return struct_access;
        } else { // only a variable
            return var;
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        return NULL;
    }
}


/**
 *
 */
astnode_t *
parse_expr(parser_t *parser)
{
    astnode_t *expr;
    init_astnode(expr, _EXPR, parser->cur.text);
    return expr;
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
parse_break(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _BREAK, parser->cur.text);

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
}



/**
 *
 */
astnode_t *
parse_continue(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _CONTINUE, parser->cur.text);

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
}



/** 
 *
 */
astnode_t *
parse_return(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _RETURN, parser->cur.text);

    update_parser(parser);

    if(parser->cur.type == SEMI) {
        update_parser(parser);
    } else {
        astnode_t *expr;
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
}



/**
 *
 */
astnode_t *
parse_if(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _IF_STATEMENT, parser->cur.text);

    update_parser(parser);

    if(parser->cur.type == LPAR) {
        update_parser(parser);

        astnode_t *if_cond;
        init_astnode(if_cond, _IF_COND, parser->cur.text);
        add_astchild(statement, if_cond);

        astnode_t *if_expr;
        if_expr = parse_expr(parser);
        add_astchild(if_cond, if_expr);

        if(parser->cur.type == RPAR) {
            update_parser(parser);

            astnode_t *if_body;
            init_astnode(if_body, _IF_BODY, parser->cur.text);
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
                init_astnode(else_body, _ELSE_BODY, parser->cur.text);
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
}



/**
 *
 */
astnode_t *
parse_for(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _FOR_STATEMENT, parser->cur.text);

    update_parser(parser);

    if(parser->cur.type == LPAR) {
        update_parser(parser);

        astnode_t *for_params;
        for_params = parse_forparams(parser);
        add_astchild(statement,for_params);

        astnode_t *for_body;
        init_astnode(for_body, _FOR_BODY, parser->cur.text);
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
}



/**
 *
 */
astnode_t *
parser_while(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _WHILE_STATEMENT, parser->cur.text);

    update_parser(parser);

    if(parser->cur.type == LPAR) {
        update_parser(parser);

        astnode_t *while_cond;
        init_astnode(while_cond, _WHILE_COND, parser->cur.text);
        add_astchild(statement, while_cond);

        astnode_t *while_expr;
        while_expr = parse_expr(parser);
        add_astchild(while_cond, while_expr);

        if(parser->cur.type == RPAR) {
            update_parser(parser);

            astnode_t *while_body;
            init_astnode(while_body, _WHILE_BODY, parser->cur.text);
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

    return statement;
}



/**
 *
 */
astnode_t *
parse_do(parser_t *parser)
{
    astnode_t *statement;
    init_astnode(statement, _DO_STATEMENT, parser->cur.text);

    update_parser(parser);

    astnode_t *do_body;
    init_astnode(do_body, _DO_BODY, parser->cur.text);
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
            init_astnode(do_cond, _DO_COND, parser->cur.text);
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
} 



/**
 *
 */
astnode_t *
parse_statement(parser_t *parser)
{
    switch(parser->cur.type) {
        case SEMI:
            update_parser(parser);
            return NULL;
        case BREAK:
            return parse_break(parser);
        case CONTINUE:
            return parse_continue(parser);
        case RETURN:
            return parse_return(parser);
        case IF:
            return parse_if(parser);
        case FOR:
            return parse_for(parser);
        case WHILE:
            return parse_while(parser);
        case DO:
            return parse_do(parser);
        default:
            astnode_t *statement;
            statement = parse_expr(parser);

            if(parser->cur.type == SEMI) {
                update_parser(parser);
                return statement;
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                    0, parser->cur.text, "Expected ';'");
                parser->status = 0;
                exit(1);
            }
    }
}



astnode_t *
parse_vardecl(parser_t *parser)
{
    astnode_t *var_decl, *type;
    
    init_astnode(var_decl, _VAR_DECL, "");
    type = parse_type(parser, var_decl);
    add_astchild(var_decl, type);

    if(parser->cur.type != IDENT) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        exit(1);
    }

    astnode_t *var, *var_init;
    while(parser->cur.type == IDENT) {
        init_astnode(var, _VAR, parser->cur.text);
        add_astchild(var_decl, var);

        if(parser->cur.type == LBRAK) {
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                init_astnode(arr_dim, _ARR_DIM, parser->cur.text);
                add_astchild(var, arr_dim);
                update_parser(parser);
                if(parser->cur.type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected '}'.");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected integer literal.");
                parser->status = 0;
                exit(1);
            }
        } else if(parser->cur.type == ASSIGN) {
            update_parser(parser);
            var_init = parse_expr(parser);
            add_astchild(var, var_init);
        } 

        if(parser->cur.type == COMMA) {
            update_parser(parser);
        }
    }

    if(parser->cur.type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'.");
        parser->status = 0;
        exit(1);
    }

    return var_decl;
}



astnode_t *
parse_vardeclnoinit(parser_t *parser)
{
    astnode_t *var_decl, *type;

    init_astnode(var_decl, _VAR_DECL, "");
    type = parse_type(parser, var_decl);
    add_astchild(var_decl, type);

    if(parser->cur.type != IDENT) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected identifier.");
        parser->status = 0;
        exit(1);
    }

    astnode_t *var;
    while(parser->cur.type == IDENT) {
        init_astnode(var, _VAR, parser->cur.text);
        add_astchild(var_decl, var);

        if(parser->cur.type == LBRAK) {
            update_parser(parser);
            if(parser->cur.type == INT_LIT) {
                init_astnode(arr_dim, _ARR_DIM, parser->cur.text);
                add_astchild(var, arr_dim);
                update_parser(parser);
                if(parser->cur.type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected '}'.");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, 
                    "Expected integer literal.");
                parser->status = 0;
                exit(1);
            }
        } 

        if(parser->cur.type == COMMA) {
            update_parser(parser);
        }
    }

    if(parser->cur.type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'.");
        parser->status = 0;
        exit(1);
    }

    return var_decl;
}



astnode_t *
parse_typedecl(parser_t *parser)
{
    update_parser(parser); // consume 'struct' keyword

    astnode_t *type_decl;
    init_astnode(type_decl, _TYPE_DECL, parser->cur.text);
    
    update_parser(parser);

    if(parser->cur.type == LBRACE) {
        update_parser(parser);

        astnode_t *var_declnoinit;
        while(is_typeorqualifier(parser->cur.type)) {
            var_declnoinit = parse_vardeclnoinit(parser);
            add_astchild(type_decl, var_declnoinit);

            if(parser->cur.type == SEMI) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex.filename, 
                    parser->lex.line_num, 0, parser->cur.text, "Expected ';'.");
                parser->status = 0;
                exit(1);
            }
        }

        if(parser->cur.type == RBRACE) {
            update_parser(parser);
            return type_decl;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected '}'.");
            parser->status = 0;
            exit(1);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 0, 
            parser->cur.text, "Expected '{'.");
        parser->status = 0;
        exit(1);
    }
}



/**
 *
 */
astnode_t *
parse_funstatement(parser_t *parser)
{
    if(parser->cur.type == RBRACE) {
        update_parser(parser);
        return NULL;
    } else if(parser->cur.type == END) {
        print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
            0, parser->cur.text, "Expected '}' before end of file.");
        parser->status = 0;
        exit(1);
    }

    astnode_t *fun_statement;

    if(parser->cur.type == STRUCT) { 
        if(parser->next.type == IDENT) {
            if(parser->nextnext.type == LBRACE) { // type-decl
                fun_statement = parse_typedecl(parser);

                if(parser->cur.type == SEMI) {
                    update_parser(parser);
                    return fun_statement;
                } else {
                    print_msg(PARSER_ERR, parser->lex.filename, 
                        parser->lex.line_num, 0, parser->cur.text, 
                        "Expected ';'.");
                    parser->status = 0;
                    exit(1);
                }
            }
        } 
    }

    if(is_typeorqualifier(parser->cur.type)) {
        fun_statement = parse_vardecl(parser);

        if(parser->cur.type == SEMI) {
            update_parser(parser);
            return fun_statement;
        } else {
            print_msg(PARSER_ERR, parser->lex.filename, parser->lex.line_num, 
                0, parser->cur.text, "Expected ';'");
            parser->status = 0;
            exit(1);
        }
    } else {
        fun_statement = parse_statement(parser);
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
    init_astnode(fun_body, _FUN_BODY, parser->cur.text);

    while(add_astchild(fun_body, parse_funstatement(parser)));

    return fun_body;
}



/**
 * This function parses a global statement. These can be one of a type
 * declaration, function prototype, or function definition. This function also
 * returns the AST rooted in the global node.
 */
astnode_t * 
parse_global(parser_t *parser) 
{
    astnode_t *global;

    if(parser->cur.type == STRUCT) {
        if(parser->next.type == IDENT) {
            if(parser->nextnext.type == LBRACE) { // type-decl
                global = parse_typedecl(parser);
                return global;
            }
        }
    }

    // determine if global is a:
    // 1) var-decl
    // 2) fun-decl

    astnode_t *type;
    type = parse_type(parser);

    if(parser->cur.type == IDENT) {
        if(parser->next.type == LPAR) {
            astnode_t *args;
            args = parse_funargs(parser);
        }
    } else {
        // error
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
    init_astnode(program, _PROGRAM, parser->cur.type);

    while(add_astchild(progam, parse_global(parser)));

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
        if(parser->next.type != END) {
            next_token(&parser->lex, &parser->nextnext);
        }
    }
}



/**
 *
 */
astnode_t * 
parse(char *infilename, FILE *outfile)
{
    parser_t parser;
    init_parser(infilename, &parser);
    parse_program(&parser);
    if(parser.status > 0) {
        fprintf(outfile, "File %s is syntactically correct.\n", infilename);
        exit(1);
    }

    return NULL;
}


/*
if(is_typeorqualifier(parser->cur.type)) {
        if(parser->cur.type == STRUCT) { 
            update_parser(parser);
            if(parser->cur.type == IDENT) { // 'struct' ident
                if(parser->cur.type == LBRACE) { // type-decl
                    global = parse_typedecl(parser);

                    astnode_t *next_global;
                    next_global = parse_global(parser);
                    add_astsibling(global, next_global);

                    return global;
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
                char *ident = parser->cur.text;

                update_parser(parser);
                update_parser(parser);

                astnode_t *var;
                var = parse_paramlist(parser);

                if(parser->cur.type == SEMI) { // fun-proto
                    init_astnode(global, _FUN_DECL, ident);                    
                    add_astchild(global, var);

                    update_parser(parser);

                    return global;
                } else if(parser->cur.type == LBRACE) { // fun-def
                    init_astnode(global, _FUN_DEF, ident);                    
                    
                    update_parser(parser);

                    astnode_t *fun_decl;
                    init_astnode(fun_decl, _FUN_DECL, parser->cur.text);
                    add_astchild(global, fun_decl);
                    add_astchild(fun_decl, var);

                    astnode_t *fun_body, *fun_statements;
                    fun_body = parse_funbody(parser);
                    add_astchild(global, fun_body);

                    return global;
                }
            } else { // var decl
                init_astnode(global, _VAR_DECL, parser->cur.text);   

                astnode_t *var;
                var = parse_varlist(parser);
                add_astchild(global, var);

                if(parser->cur.type == SEMI) {
                    update_parser(parser);
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
*/