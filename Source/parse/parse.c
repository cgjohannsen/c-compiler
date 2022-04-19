#include "../util/io.h"
#include "ast.h"
#include "parse.h"

/**
 *
 */
void 
update_parser(parser_t *parser)
{
    if(parser->cur->tok_type == END) {
        return;
    }

    parser->cur = parser->next;
    parser->next = parser->nextnext;
    parser->nextnext = next_token(parser->lex);
}  


// forward declarations
astnode_t *parse_statement(parser_t *parser);
astnode_t *parse_expr(parser_t *parser, int prec);



/**
 *
 */
astnode_t *
parse_type(parser_t *parser)
{
    astnode_t *type;
    type = init_astnode(_TYPE, parser->cur);

    if(parser->cur->tok_type == CONST) { // 'const'
        type->ctype.is_const = true;
        update_parser(parser);
        if(parser->cur->tok_type == TYPE) { // 'const' type
            set_asttext(type, parser->cur->text);
            set_ctypename(type, parser->cur->text);
            update_parser(parser);
            return type;
        } else if(parser->cur->tok_type == STRUCT) { // 'const' 'struct'
            type->ctype.is_struct = true;
            update_parser(parser);
            if(parser->cur->tok_type == IDENT) { // 'const' 'struct' ident
                set_asttext(type, parser->cur->text);
                set_ctypename(type, parser->cur->text);
                update_parser(parser);
                return type;
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, 
                    "Expected identifier.");
                parser->status = 0;
                return NULL;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected type or 'struct'.");
            parser->status = 0;
            return NULL;
        }
    }   

    if(parser->cur->tok_type == TYPE) { // type
        set_asttext(type, parser->cur->text);
        set_ctypename(type, parser->cur->text);
        update_parser(parser);
        if(parser->cur->tok_type == CONST) { // type 'const'
            type->ctype.is_const = true;
            update_parser(parser);
            return type;
        }
        return type;
    }

    if(parser->cur->tok_type == STRUCT) { // 'struct'
        type->ctype.is_struct = true;
        update_parser(parser);
        if(parser->cur->tok_type == IDENT) { // 'struct' ident
            set_asttext(type, parser->cur->text);
            set_ctypename(type, parser->cur->text);
            update_parser(parser);
            if(parser->cur->tok_type == CONST) { // 'struct' ident 'const'
                type->ctype.is_const = true;
                update_parser(parser);
                return type;
            }
            return type;
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected identifier.");
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
parse_funparams(parser_t *parser)
{
    astnode_t *params;
    params = init_astnode(_FUN_PARAMS, parser->cur);

    astnode_t *type, *var;
    while(is_typeorqualifier(parser->cur->tok_type)) {
        type = parse_type(parser);
        
        if(parser->cur->tok_type == IDENT) {
            var = init_astnode(_VAR, parser->cur);
            add_astchild(params, type);
            add_astchild(params, var);

            update_parser(parser);

            if(parser->cur->tok_type == LBRAK) {
                update_parser(parser);
                if(parser->cur->tok_type == RBRAK) {
                    var->ctype.is_array = true;
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ']'");
                    parser->status = 0;
                    exit(1);
                }
            }
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected identifier.");
            parser->status = 0;
            exit(1);
        }

        if(parser->cur->tok_type == COMMA) {
            update_parser(parser);
            if(!is_typeorqualifier(parser->cur->tok_type)) {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, 
                    "Expected type name.");
                parser->status = 0;
                exit(1);
            }
        }
    }

    return params;
}



/**
 *
 */
astnode_t *
parse_literal(parser_t *parser)
{
    astnode_t *literal;

    switch(parser->cur->tok_type) {
        case CHAR_LIT: 
            literal = init_astnode(_CHAR_LIT, parser->cur);
            break;
        case INT_LIT:
            literal = init_astnode(_INT_LIT, parser->cur);
            break;
        case REAL_LIT:
            literal = init_astnode(_REAL_LIT, parser->cur);
            break;
        case STR_LIT:
            literal = init_astnode(_STR_LIT, parser->cur);
            break;
        default:
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected literal.");
            parser->status = 0;
            exit(1);
    }

    literal->ctype.is_const = true;

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
    var = init_astnode(_VAR, parser->cur);

    if(parser->cur->tok_type == IDENT) {
        update_parser(parser);

        if(parser->cur->tok_type == LBRAK) { // array access
            update_parser(parser);

            astnode_t *arr_access;
            arr_access = init_astnode(_ARR_ACCESS, parser->cur);

            astnode_t *arr_expr;
            arr_expr = parse_expr(parser, 1);
            add_astchild(arr_access, arr_expr);
            add_astchild(arr_access, var);

            if(parser->cur->tok_type == RBRAK) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, "Expected ']'");
                parser->status = 0;
                exit(1);
            }

            if(parser->cur->tok_type == DOT) { // array and struct access
                update_parser(parser);

                astnode_t *struct_access;
                struct_access = init_astnode(_STRUCT_ACCESS, parser->cur);

                astnode_t *struct_member;
                struct_member = parse_lvalue(parser);
                add_astchild(struct_access, arr_access);
                add_astchild(struct_access, struct_member);

                return struct_access;
            } else { // only array access
                return arr_access;
            }
        }
        if(parser->cur->tok_type == DOT) { // struct access
            update_parser(parser);

            astnode_t *struct_access;
            struct_access = init_astnode(_STRUCT_ACCESS, parser->cur);

            astnode_t *struct_member;
            struct_member = parse_lvalue(parser);
            add_astchild(struct_access, var);
            add_astchild(struct_access, struct_member);

            return struct_access;
        } else { // only a variable
            return var;
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected identifier");
        parser->status = 0;
        exit(1);
    }
}



astnode_t *
parse_term(parser_t *parser)
{
    astnode_t *op, *expr;

    switch(parser->cur->tok_type) {
        case BANG:
            op = init_astnode(_LOG_NEG, parser->cur);
            update_parser(parser);
            expr = parse_term(parser);
            add_astchild(op, expr);
            return op;
        case TILDE:
            op = init_astnode(_BIT_NEG, parser->cur);
            update_parser(parser);
            expr = parse_term(parser);
            add_astchild(op, expr);
            return op;
        case MINUS:
            op = init_astnode(_ARITH_NEG, parser->cur);
            update_parser(parser);
            expr = parse_term(parser);
            add_astchild(op, expr);
            return op;
        case DECR:
            op = init_astnode(_DECR, parser->cur);
            update_parser(parser);
            expr = parse_lvalue(parser);
            add_astchild(op, expr);
            return op;
        case INCR:
            op = init_astnode(_INCR, parser->cur);
            update_parser(parser);
            expr = parse_lvalue(parser);
            add_astchild(op, expr);
            return op;
        case LPAR:
            if(is_typeorqualifier(parser->next->tok_type)) { // type cast
                update_parser(parser);
                op = parse_type(parser);
                if(parser->cur->tok_type == RPAR) {
                    update_parser(parser);
                    expr = parse_expr(parser, 1);
                    add_astchild(op, expr);
                    return op;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ')'");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                update_parser(parser);
                expr = parse_expr(parser, 1);
                if(parser->cur->tok_type == RPAR) {
                    update_parser(parser);
                    return expr;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                       parser->cur->line_num,0,parser->cur->text,"Expected ')'");
                    parser->status = 0;
                    exit(1);
                }
            }
        case IDENT:
            if(parser->next->tok_type == LPAR) { // function call
                op = init_astnode(_FUN_CALL, parser->cur);
                update_parser(parser);
                update_parser(parser);

                astnode_t *arg;
                if(parser->cur->tok_type != RPAR) { // non-empty args list
                    arg = parse_expr(parser, 1);
                    add_astchild(op, arg);

                    while(parser->cur->tok_type == COMMA) {
                        if(parser->next->tok_type == RPAR) {
                            print_msg(PARSER_ERR, parser->lex->filename, 
                                parser->cur->line_num,0,parser->cur->text,
                                "Expected expression in function call");
                            parser->status = 0;
                            exit(1);
                        }
                        update_parser(parser);
                        arg = parse_expr(parser, 1);
                        add_astchild(op, arg);
                    }
                }

                if(parser->cur->tok_type != RPAR) {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                       parser->cur->line_num,0,parser->cur->text,"Expected ')'");
                    parser->status = 0;
                    exit(1);
                }

                update_parser(parser); // consume ')'

                return op;
            } else { // l-value
                expr = parse_lvalue(parser);

                if(parser->cur->tok_type == INCR) {
                    op = init_astnode(_INCR, parser->cur);
                    add_astchild(op, expr);
                    update_parser(parser);
                    return op;
                } else if(parser->cur->tok_type == DECR) {
                    op = init_astnode(_DECR, parser->cur);
                    add_astchild(op, expr);
                    update_parser(parser);
                    return op;
                } else {
                    return expr;
                }
            }
        default:
            if(is_literal(parser->cur->tok_type)) {
                return parse_literal(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num,0,parser->cur->text,"Expected term");
                parser->status = 0;
                exit(1);
            }
    }
}



