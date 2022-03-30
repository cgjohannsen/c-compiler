#include <stdbool.h>
#include <string.h>

#include "../util/io.h"
#include "parse.h"
#include "symtable.h"
#include "typecheck.h"

bool
is_sametype(astnode_t *type1, astnode_t *type2)
{
    return type1->is_const == type2->is_const &&
           type1->is_array == type2->is_array && 
           type1->is_struct == type2->is_struct &&
           !strcmp(type1->text, type2->text);
}


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
typecheck_statement(symtable_t *table, astnode_t *statement)
{
    
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

    if(!add_function(table, fun_decl, is_def)) {
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

            type1 = type1->right->right;
            type2 = type2->next;
        }

        if((type1 == NULL && type2 != NULL) || (type1 != NULL && type2 == NULL)) {
            print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Number of function parameters does not match previous declaration.");
        }
          
    } else { // new function declaration
        fprintf(outfile, "Line %*d: function ", 4, fun_decl->line_num);
        print_var(ret_type, ident);
        fprintf(outfile, "\n");   

        astnode_t *param;
        param = args->left;

        while(param != NULL) {
            fprintf(outfile, "\tLine %*d: parameter ", 4, param->line_num);
            print_var(param, param->right);
            fprintf(outfile, "\n");

            if(!add_localvar(table, param, param->right)) {
                // parameter already exists
                print_msg(TYPE_ERR, fun_decl->filename, fun_decl->line_num, 0, "", 
                    "Parameter name already in use.");
            }

            param = param->right->right;
        } 

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