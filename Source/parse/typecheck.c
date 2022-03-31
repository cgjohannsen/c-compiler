#include <stdbool.h>
#include <string.h>

#include "../util/io.h"
#include "parse.h"
#include "symtable.h"
#include "typecheck.h"


exprtype_t
to_exprtype(char *type)
{
    if(!strcmp(type, "char")) {
        return __CHAR;
    } else if(!strcmp(type, "int")) {
        return __INT;
    } else if(!strcmp(type, "float")) {
        return __REAL;
    }
    return __STRUCT;
}

bool
is_sametype(astnode_t *type1, astnode_t *type2)
{
    return type1->is_const == type2->is_const &&
           type1->is_array == type2->is_array && 
           type1->is_struct == type2->is_struct &&
           !strcmp(type1->text, type2->text);
}

bool
is_numeric(exprtype_t type)
{
    return type == __CHAR || type == __INT || type == __REAL;
}

bool
is_integral(exprtype_t type)
{
    return type == __CHAR || type == __INT;
}


/**
 *
 */
exprtype_t
widen_to(exprtype_t source, exprtype_t target)
{
    if(source == target) {
        return source;
    }

    if(source == __CHAR && target == __INT) {
        return __INT;
    } else if(source == __CHAR && target == __REAL) {
        return __REAL;
    } else if(source == __INT && target == __REAL) {
        return __REAL;
    }

    return __NONE;
}

/**
 *
 */
exprtype_t
widen(exprtype_t type1, exprtype_t type2)
{
    exprtype_t ret;
    
    ret = widen_to(type1, type2);
    if(ret == __NONE) {
        ret = widen_to(type2, type1);
        if(ret == __NONE) {
            ret = __NONE;
        }
    }

    return ret;
}


/**
 *
 */
void
print_var(astnode_t *type, astnode_t *var)
{
    if(type->is_const) {
        fprintf(outfile, "const ");
    }
    if(type->is_struct) {
        fprintf(outfile, "struct ");
    }
    fprintf(outfile, "%s %s", type->text, var->text);
    if(var->is_array && var->arr_dim > -1) {
        fprintf(outfile, "[%d]", var->arr_dim);
    } else if(var->is_array) { // function paramter -- default arr_dim = -1
        fprintf(outfile, "[]");
    }
}


/**
 *
 */
void
print_exprtype(astnode_t *expr)
{
    switch(expr->exprtype) {
        case __CHAR:
            fprintf(outfile, "char");
            break;
        case __INT:
            fprintf(outfile, "int");
            break;
        case __REAL:
            fprintf(outfile, "float");
            break;
        case __STRING:
            fprintf(outfile, "char[]");
            break;
        default:
            break;
    }
    if(expr->is_array) {
        fprintf(outfile, "[]");
    }
}


/**
 *
 */
