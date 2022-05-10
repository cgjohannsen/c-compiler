#include <stdbool.h>
#include <string.h>

#include "../util/io.h"
#include "parse.h"
#include "symtable.h"
#include "typecheck.h"


/**
 *
 */
bool
widen_to(astnode_t *source, astnode_t *target, astnode_t *result)
{   
    if(!strcmp(source->ctype.name, target->ctype.name)) {
        set_ctypename(result, source->ctype.name);
        return false;
    }

    if(!strcmp(source->ctype.name, "char")) {
        if(!strcmp(target->ctype.name, "int") || !strcmp(target->ctype.name, "float")) {
            set_ctypename(result, target->ctype.name);
            return false;
        }
    } else if(!strcmp(source->ctype.name, "int") && !strcmp(target->ctype.name, "float")) {
        set_ctypename(result, target->ctype.name);
        return false;
    }

    return true;
}

/**
 * return true on failure, false otherwise
 */
bool
widen(astnode_t *type1, astnode_t *type2, astnode_t *result)
{
    if(widen_to(type1, type2, result)) { 
        // if type1 to type2 fails, try reciprocal
        return widen_to(type2, type1, result);
    }

    // success
    return false; 
}


/**
 *
 */
void
print_var(astnode_t *type, astnode_t *var)
{
    if(type->ctype.is_const) {
        fprintf(outfile, "const ");
    }
    if(type->ctype.is_struct) {
        fprintf(outfile, "struct ");
    }
    fprintf(outfile, "%s %s", type->text, var->text);
    if(var->ctype.is_array && var->arr_dim > -1) {
        fprintf(outfile, "[%d]", var->arr_dim);
    } else if(var->ctype.is_array) { // function paramter -- default arr_dim = -1
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

    lhs = NULL;
    rhs = NULL;
    rhs2 = NULL;

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

    switch(expr->node_type) {
        case _ITE:
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);
            typecheck_expr(table, rhs2);

            if(!is_numctype(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tFirst argument %s of ternary not numeric type (%s)\n", 
                    lhs->text, lhs->ctype.name);
            }

            if(widen(rhs, rhs2, expr)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tArguments %s, %s of ternary of incomptaible types (%s, %s)\n", 
                    rhs->text, rhs2->text, rhs2->ctype.name, rhs2->ctype.name);
            }

            break;
        case _ASSIGN:
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);
            
            if(lhs->ctype.is_const) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tAttempting to change value of const variable (%s)\n", lhs->text);
            }

            if(widen_to(rhs, lhs, expr)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tAttempting to assign variable %s to incompatible type (%s, %s)\n", 
                    lhs->text, lhs->ctype.name, rhs->ctype.name);
            }

            break;
        case _PRE_INCR: 
        case _PRE_DECR: 
        case _POST_INCR: 
        case _POST_DECR: 
        case _ARITH_NEG: // N -> N
            typecheck_expr(table, lhs);

            if(is_numctype(lhs) && !lhs->ctype.is_array) {
                set_ctypename(expr, lhs->ctype.name);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            break;
        case _LOG_NEG: // N -> char
            typecheck_expr(table, lhs);

            if(is_numctype(lhs) && !lhs->ctype.is_array) {
                set_ctypename(expr, "char");
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            break;
        case _BIT_NEG: // I -> I
            typecheck_expr(table, lhs);

            if(is_intctype(lhs) && !lhs->ctype.is_array) {
                set_ctypename(expr, lhs->ctype.name);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of integral type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            break;
        case _TYPE: // N -> type
            typecheck_expr(table, lhs);

            if(is_numctype(lhs) && !lhs->ctype.is_array) {
                set_ctypename(expr, "char");
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for type cast (%s) (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

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

            if(!is_numctype(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of integral type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            if(!is_numctype(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of integral type for operator %s (%s)\n", 
                    expr->text, rhs->ctype.name);
            }

            set_ctypename(expr, "char");

            break;
        case _ADD:
        case _SUB:
        case _MULT:
        case _DIV: // N x N -> N
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_numctype(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of numeric type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            if(!is_numctype(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of numeric type for operator %s (%s)\n", 
                    expr->text, rhs->ctype.name);
            }

            widen(rhs, lhs, expr);

            break;
        case _MOD:
        case _BIT_AND:
        case _BIT_OR: // I x I -> I
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_intctype(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of integral type for operator %s (%s)\n", 
                    expr->text, lhs->ctype.name);
            }

            if(!is_intctype(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of integral type for operator %s (%s)\n", 
                    expr->text, rhs->ctype.name);
            }

            widen(rhs, lhs, expr);

            break;
        case _FUN_CALL:
            funsym_t *funsym;

            funsym = get_function(table, expr->text);
            if(funsym == NULL) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tFunction %s called before declaration.\n", expr->text);
                exit(1);
            }

            astnode_t *param1;
            varsym_t *param2;
            param1 = expr->left;
            param2 = funsym->param;
            while(param1 != NULL && param2 != NULL) {
                typecheck_expr(table, param1);

                if(widen_to(param1, param2->type, param1)) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                    fprintf(stderr, "\tParameter mismatch in call to %s\n", expr->text);
                    fprintf(stderr, "\tArgument types incompatible (%s, %s)\n", 
                        param1->ctype.name, param2->type->ctype.name);
                } 

                param1 = param1->right;
                param2 = param2->next;
            }
            if(param1 != NULL || param2 != NULL) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tParameter number mismatch in call to %s\n", expr->text);
            }

            copy_ctype(funsym->ret_type, expr);

            break;
        case _ARR_ACCESS:
            typecheck_expr(table, lhs); // arr index
            typecheck_expr(table, rhs); // arr variable

            if(strcmp(lhs->ctype.name, "char") && strcmp(lhs->ctype.name, "int")) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tArray %s index not an integer (%s)\n", 
                    rhs->text, lhs->ctype.name);
            }

            set_ctypename(expr, rhs->ctype.name);

            break;
        case _STRUCT_ACCESS:
            break;
        case _VAR:
            varsym_t *varsym;

            varsym = get_localvar(table, expr->text);
            if(varsym == NULL) {
                varsym = get_globalvar(table, expr->text);
                if(varsym == NULL) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                    fprintf(stderr, "\tVariable %s referenced before declaration.\n", expr->text);
                    exit(1);
                }
            } 

            copy_ctype(varsym->type, expr);

            break;
        case _CHAR_LIT:
            set_ctypename(expr, "char");
            expr->ctype.is_const = true;
            break;
        case _INT_LIT:
            set_ctypename(expr, "int");
            expr->ctype.is_const = true;
            break;
        case _REAL_LIT:
            set_ctypename(expr, "float");
            expr->ctype.is_const = true;
            break;
        case _STR_LIT:
            set_ctypename(expr, "char");
            expr->ctype.is_array = true;
            expr->ctype.is_const = true;
            break;
        default: // fail
            break;
    }
}


