#include "../util/io.h"
#include "parse.h"
#include "symtable.h"
#include "typecheck.h"


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


void
check_typeequiv(astnode_t *type1, astnode_t *type2) 
{
    
}


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
        }

        statement = statement->right;
    }
}




void 
typecheck_fundecl(symtable_t *table, astnode_t *fun_decl)
{
    astnode_t *ret_type, *ident, *args, *param, *type, *var;

    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;

    /*
    funsym_t *sym;
    sym = get_function(table, ident->text);

    if(sym == NULL) {
        add_function(table, fun_decl);

        fprintf(outfile, "Line %*d: function ", 4, ret_type->line_num);
        print_var(ret_type, ident);
        fprintf(outfile, "\n");   
    }
    */

    fprintf(outfile, "Line %*d: function ", 4, ret_type->line_num);
    print_var(ret_type, ident);
    fprintf(outfile, "\n");

    param = args->left;
    while(param != NULL) {
        type = param;
        var = type->right;

        fprintf(outfile, "\tLine %*d: parameter ", 4, type->line_num);
        print_var(type, var);
        fprintf(outfile, "\n");

        param = var->right;
    }
}




void
typecheck_fundef(symtable_t *table, astnode_t *fun_def)
{
    astnode_t *fun_decl, *fun_body;
    fun_decl = fun_def->left;
    fun_body = fun_decl->right;

    typecheck_fundecl(table, fun_decl);
    typecheck_funbody(table, fun_body);
}




void
typecheck_funproto(symtable_t *table, astnode_t *fun_decl)
{
    return;

    /*
    funsym_t *sym;
    sym = get_function(table, ident->text);

    if(sym == NULL) {
        add_function(table, fun_decl);

        fprintf(outfile, "Line %*d: function ", 4, ret_type->line_num);
        print_var(ret_type, ident);
        fprintf(outfile, "\n");   
    } else { // function already declared
        // check that params match

        // check return type
        check_typeequiv(fun_decl->left, sym->ret_type);

        // check params
        astnode_t *param1, *param2;
        param1 = fun_decl->left->right->right->left;
        param2 = sym->param;

        while(param1 != NULL && param2 != NULL) {
            check_typeequiv(param1, param2->type);

            param1 = param1->right->right;
            param2 = param2->next;
        }

        if((param1 == NULL && param2 != NULL) || (param1 != NULL && param2 == NULL)) {
            // unmatched number of args
        }
    }
    */
}




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
                typecheck_funproto(table, global);
                break;
            case _FUN_DEF:
                typecheck_fundef(table, global);
                break;
        }
        global = global->right;
    }
}


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