void
typecheck_expr(symtable_t *table, astnode_t *expr)
{
    astnode_t *lhs, *rhs, *rhs2;

    if(expr == NULL) {
        return;
    }
    
    if(expr->left != NULL) {
        lhs = expr->left;
        if(lhs->right != NULL) {
            rhs = lhs->right;
            if(rhs->right != NULL) {
                rhs2 = rhs->right;
            }
        }
    }

    switch(expr->type) {
        case _ITE:
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);
            typecheck_expr(table, rhs2);

            if(!is_numeric(lhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "First argument to ternary of invalid type.");
            }

            if(rhs->exprtype == rhs2->exprtype) {
                expr->exprtype = rhs->exprtype;
            } else if(widen(rhs->exprtype, rhs2->exprtype) != __NONE) {
                expr->exprtype = widen(rhs->exprtype, rhs2->exprtype);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Second and third arguments to ternary operator of invalid types.");
            }

            break;
        case _ASSIGN:
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);
            
            if(lhs->is_const) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Attempting to change value of const variable.");
            }

            if(widen_to(rhs->exprtype, lhs->exprtype) != __NONE) {
                expr->exprtype = widen_to(rhs->exprtype, lhs->exprtype);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Attempting to set variable to inconsistent type.");
            }

            break;
        case _INCR: 
        case _DECR: 
        case _ARITH_NEG: // N -> N
            typecheck_expr(table, lhs);

            if(is_numeric(lhs->exprtype)) {
                expr->exprtype = lhs->exprtype;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for unary operator.");
            }

            break;
        case _LOG_NEG: // N -> char
            typecheck_expr(table, lhs);

            if(is_numeric(lhs->exprtype)) {
                expr->exprtype = __CHAR;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for unary operator.");
            }

            break;
        case _BIT_NEG: // I -> I
            typecheck_expr(table, lhs);

            if(is_integral(lhs->exprtype)) {
                expr->exprtype = lhs->exprtype;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for unary operator.");
            }

            break;
        case _TYPE: // N -> type
            typecheck_expr(table, lhs);
            expr->exprtype = to_exprtype(lhs->text);
            break;
        case _EQ:
        case _NEQ:
        case _GEQ:
        case _GT:
        case _LEQ:
        case _LT:
        case _LOG_AND:
        case _LOG_OR: // N x N -> char
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_numeric(lhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for LHS of operator.");
            }

            if(!is_numeric(rhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for RHS of operator.");
            }

            expr->exprtype = __CHAR;

            break;
        case _ADD:
        case _SUB:
        case _MULT:
        case _DIV: // N x N -> N
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_numeric(lhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for LHS of operator.");
            }

            if(!is_numeric(rhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for RHS of operator.");
            }

            expr->exprtype = widen(lhs->exprtype, rhs->exprtype);

            break;
        case _MOD:
        case _BIT_AND:
        case _BIT_OR: // I x I -> I
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_integral(lhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for LHS of operator.");
            }

            if(!is_integral(rhs->exprtype)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Invalid input type for RHS of operator.");
            }

            expr->exprtype = widen(lhs->exprtype, rhs->exprtype);

            break;
        case _FUN_CALL:
            funsym_t *funsym;

            funsym = get_function(table, expr->text);
            if(funsym == NULL) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Function referenced before declaration.");
            }

            astnode_t *param1;
            varsym_t *param2;
            param1 = expr->left;
            param2 = funsym->param;
            while(param1 != NULL && param2 != NULL) {
                typecheck_expr(table, param1);

                if(param1->exprtype != to_exprtype(param2->type->text)) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                        "Function parameter types mismatch.");
                }

                param1 = param1->right;
                param2 = param2->next;
            }
            if(param1 != NULL || param2 != NULL) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                    "Number of function parameters mismatch.");
            }

            expr->exprtype = to_exprtype(funsym->ret_type->text);

            break;
        case _ARR_ACCESS:
            break;
        case _STRUCT_ACCESS:
            break;
        case _VAR:
            varsym_t *varsym;

            varsym = get_globalvar(table, expr->text);
            if(varsym == NULL) {
                varsym = get_localvar(table, expr->text);
                if(varsym == NULL) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", 
                        "Variable referenced before declaration.");
                }
            } 

            expr->is_const = varsym->type->is_const;

            expr->exprtype = to_exprtype(varsym->type->text);

            break;
        case _CHAR_LIT:
            expr->exprtype = __CHAR;
            break;
        case _INT_LIT:
            expr->exprtype = __INT;
            break;
        case _REAL_LIT:
            expr->exprtype = __REAL;
            break;
        default: // fail
            break;
    }
}


/**
 *
 */
void
typecheck_statement(symtable_t *table, astnode_t *statement)
{
    astnode_t *cur;

    switch(statement->type) {
        case SEMI:
        case BREAK:
        case CONTINUE:
            break;
        case RETURN:
            typecheck_expr(table, statement->left);
            if(!is_sametype(statement->left, table->ret_type)) {
                print_msg(TYPE_ERR, statement->filename, statement->line_num, 0, "", 
                    "Return type does not match function definition.");
            }
            break;
        case IF:
            cur = statement->left->left; // if-cond
            typecheck_expr(table, cur);

            cur = statement->left->right->left; // if-body
            while(cur != NULL) {
                typecheck_statement(table, cur);
                cur = cur->right;
            }

            cur = statement->left->right->right->left; // else-body
            while(cur != NULL) {
                typecheck_statement(table, cur);
                cur = cur->right;
            }

            break;
        case FOR:
            cur = statement->left->left; // for-params
            while(cur != NULL) {
                typecheck_expr(table, cur);
                cur = cur->right;
            }
            
            cur = statement->left->right->left; // for-body
            while(cur != NULL) {
                typecheck_statement(table, cur);
                cur = cur->right;
            }

            break;
        case WHILE:
            cur = statement->left->left; // while-cond
            typecheck_expr(table, cur);
            
            cur = statement->left->right->left; // while-body
            while(cur != NULL) {
                typecheck_statement(table, cur);
                cur = cur->right;
            }
            
            break;
        case DO:
            cur = statement->left->right->left; // do-body
            while(cur != NULL) {
                typecheck_statement(table, cur);
                cur = cur->right;
            }

            cur = statement->left->right->left; // do-cond
            typecheck_expr(table, cur);
            
            break;
        default:
            typecheck_expr(table, statement);
            fprintf(outfile, "\tLine %*d: expression has type ", 4, statement->line_num);
            print_exprtype(statement);
            fprintf(outfile, "\n");
            break; 
    }
}