/**
 *
 */
void
typecheck_statement(symtable_t *table, astnode_t *statement, bool output)
{
    astnode_t *cur;

    switch(statement->node_type) {
        case _BREAK:
        case _CONTINUE:
            break;
        case _RETURN:
            if(statement->left != NULL) {
                typecheck_expr(table, statement->left);
                if(widen_to(statement->left, table->ret_type, statement)) {
                    print_msg(TYPE_ERR, statement->filename, statement->line_num, 0, "", "");
                    fprintf(stderr, "\tReturn type does not match function definition (%s, %s)\n", 
                        statement->left->ctype.name, table->ret_type->text);
                }
            } else if(strcmp(table->ret_type->text, "void")) {
                print_msg(TYPE_ERR, statement->filename, statement->line_num, 0, "", "");
                fprintf(stderr, "\tReturn type does not match function definition (%s, %s)\n", 
                    statement->left->ctype.name, table->ret_type->text);
            }

            break;
        case _IF_STATEMENT:
            cur = statement->left->left; // if-cond
            typecheck_expr(table, cur);

            cur = statement->left->right->left; // if-body
            while(cur != NULL) {
                typecheck_statement(table, cur, output);
                cur = cur->right;
            }

            cur = statement->left->right->right->left; // else-body
            while(cur != NULL) {
                typecheck_statement(table, cur, output);
                cur = cur->right;
            }

            break;
        case _FOR_STATEMENT:
            cur = statement->left->left; // for-params
            while(cur != NULL) {
                typecheck_expr(table, cur);
                cur = cur->right;
            }
            
            cur = statement->left->right->left; // for-body
            while(cur != NULL) {
                typecheck_statement(table, cur, output);
                cur = cur->right;
            }

            break;
        case _WHILE_STATEMENT:
            cur = statement->left->left; // while-cond
            typecheck_expr(table, cur);
            
            cur = statement->left->right->left; // while-body
            while(cur != NULL) {
                typecheck_statement(table, cur, output);
                cur = cur->right;
            }
            
            break;
        case _DO_STATEMENT:
            cur = statement->left->right->left; // do-body
            while(cur != NULL) {
                typecheck_statement(table, cur, output);
                cur = cur->right;
            }

            cur = statement->left->right->left; // do-cond
            typecheck_expr(table, cur);
            
            break;
        default:
            typecheck_expr(table, statement);
            if(output) {
                fprintf(outfile, "\tLine %*d: expression has type ", 4, statement->line_num);
                print_ctype(statement);
                fprintf(outfile, "\n");
            }
            break; 
    }
}