/**
 *
 * Note that the switch statement ~does not~ include break statements.
 *
 * Precedence chart:
 * = += -= *= /=      1 (lowest)
 * ?:                 2
 * ||                 3
 * &&                 4
 * |                  5
 * &                  6
 * == !=              7
 * < <= > >=          8
 * + -                9
 * * / %              10
 *
 * --- parse_term ---
 *
 * ! ~ - -- ++ (type) 11
 * () [] .            12 (highest)
 * 
 * @param parser
 * @param prec
 *
 * @return 
 */
astnode_t *
parse_expr(parser_t *parser, int prec)
{
    astnode_t *term, *op, *op2, *expr1, *expr2;

    term = parse_term(parser);

    switch(prec) {
        case 1:
            if(is_assignop(parser->cur->tok_type)) {
                if(is_lvalue(term)) {
                    op = init_astnode(_ASSIGN, parser->cur);

                    switch(parser->cur->tok_type) {
                        case ASSIGN:
                            op2 = NULL;
                            break;
                        case PLUSASSIGN:
                            op2 = init_astnode(_ADD, parser->cur);
                            break;
                        case MINUSASSIGN:
                            op2 = init_astnode(_SUB, parser->cur);
                            break;
                        case STARASSIGN:
                            op2 = init_astnode(_MULT, parser->cur);
                            break;
                        case SLASHASSIGN:
                            op2 = init_astnode(_DIV, parser->cur);
                            break;
                        default:
                            // impossible
                            break;
                    }
                    update_parser(parser);

                    expr1 = parse_expr(parser, prec);

                    if(op2 != NULL) {
                        add_astchild(op2, term);
                        add_astchild(op2, expr1);

                        add_astchild(op, term);
                        add_astchild(op, op2);
                    } else {
                        add_astchild(op, term);
                        add_astchild(op, expr1);
                    }

                    return op;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected l-value on lhs of assignment operator.");
                    parser->status = 0;
                    exit(1);
                }
            }
        case 2:
            if(parser->cur->tok_type == QUEST) {
                op = init_astnode(_ITE, parser->cur);
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                if(parser->cur->tok_type == COLON) {
                    update_parser(parser);

                    expr2 = parse_expr(parser, prec);

                    add_astchild(op, term);
                    add_astchild(op, expr1);
                    add_astchild(op, expr2);

                    return op;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ':'");
                    parser->status = 0;
                    exit(1);
                }
            }
        case 3:
            if(parser->cur->tok_type == DPIPE) {
                op = init_astnode(_LOG_OR, parser->cur);
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 4:
            if(parser->cur->tok_type == DAMP) {
                op = init_astnode(_LOG_AND, parser->cur);
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 5:
            if(parser->cur->tok_type == PIPE) {
                op = init_astnode(_BIT_OR, parser->cur);
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 6:
            if(parser->cur->tok_type == AMP) {
                op = init_astnode(_BIT_AND, parser->cur);
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 7:
            if(parser->cur->tok_type == EQ || parser->cur->tok_type == NEQ) {
                if(parser->cur->tok_type == EQ) {
                    op = init_astnode(_EQ, parser->cur);
                } else { // NEQ
                    op = init_astnode(_NEQ, parser->cur);
                }
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 8:
            if(parser->cur->tok_type == GT || parser->cur->tok_type == GEQ || 
                parser->cur->tok_type == LT || parser->cur->tok_type == LEQ) {

                if(parser->cur->tok_type == GT) {
                    op = init_astnode(_GT, parser->cur);
                } else if(parser->cur->tok_type == GEQ) {
                    op = init_astnode(_GEQ, parser->cur);
                } else if(parser->cur->tok_type == LT) {
                    op = init_astnode(_LT, parser->cur);
                } else { // LEQ
                    op = init_astnode(_LEQ, parser->cur);
                }
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 9:
            if(parser->cur->tok_type == PLUS || parser->cur->tok_type == MINUS) {
                if(parser->cur->tok_type == PLUS) {
                    op = init_astnode(_ADD, parser->cur);
                } else { // MINUS
                    op = init_astnode(_SUB, parser->cur);
                }
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 10:
            if(parser->cur->tok_type == STAR || parser->cur->tok_type == SLASH ||
                parser->cur->tok_type == MOD) {
                if(parser->cur->tok_type == STAR) {
                    op = init_astnode(_MULT, parser->cur);
                } else if(parser->cur->tok_type == SLASH) {
                    op = init_astnode(_DIV, parser->cur);
                } else { // MOD
                    op = init_astnode(_MOD, parser->cur);
                }
                update_parser(parser);

                expr1 = parse_expr(parser, prec);

                add_astchild(op, term);
                add_astchild(op, expr1);

                return op;
            }
        case 11:
            if(parser->cur->tok_type == INCR || parser->cur->tok_type == DECR) {
                if(parser->cur->tok_type == INCR) {
                    op = init_astnode(_INCR, parser->cur);
                } else { // DECR
                    op = init_astnode(_DECR, parser->cur);
                }
                update_parser(parser);

                add_astchild(op, term);

                return op;
            }
        default:
            // no such operator with precedence > prec --> return term
            return term;
    }
}



/**
 *
 */
astnode_t *
parse_statementblock(parser_t *parser)
{
    if(parser->cur->tok_type == RBRACE) {
        update_parser(parser);
        return NULL;
    } else if(parser->cur->tok_type == END) {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected '}' before end of file.");
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
    statement = init_astnode(_BREAK, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ';'.");
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
    statement = init_astnode(_CONTINUE, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ';'.");
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
    statement = init_astnode(_RETURN, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == SEMI) {
        update_parser(parser);
    } else {
        astnode_t *expr;
        expr = parse_expr(parser, 1);
        add_astchild(statement, expr);

        if(parser->cur->tok_type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, 
                parser->cur->line_num, 0, parser->cur->text, "Expected ';'");
            parser->status = 0;
            exit(1);
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
    statement = init_astnode(_IF_STATEMENT, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == LPAR) {
        update_parser(parser);

        astnode_t *if_cond;
        if_cond = init_astnode(_IF_COND, parser->cur);
        add_astchild(statement, if_cond);

        astnode_t *if_expr;
        if_expr = parse_expr(parser, 1);
        add_astchild(if_cond, if_expr);

        if(parser->cur->tok_type == RPAR) {
            update_parser(parser);

            astnode_t *if_body;
            if_body = init_astnode(_IF_BODY, parser->cur);
            add_astchild(statement, if_body); 

            if(parser->cur->tok_type == LBRACE) { // if(cond) { }
                update_parser(parser);

                astnode_t *if_statement_block;
                if_statement_block = parse_statementblock(parser);
                add_astchild(if_body, if_statement_block);
                
            } else { // if(cond) statement
                astnode_t *if_single_statement;
                if_single_statement = parse_statement(parser);
                add_astchild(if_body, if_single_statement);
            }

            if(parser->cur->tok_type == ELSE) {
                update_parser(parser);

                astnode_t *else_body;
                else_body = init_astnode(_ELSE_BODY, parser->cur);
                add_astchild(statement, else_body);

                if(parser->cur->tok_type == LBRACE) { // else { }
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
            print_msg(PARSER_ERR, parser->lex->filename, 
                parser->cur->line_num, 0, parser->cur->text, "Expected ')'");
            exit(1);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected '('");
        exit(1);
    }

    return statement;
}



/**
 *
 */
astnode_t *
parse_forparams(parser_t *parser)
{
    astnode_t *for_init, *for_exit, *for_update;

    if(parser->cur->tok_type == SEMI) { // empty first param
        for_init = init_astnode(_FOR_INIT, parser->cur);
        update_parser(parser);
    } else { // non-empty first param
        astnode_t *init_expr;
        for_init = init_astnode(_FOR_INIT, parser->cur);
        init_expr = parse_expr(parser, 1);
        add_astchild(for_init, init_expr);
        
        if(parser->cur->tok_type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ';'");
            parser->status = 0;
            exit(1);
        }
    }

    if(parser->cur->tok_type == SEMI) { // empty second param
        for_exit = init_astnode(_FOR_EXIT, parser->cur);
        update_parser(parser);
    } else { // non-empty second param
        astnode_t *exit_expr;
        for_exit = init_astnode(_FOR_EXIT, parser->cur);
        exit_expr = parse_expr(parser, 1);
        add_astchild(for_exit, exit_expr);

        if(parser->cur->tok_type == SEMI) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ';'");
            parser->status = 0;
            exit(1);
        }
    }

    if(parser->cur->tok_type == RPAR) { // empty third param
        for_update = init_astnode(_FOR_UPDATE, parser->cur);
        update_parser(parser);
    } else { // non-empty third param
        astnode_t *update_expr;
        for_update = init_astnode(_FOR_UPDATE, parser->cur);
        update_expr = parse_expr(parser, 1);
        add_astchild(for_update, update_expr);

        if(parser->cur->tok_type == RPAR) {
            update_parser(parser);
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ')'");
            parser->status = 0;
            exit(1);
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
parse_for(parser_t *parser)
{
    astnode_t *statement;
    statement = init_astnode(_FOR_STATEMENT, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == LPAR) {
        update_parser(parser);

        astnode_t *for_params;
        for_params = parse_forparams(parser);
        add_astchild(statement,for_params);

        astnode_t *for_body;
        for_body = init_astnode(_FOR_BODY, parser->cur);
        add_astchild(statement, for_body);

        if(parser->cur->tok_type == LBRACE) { // for() { }
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
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected '('");
        parser->status = 0;
        return NULL;
    }

    return statement;
}



/**
 *
 */
astnode_t *
parse_while(parser_t *parser)
{
    astnode_t *statement;
    statement = init_astnode(_WHILE_STATEMENT, parser->cur);

    update_parser(parser);

    if(parser->cur->tok_type == LPAR) {
        update_parser(parser);

        astnode_t *while_cond;
        while_cond = init_astnode(_WHILE_COND, parser->cur);
        add_astchild(statement, while_cond);

        astnode_t *while_expr;
        while_expr = parse_expr(parser, 1);
        add_astchild(while_cond, while_expr);

        if(parser->cur->tok_type == RPAR) {
            update_parser(parser);

            astnode_t *while_body;
            while_body = init_astnode(_WHILE_BODY, parser->cur);
            add_astchild(statement, while_body);

            if(parser->cur->tok_type == LBRACE) {
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
            print_msg(PARSER_ERR, parser->lex->filename, 
                parser->cur->line_num, 0, parser->cur->text, "Expected ')'");
            parser->status = 0;
            return NULL;
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected '('");
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
    statement = init_astnode(_DO_STATEMENT, parser->cur);

    update_parser(parser);

    astnode_t *do_body;
    do_body = init_astnode(_DO_BODY, parser->cur);
    add_astchild(statement, do_body);

    if(parser->cur->tok_type == LBRACE) {
        update_parser(parser);
        
        astnode_t *do_statement_block;
        do_statement_block = parse_statementblock(parser);
        add_astchild(do_body, do_statement_block); 
    } else {
        astnode_t *do_single_statement;
        do_single_statement = parse_statement(parser);
        add_astchild(do_body, do_single_statement); 
    }

    if(parser->cur->tok_type == WHILE) {
        update_parser(parser);
        if(parser->cur->tok_type == LPAR) {
            update_parser(parser);

            astnode_t *do_cond;
            do_cond = init_astnode(_DO_COND, parser->cur);
            add_astchild(statement, do_cond);
            
            astnode_t *do_expr;
            do_expr = parse_expr(parser, 1);
            add_astchild(do_cond, do_expr);

            if(parser->cur->tok_type == RPAR) {
                update_parser(parser);

                if(parser->cur->tok_type == SEMI) {
                    update_parser(parser);
                    return statement;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ';'.");
                    parser->status = 0;
                    return NULL;
                }
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, 
                    "Expected ')'.");
                parser->status = 0;
                return NULL;
            }
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, 
                parser->cur->line_num, 0, parser->cur->text, "Expected '('.");
            parser->status = 0;
            return NULL;
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
            0, parser->cur->text, "Expected while following do.");
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
    switch(parser->cur->tok_type) {
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
            statement = parse_expr(parser, 1);

            if(parser->cur->tok_type == SEMI) {
                update_parser(parser);
                return statement;
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                    0, parser->cur->text, "Expected ';'");
                parser->status = 0;
                exit(1);
            }
    }
}



astnode_t *
parse_vardecl(parser_t *parser)
{
    astnode_t *var_decl, *type;
    
    var_decl = init_astnode(_VAR_DECL, parser->cur);
    type = parse_type(parser);
    add_astchild(var_decl, type);

    if(parser->cur->tok_type != IDENT) {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected identifier.");
        parser->status = 0;
        exit(1);
    }

    astnode_t *var, *var_init, *arr_dim;
    while(parser->cur->tok_type == IDENT) {
        var = init_astnode(_VAR, parser->cur);
        add_astchild(var_decl, var);

        update_parser(parser);

        if(parser->cur->tok_type == LBRAK) {
            var->ctype.is_array = true;
            update_parser(parser);
            if(parser->cur->tok_type == INT_LIT) {
                var->arr_dim = atoi(parser->cur->text);
                update_parser(parser);
                if(parser->cur->tok_type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ']'.");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, 
                    "Expected integer literal.");
                parser->status = 0;
                exit(1);
            }
        } else if(parser->cur->tok_type == ASSIGN) {
            update_parser(parser);
            var_init = parse_expr(parser, 1);
            add_astchild(var, var_init);
        } 

        if(parser->cur->tok_type == COMMA) {
            update_parser(parser);
        }
    }

    if(parser->cur->tok_type == SEMI) {
        update_parser(parser);
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected ';'.");
        parser->status = 0;
        exit(1);
    }

    return var_decl;
}



astnode_t *
parse_vardeclnoinit(parser_t *parser)
{
    astnode_t *var_decl, *type;

    var_decl = init_astnode(_VAR_DECL, parser->cur);
    type = parse_type(parser);
    add_astchild(var_decl, type);

    if(parser->cur->tok_type != IDENT) {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected identifier.");
        parser->status = 0;
        exit(1);
    }

    astnode_t *var, *arr_dim;
    while(parser->cur->tok_type == IDENT) {
        var = init_astnode(_VAR, parser->cur);
        add_astchild(var_decl, var);

        update_parser(parser);

        if(parser->cur->tok_type == LBRAK) {
            var->ctype.is_array = true;
            update_parser(parser);
            if(parser->cur->tok_type == INT_LIT) {
                var->arr_dim = atoi(parser->cur->text);
                update_parser(parser);
                if(parser->cur->tok_type == RBRAK) {
                    update_parser(parser);
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected '}'.");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, 
                    "Expected integer literal.");
                parser->status = 0;
                exit(1);
            }
        } 

        if(parser->cur->tok_type == COMMA) {
            update_parser(parser);
        }
    }

    return var_decl;
}



astnode_t *
parse_typedecl(parser_t *parser)
{
    update_parser(parser); // consume 'struct' keyword

    astnode_t *type_decl;
    type_decl = init_astnode(_TYPE_DECL, parser->cur);
    
    update_parser(parser);

    if(parser->cur->tok_type == LBRACE) {
        update_parser(parser);

        astnode_t *var_declnoinit;
        while(is_typeorqualifier(parser->cur->tok_type)) {
            var_declnoinit = parse_vardeclnoinit(parser);
            add_astchild(type_decl, var_declnoinit);

            if(parser->cur->tok_type == SEMI) {
                update_parser(parser);
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, "Expected ';'.");
                parser->status = 0;
                exit(1);
            }
        }

        if(parser->cur->tok_type == RBRACE) {
            update_parser(parser);
            return type_decl;
        } else {
            print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 
                0, parser->cur->text, "Expected '}'.");
            parser->status = 0;
            exit(1);
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 0, 
            parser->cur->text, "Expected '{'.");
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
    astnode_t *fun_statement;

    if(parser->cur->tok_type == STRUCT) { 
        if(parser->next->tok_type == IDENT) {
            if(parser->nextnext->tok_type == LBRACE) { // type-decl
                fun_statement = parse_typedecl(parser);

                if(parser->cur->tok_type == SEMI) {
                    update_parser(parser);
                    return fun_statement;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ';'.");
                    parser->status = 0;
                    exit(1);
                }
            }
        } 
    }

    if(is_typeorqualifier(parser->cur->tok_type)) {
        fun_statement = parse_vardecl(parser);
        return fun_statement;
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
    astnode_t *fun_body, *statement;
    fun_body = init_astnode(_FUN_BODY, parser->cur);

    while(parser->cur->tok_type != RBRACE) {
        statement = parse_funstatement(parser);
        if(statement != NULL) {
            add_astchild(fun_body, statement);
        }
    }

    return fun_body;
}



/**
 * Parses a global statement. These can be one of a type declaration, function
 * prototype, or function definition. This function also returns the AST rooted
 * in the global node.
 */
astnode_t * 
parse_global(parser_t *parser) 
{
    if(parser->cur->tok_type == STRUCT) {
        if(parser->next->tok_type == IDENT) {
            if(parser->nextnext->tok_type == LBRACE) { // type-decl
                astnode_t *type_decl;
                type_decl = parse_typedecl(parser);

                if(parser->cur->tok_type == SEMI) {
                    update_parser(parser);
                    return type_decl;
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ';'.");
                    parser->status = 0;
                    exit(1);
                }

                return type_decl;
            }
        }
    }

    astnode_t *type;
    type = parse_type(parser);

    if(parser->cur->tok_type == IDENT) {
        if(parser->next->tok_type == LPAR) {
            astnode_t *fun_decl;
            fun_decl = init_astnode(_FUN_DECL, parser->cur);
            add_astchild(fun_decl, type);

            astnode_t *ident;
            ident = init_astnode(_VAR, parser->cur);
            add_astchild(fun_decl, ident);

            update_parser(parser);
            update_parser(parser);

            astnode_t *args;
            args = parse_funparams(parser);
            add_astchild(fun_decl, args);

            if(parser->cur->tok_type == RPAR) {
                update_parser(parser);
                if(parser->cur->tok_type == SEMI) { // fun-decl
                    update_parser(parser);
                    return fun_decl;
                } else if(parser->cur->tok_type == LBRACE) { // fun-def
                    astnode_t *fun_def;
                    fun_def = init_astnode(_FUN_DEF, parser->cur);
                    add_astchild(fun_def, fun_decl);

                    update_parser(parser);
                    
                    astnode_t *fun_body;
                    fun_body = parse_funbody(parser);
                    add_astchild(fun_def, fun_body);

                    if(parser->cur->tok_type == RBRACE) {
                        update_parser(parser);
                        return fun_def;
                    } else {
                        print_msg(PARSER_ERR, parser->lex->filename, 
                                parser->cur->line_num, 0, parser->cur->text, 
                                "Expected '}'.");
                        parser->status = 0;
                        exit(1);
                    }
                } else {
                    print_msg(PARSER_ERR, parser->lex->filename, 
                        parser->cur->line_num, 0, parser->cur->text, 
                        "Expected ';' or '{'.");
                    parser->status = 0;
                    exit(1);
                }
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, "Expected ')'.");
                parser->status = 0;
                exit(1);
            }
        } else { // var-decl
            astnode_t *var_decl;
            var_decl = init_astnode(_VAR_DECL, parser->cur);
            set_asttext(var_decl, type->text);
            add_astchild(var_decl, type);

            astnode_t *var, *var_init, *arr_dim;
            while(parser->cur->tok_type == IDENT) {
                var = init_astnode(_VAR, parser->cur);
                add_astchild(var_decl, var);

                update_parser(parser);

                if(parser->cur->tok_type == LBRAK) {
                    var->ctype.is_array = true;
                    update_parser(parser);
                    if(parser->cur->tok_type == INT_LIT) {
                        arr_dim = init_astnode(_ARR_DIM, parser->cur);
                        add_astchild(var, arr_dim);
                        update_parser(parser);
                        if(parser->cur->tok_type == RBRAK) {
                            update_parser(parser);
                        } else {
                            print_msg(PARSER_ERR, parser->lex->filename, 
                                parser->cur->line_num, 0, parser->cur->text, 
                                "Expected ']'.");
                            parser->status = 0;
                            exit(1);
                        }
                    } else {
                        print_msg(PARSER_ERR, parser->lex->filename, 
                            parser->cur->line_num, 0, parser->cur->text, 
                            "Expected integer literal.");
                        parser->status = 0;
                        exit(1);
                    }
                } else if(parser->cur->tok_type == ASSIGN) {
                    update_parser(parser);
                    var_init = parse_expr(parser, 1);
                    add_astchild(var, var_init);
                } 

                if(parser->cur->tok_type == COMMA) {
                    update_parser(parser);
                }
            }

            if(parser->cur->tok_type == SEMI) {
                update_parser(parser);
                return var_decl;
            } else {
                print_msg(PARSER_ERR, parser->lex->filename, 
                    parser->cur->line_num, 0, parser->cur->text, "Expected ';'.");
                parser->status = 0;
                exit(1);
            }
        }
    } else {
        print_msg(PARSER_ERR, parser->lex->filename, parser->cur->line_num, 0, 
            parser->cur->text, "Expected identifier.");
        parser->status = 0;
        exit(1);
    }

    return NULL;
}



/**
 *
 */
astnode_t * 
parse_program(parser_t *parser)
{
    astnode_t *program, *global;
    program = init_astnode(_PROGRAM, parser->cur);

    while(parser->cur->tok_type != END) {
        global = parse_global(parser);
        add_astchild(program, global);
    }

    return program;
}



/**
 *
 */
void
init_parser(char *filename, parser_t *parser)
{
    parser->lex = init_lexer(filename);
    
    parser->status = 1;

    parser->cur = next_token(parser->lex);
    if(parser->cur->tok_type != END) {
        parser->next = next_token(parser->lex);
        if(parser->next->tok_type != END) {
            parser->nextnext = next_token(parser->lex);
        }
    }
}



/**
 *
 */
void
parse(char *filename)
{
    parser_t parser;
    astnode_t *program;

    init_parser(filename, &parser);
    program = parse_program(&parser);

    if(parser.status > 0) {
        fprintf(outfile, "File %s is syntactically correct.\n", filename);
        exit(1);
    }

    free_ast(program);
}

