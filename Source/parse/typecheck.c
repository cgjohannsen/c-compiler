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
is_numeric(astnode_t *type)
{
    return (type->exprtype == __CHAR || type->exprtype == __INT || 
        type->exprtype == __REAL) && !type->is_array;
}

bool
is_integral(astnode_t *type)
{
    return (type->exprtype == __CHAR || type->exprtype == __INT) &&
        !type->is_array;
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
    if(expr->is_const) {
        fprintf(outfile, "const ");
    }

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
char *
str_exprtype(astnode_t *expr)
{
    switch(expr->exprtype) {
        case __CHAR:
            return expr->is_array ? "char[]" : "char" ;
        case __INT:
            return expr->is_array ? "int[]" : "int" ;
        case __REAL:
            return expr->is_array ? "float[]" : "float" ;
        case __STRING:
            return "char[]";
        case __STRUCT:
            return "struct";
        default:
            return "none";
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

            if(!is_numeric(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tFirst argument %s of ternary not numeric type (%s)\n", 
                    lhs->text, str_exprtype(lhs));
            }

            if(rhs->exprtype == rhs2->exprtype) {
                expr->exprtype = rhs->exprtype;
            } else if(widen(rhs->exprtype, rhs2->exprtype) != __NONE && !rhs->is_array && !rhs2->is_array) {
                expr->exprtype = widen(rhs->exprtype, rhs2->exprtype);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tArguments %s, %s of ternary of incomptaible types (%s, %s)\n", 
                    rhs->text, rhs2->text, str_exprtype(rhs), str_exprtype(rhs2));
            }

            break;
        case _ASSIGN:
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);
            
            if(lhs->is_const) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tAttempting to change value of const variable (%s)\n", lhs->text);
            }

            if(widen_to(rhs->exprtype, lhs->exprtype) != __NONE && !lhs->is_array && !rhs->is_array) {
                expr->exprtype = widen_to(rhs->exprtype, lhs->exprtype);
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tAttempting to assign variable %s to incompatible type (%s, %s)\n", 
                    lhs->text, str_exprtype(lhs), str_exprtype(rhs));
            }

            break;
        case _INCR: 
        case _DECR: 
        case _ARITH_NEG: // N -> N
            typecheck_expr(table, lhs);

            if(is_numeric(lhs) && !lhs->is_array) {
                expr->exprtype = lhs->exprtype;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            break;
        case _LOG_NEG: // N -> char
            typecheck_expr(table, lhs);

            if(is_numeric(lhs) && !lhs->is_array) {
                expr->exprtype = __CHAR;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            break;
        case _BIT_NEG: // I -> I
            typecheck_expr(table, lhs);

            if(is_integral(lhs) && !lhs->is_array) {
                expr->exprtype = lhs->exprtype;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of integral type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            break;
        case _TYPE: // N -> type
            typecheck_expr(table, lhs);

            if(is_numeric(lhs) && !lhs->is_array) {
                expr->exprtype = __CHAR;
            } else {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tOperand not of numeric type for type cast (%s) (%s)\n", 
                    expr->text, str_exprtype(lhs));
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

            if(!is_numeric(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of integral type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            if(!is_numeric(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of integral type for operator %s (%s)\n", 
                    expr->text, str_exprtype(rhs));
            }

            expr->exprtype = __CHAR;

            break;
        case _ADD:
        case _SUB:
        case _MULT:
        case _DIV: // N x N -> N
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_numeric(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of numeric type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            if(!is_numeric(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of numeric type for operator %s (%s)\n", 
                    expr->text, str_exprtype(rhs));
            }

            expr->exprtype = widen(lhs->exprtype, rhs->exprtype);

            break;
        case _MOD:
        case _BIT_AND:
        case _BIT_OR: // I x I -> I
            typecheck_expr(table, lhs);
            typecheck_expr(table, rhs);

            if(!is_integral(lhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tLHS not of integral type for operator %s (%s)\n", 
                    expr->text, str_exprtype(lhs));
            }

            if(!is_integral(rhs)) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tRHS not of integral type for operator %s (%s)\n", 
                    expr->text, str_exprtype(rhs));
            }

            expr->exprtype = widen(lhs->exprtype, rhs->exprtype);

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

                if(widen(param1->exprtype, to_exprtype(param2->type->text)) == __NONE) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                    fprintf(stderr, "\tParameter mismatch in call to %s.\n", expr->text);
                    fprintf(stderr, "\tArgument types incompatible (%s, %s)\n", 
                        str_exprtype(param1), str_exprtype(param2->var));
                } else {
                    param1->exprtype = widen(param1->exprtype, to_exprtype(param2->type->text));
                }

                param1 = param1->right;
                param2 = param2->next;
            }
            if(param1 != NULL || param2 != NULL) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tParameter number mismatch in call to %s.\n", expr->text);
            }

            expr->exprtype = to_exprtype(funsym->ret_type->text);

            break;
        case _ARR_ACCESS:
            typecheck_expr(table, lhs); // arr index
            typecheck_expr(table, rhs); // arr variable

            if(widen_to(lhs->exprtype, __INT) == __NONE) {
                print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                fprintf(stderr, "\tArray %s index not an integer (%s)\n", 
                    rhs->text, str_exprtype(lhs));
            }

            expr->exprtype = rhs->exprtype;

            break;
        case _STRUCT_ACCESS:
            break;
        case _VAR:
            varsym_t *varsym;

            varsym = get_globalvar(table, expr->text);
            if(varsym == NULL) {
                varsym = get_localvar(table, expr->text);
                if(varsym == NULL) {
                    print_msg(TYPE_ERR, expr->filename, expr->line_num, 0, "", "");
                    fprintf(stderr, "\tVariable %s referenced before declaration.\n", expr->text);
                    exit(1);
                }
            } 

            expr->is_array = varsym->var->is_array;
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
        case _STR_LIT:
            expr->exprtype = __STRING;
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

    switch(statement->type) {
        case _BREAK:
        case _CONTINUE:
            break;
        case _RETURN:
            typecheck_expr(table, statement->left);
            if(!is_sametype(statement->left, table->ret_type)) {
                print_msg(TYPE_ERR, statement->filename, statement->line_num, 0, "", "");
                fprintf(stderr, "Return type does not match function definition (%s, %s)", 
                    str_exprtype(statement->left), table->ret_type->text);
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
                print_exprtype(statement);
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

            if(output) {
                fprintf(outfile, "\tLine %*d: local ", 4, var->line_num);
                print_var(type, var);
                fprintf(outfile, "\n");    
            }
            
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
typecheck_globalvardecl(symtable_t *table, astnode_t *var_decl, bool output)
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

        if(var->left != NULL && !var->is_array) { // includes initialization
            typecheck_expr(table, var->left);

            if(widen_to(var->left->exprtype, to_exprtype(type->text)) == __NONE) {
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
        if(statement->type == _VAR_DECL) {
            typecheck_localvardecl(table, statement, output);
        } else if(statement->type == _TYPE_DECL) {
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
                "Parameter name already in use.");
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
        switch(global->type) {
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