/**
 *
 */
void
typecheck_localvardecl(symtable_t *table, astnode_t *var_decl, bool output)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    // check if struct type has been declared yet
    if(type->ctype.is_struct && get_localstruct(table, type->text) == NULL && 
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

            if(output) {
                fprintf(outfile, "\tLine %*d: local ", 4, var->line_num);
                print_var(type, var);
                fprintf(outfile, "\n");    
            }
            
        }

        if(var->left != NULL) { // includes initialization
            typecheck_expr(table, var->left);

            if(widen_to(var->left, type, var->left)) {
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
typecheck_globalvardecl(symtable_t *table, astnode_t *var_decl, bool output)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    // check if struct type has been declared yet
    if(type->ctype.is_struct && get_localstruct(table, type->text) == NULL && 
            get_globalstruct(table, type->text) == NULL) {
        print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", 
            "Struct type never declared.");  
    }

    while(var != NULL) {
        varsym_t *sym;
        sym = get_globalvar(table, var->text);

        if(sym != NULL) { // already declared
            print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", "");
            fprintf(stderr, "Global variable previously declared.\n");
        } else {
            add_globalvar(table, type, var);

            if(output) {
                fprintf(outfile, "Line %*d: global ", 4, var->line_num);
                print_var(type, var);
                fprintf(outfile, "\n");
            }
        }

        if(var->left != NULL && !var->ctype.is_array) { // includes initialization
            typecheck_expr(table, var->left);

            if(widen_to(var->left, type, var->left)) {
                print_msg(TYPE_ERR, var->filename, var->line_num, 0, "", "");
                fprintf(stderr, "Attempting to set variable to inconsistent type.\n");
            }
        }

        var = var->right;
    }
}


/**
 *
 */
