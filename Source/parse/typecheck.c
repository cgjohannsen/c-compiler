
#include "parse.h"
#include "typecheck.h"


void
print_var(astnode_t *type, astnode_t *var, FILE *outfile)
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
typecheck_localvardecl(astnode_t *var_decl, FILE *outfile)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    while(var != NULL) {
        fprintf(outfile, "\tLine %*d: local ", 4, var->line_num);
        print_var(type, var, outfile);
        fprintf(outfile, "\n");

        // add var to local symbol table

        var = var->right;
    }
}





void
typecheck_globalvardecl(astnode_t *var_decl, FILE *outfile)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    while(var != NULL) {
        fprintf(outfile, "Line %*d: global ", 4, var->line_num);
        print_var(type, var, outfile);
        fprintf(outfile, "\n");

        // add var to global symbol table

        var = var->right;
    }
}




void
typecheck_localtypedecl(astnode_t *type_decl, FILE *outfile)
{
    fprintf(outfile, "\tLine %*d: local struct %s\n", 4, type_decl->line_num, type_decl->text);

    // add struct to global struct symbol tble


    astnode_t *var_decl, *type, *var;
    var_decl = type_decl->left;

    while(var_decl != NULL) { // cycle thru member declarations
        type = var_decl->left;
        var = type->right;

        while(var != NULL) { // cycle thru each variable
            fprintf(outfile, "\t\tLine %*d: member ", 4, var->line_num);
            print_var(type, var, outfile);
            fprintf(outfile, "\n");
            
            // add var as member to struct in symbol table

            var = var->right;
        }

        var_decl = var_decl->right;
    } 
}




void
typecheck_globaltypedecl(astnode_t *type_decl, FILE *outfile)
{
    fprintf(outfile, "Line %*d: global struct %s\n", 4, type_decl->line_num, type_decl->text);
    
    // add struct to global struct symbol tble

    astnode_t *var_decl, *type, *var;
    var_decl = type_decl->left;

    while(var_decl != NULL) { // cycle thru member declarations
        type = var_decl->left;
        var = type->right;

        while(var != NULL) { // cycle thru each variable
            fprintf(outfile, "\tLine %*d: member ", 4, var->line_num);
            print_var(type, var, outfile);
            fprintf(outfile, "\n");
            
            // add var as member to struct in symbol table

            var = var->right;
        }

        var_decl = var_decl->right;
    } 
}



void
typecheck_funbody(astnode_t *fun_body, FILE *outfile)
{
    astnode_t *statement;

    statement = fun_body->left;
    while(statement != NULL) {
        if(statement->type == _VAR_DECL) {
            typecheck_localvardecl(statement, outfile);
        } else if(statement->type == _TYPE_DECL) {
            typecheck_localtypedecl(statement, outfile);
        }

        statement = statement->right;
    }
}




void 
typecheck_fundecl(astnode_t *fun_decl, FILE *outfile)
{
    astnode_t *ret_type, *ident, *args, *param, *type, *var;

    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;

    fprintf(outfile, "Line %*d: function ", 4, ret_type->line_num);
    print_var(ret_type, ident, outfile);
    fprintf(outfile, "\n");

    // add function to symbol table

    param = args->left;
    while(param != NULL) {
        type = param;
        var = type->right;

        fprintf(outfile, "\tLine %*d: parameter ", 4, type->line_num);
        print_var(type, var, outfile);
        fprintf(outfile, "\n");

        param = var->right;
    }
}




void
typecheck_fundef(astnode_t *fun_def, FILE *outfile)
{
    astnode_t *fun_decl, *fun_body;
    fun_decl = fun_def->left;
    fun_body = fun_decl->right;

    typecheck_fundecl(fun_decl, outfile);
    typecheck_funbody(fun_body, outfile);
}




void
typecheck_funproto(astnode_t *fun_proto, FILE *outfile)
{
    // add function to symbol table
}




void
typecheck_program(astnode_t *program, FILE *outfile)
{
    astnode_t *global;
    global = program->left;


    while(global != NULL) {
        switch(global->type) {
            case _TYPE_DECL:
                typecheck_globaltypedecl(global, outfile);
                break;
            case _VAR_DECL:
                typecheck_globalvardecl(global, outfile);
                break;
            case _FUN_DECL:
                typecheck_funproto(global, outfile);
                break;
            case _FUN_DEF:
                typecheck_fundef(global, outfile);
                break;
        }
        global = global->right;
    }
}


int 
typecheck(char *infilename, FILE *outfile)
{
    parser_t parser;
    astnode_t *program;

    init_parser(infilename, &parser);
    program = parse_program(&parser);

    typecheck_program(program, outfile);

    return 0;
}