/**
 *
 */
void
typecheck_localvardecl(symtable_t *table, astnode_t *var_decl)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    // check if struct type has been declared yet
    if(type->is_struct && get_localstruct(table, type->text) == NULL && 
            get_globalstruct(table, type->text) == NULL) {
        print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
            "Struct type never declared.");  
    }

    while(var != NULL) {
        varsym_t *sym;
        sym = get_localvar(table, var->text);

        if(sym != NULL) { // already declared
            print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
                "Local variable previously declared.");  
        } else {
            add_localvar(table, type, var);

            fprintf(outfile, "\tLine %*d: local ", 4, var->line_num);
            print_var(type, var);
            fprintf(outfile, "\n");
        }

        if(var->left != NULL) { // includes initialization
            typecheck_expr(table, var->left);

            if(widen_to(var->left->exprtype, to_exprtype(type->text)) == __NONE) {
                print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
                    "Attempting to set variable to inconsistent type.");
            }
        }

        var = var->right;
    }
}


/**
 *
 */
void
typecheck_globalvardecl(symtable_t *table, astnode_t *var_decl)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    // check if struct type has been declared yet
    if(type->is_struct && get_localstruct(table, type->text) == NULL && 
            get_globalstruct(table, type->text) == NULL) {
        print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
            "Struct type never declared.");  
    }

    while(var != NULL) {
        varsym_t *sym;
        sym = get_globalvar(table, var->text);

        if(sym != NULL) { // already declared
            print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
                "Global variable previously declared.");  
        } else {
            add_globalvar(table, type, var);

            fprintf(outfile, "Line %*d: global ", 4, var->line_num);
            print_var(type, var);
            fprintf(outfile, "\n");
        }

        if(var->left != NULL) { // includes initialization
            typecheck_expr(table, var->left);

            if(widen_to(var->left->exprtype, to_exprtype(type->text)) == __NONE) {
                print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
                    "Attempting to set variable to inconsistent type.");
            }
        }

        var = var->right;
    }
}


/**
 *
 */
void
typecheck_localtypedecl(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *sym;
    sym = get_localstruct(table, type_decl->text);

    if(sym != NULL) { // struct already declared
        print_msg(TYPE_ERR, type_decl->filename, type_decl->line_num, 0, "", 
            "Local struct previously declared.");  
    } else {
        add_localstruct(table, type_decl);
        fprintf(outfile, "\tLine %*d: local struct %s\n", 4, type_decl->line_num, type_decl->text);
    }


    astnode_t *var_decl, *type, *var;
    var_decl = type_decl->left;

    while(var_decl != NULL) { // cycle thru member declarations
        type = var_decl->left;
        var = type->right;

        while(var != NULL) { // cycle thru each variable
            fprintf(outfile, "\t\tLine %*d: member ", 4, var->line_num);
            print_var(type, var);
            fprintf(outfile, "\n");

            var = var->right;
        }

        var_decl = var_decl->right;
    } 
}


/**
 *
 */