void
typecheck_localtypedecl(symtable_t *table, astnode_t *type_decl, bool output)
{
    structsym_t *sym;
    sym = get_localstruct(table, type_decl->text);

    if(sym != NULL) { // struct already declared
        print_msg(TYPE_ERR, type_decl->filename, type_decl->line_num, 0, "", 
            "Local struct previously declared.");  
    } else {
        add_localstruct(table, type_decl);
        if(output) {
            fprintf(outfile, "\tLine %*d: local struct %s\n", 4, type_decl->line_num, type_decl->text);
        }
    }

    if(output) {
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
}


/**
 *
 */
void
typecheck_globaltypedecl(symtable_t *table, astnode_t *type_decl, bool output)
{
    structsym_t *sym;
    sym = get_globalstruct(table, type_decl->text);

    if(sym != NULL) { // struct already declared
        print_msg(TYPE_ERR, type_decl->filename, type_decl->line_num, 0, "", 
            "Global struct previously declared.");  
    } else {
        add_globalstruct(table, type_decl);
        if(output) {
            fprintf(outfile, "Line %*d: global struct %s\n", 4, type_decl->line_num, type_decl->text);
        }
    }

    if(output) {
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
}


/**
 *
 */
void
typecheck_funbody(symtable_t *table, astnode_t *fun_body, bool output)
{
    astnode_t *statement;

    statement = fun_body->left;
    while(statement != NULL) {
        if(statement->node_type == _VAR_DECL) {
            typecheck_localvardecl(table, statement, output);
        } else if(statement->node_type == _TYPE_DECL) {
            typecheck_localtypedecl(table, statement, output);
        } else {
            typecheck_statement(table, statement, output);
        }

        statement = statement->right;
    }
}


/**
 *
 */
void 
typecheck_fundecl(symtable_t *table, astnode_t *fun_decl, bool is_def, bool output)
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
                    "Function already defined");
                exit(1);
            } else {
                sym->is_def = true;
            }
        }

        // check return type
        if(!is_samectype(fun_decl->left, sym->ret_type)) {
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                "Function return type does not match previous declaration");
            exit(1);
        }

        // check params
        astnode_t *type1;
        varsym_t *type2;
        type1 = args->left;
        type2 = sym->param;

        while(type1 != NULL && type2 != NULL) {
            if(!is_samectype(type1, type2->var)) {
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Parameter type does not match previous declaration");
                exit(1);
            }

            if(!add_localvar(table, type1, type1->right)) {
                // parameter already exists
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Parameter name already in use");
                exit(1);
            }
            free_locals(table);

            type1 = type1->right->right;
            type2 = type2->next;
        }

        if((type1 == NULL && type2 != NULL) || (type1 != NULL && type2 == NULL)) {
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Number of function parameters does not match previous declaration");
            exit(1);
        }
          
    }

    if(output && is_def) {
        fprintf(outfile, "Line %*d: function ", 4, fun_decl->line_num);
        print_var(ret_type, ident);
        fprintf(outfile, "\n");   
    }

    astnode_t *param;
    param = args->left;

    while(param != NULL) {
        if(output && is_def) {
            fprintf(outfile, "\tLine %*d: parameter ", 4, param->line_num);
            print_var(param, param->right);
            fprintf(outfile, "\n");
        }

        if(!add_localvar(table, param, param->right)) {
            // parameter already exists
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                "Parameter name already in use");
            exit(1);
        }

        param = param->right->right;
    }
}


/**
 *
 */
 void
typecheck_fundef(symtable_t *table, astnode_t *fun_def, bool output)
{
    astnode_t *fun_decl, *fun_body;
    fun_decl = fun_def->left;
    fun_body = fun_decl->right;

    typecheck_fundecl(table, fun_decl, true, output);
    typecheck_funbody(table, fun_body, output);

    free_locals(table);
    table->ret_type = NULL;
}


/**
 *
 */
void
typecheck_program(symtable_t *table, astnode_t *program, bool output)
{
    astnode_t *global;
    global = program->left;

    while(global != NULL) {
        switch(global->node_type) {
            case _TYPE_DECL:
                typecheck_globaltypedecl(table, global, output);
                break;
            case _VAR_DECL:
                typecheck_globalvardecl(table, global, output);
                break;
            case _FUN_DECL:
                typecheck_fundecl(table, global, false, output);
                free_locals(table);
                break;
            case _FUN_DEF:
                typecheck_fundef(table, global, output);
                break;
            default: // impossible
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

    typecheck_program(table, program, true);

    free_symtable(table);

    return 0;
}