void
typecheck_globaltypedecl(symtable_t *table, astnode_t *type_decl)
{
    structsym_t *sym;
    sym = get_globalstruct(table, type_decl->text);

    if(sym != NULL) { // struct already declared
        print_msg(TYPE_ERR, type_decl->filename, type_decl->line_num, 0, "", 
            "Global struct previously declared.");  
    } else {
        add_globalstruct(table, type_decl);
        fprintf(outfile, "Line %*d: global struct %s\n", 4, type_decl->line_num, type_decl->text);
    }

    astnode_t *var_decl, *type, *var;
    var_decl = type_decl->left;

    while(var_decl != NULL) { // cycle thru member declarations
        type = var_decl->left;
        var = type->right;

        while(var != NULL) { // cycle thru each variable
            fprintf(outfile, "\tLine %*d: member ", 4, var->line_num);
            print_var(type, var);
            fprintf(outfile, "\n");

            var = var->right;
        }

        var_decl = var_decl->right;
    } 
}


/**
 *
 */
void
typecheck_funbody(symtable_t *table, astnode_t *fun_body)
{
    astnode_t *statement;

    statement = fun_body->left;
    while(statement != NULL) {
        if(statement->type == _VAR_DECL) {
            typecheck_localvardecl(table, statement);
        } else if(statement->type == _TYPE_DECL) {
            typecheck_localtypedecl(table, statement);
        } else {
            typecheck_statement(table, statement);
        }

        statement = statement->right;
    }
}


/**
 *
 */
void 
typecheck_fundecl(symtable_t *table, astnode_t *fun_decl, bool is_def)
{
    astnode_t *ret_type, *ident, *args;

    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;

    table->ret_type = ret_type;

    if(!add_function(table, fun_decl, is_def)) { // function previously declared
        funsym_t *sym;
        sym = get_function(table, ident->text);

        if(is_def) {
            if(sym->is_def) {
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Function already defined.");
            } else {
                sym->is_def = true;
            }
        }

        // check return type
        if(!is_sametype(fun_decl->left, sym->ret_type)) {
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                "Function return type does not match previous declaration.");
        }

        // check params
        astnode_t *type1;
        varsym_t *type2;
        type1 = args->left;
        type2 = sym->param;

        while(type1 != NULL && type2 != NULL) {
            if(!is_sametype(type1, type2->type)) {
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Parameter type does not match previous declaration.");
            }

            if(!add_localvar(table, type1, type1->right)) {
                // parameter already exists
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Parameter name already in use.");
            }
            free_locals(table);

            type1 = type1->right->right;
            type2 = type2->next;
        }

        if((type1 == NULL && type2 != NULL) || (type1 != NULL && type2 == NULL)) {
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Number of function parameters does not match previous declaration.");
        }
          
    }
    if(is_def) {
        fprintf(outfile, "Line %*d: function ", 4, fun_decl->line_num);
        print_var(ret_type, ident);
        fprintf(outfile, "\n");   
    }

    astnode_t *param;
    param = args->left;

    while(param != NULL) {
        if(is_def) {
            fprintf(outfile, "\tLine %*d: parameter ", 4, param->line_num);
            print_var(param, param->right);
            fprintf(outfile, "\n");
        }

        if(!add_localvar(table, param, param->right)) {
            // parameter already exists
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                "Parameter name already in use.");
        }

        param = param->right->right;
    }
}


/**
 *
 */
 void
typecheck_fundef(symtable_t *table, astnode_t *fun_def)
{
    astnode_t *fun_decl, *fun_body;
    fun_decl = fun_def->left;
    fun_body = fun_decl->right;

    typecheck_fundecl(table, fun_decl, true);
    typecheck_funbody(table, fun_body);

    free_locals(table);
}


/**
 *
 */
void
typecheck_program(symtable_t *table, astnode_t *program)
{
    astnode_t *global;
    global = program->left;


    while(global != NULL) {
        switch(global->type) {
            case _TYPE_DECL:
                typecheck_globaltypedecl(table, global);
                break;
            case _VAR_DECL:
                typecheck_globalvardecl(table, global);
                break;
            case _FUN_DECL:
                typecheck_fundecl(table, global, false);
                free_locals(table);
                break;
            case _FUN_DEF:
                typecheck_fundef(table, global);
                break;
        }
        global = global->right;
    }
}


/**
 *
 */
int 
typecheck(char *filename)
{
    symtable_t *table;
    parser_t parser;
    astnode_t *program;

    table = init_symtable();
    init_parser(filename, &parser);
    program = parse_program(&parser);

    typecheck_program(table, program);

    free_symtable(table);

    return 0;
}