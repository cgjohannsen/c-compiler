#include <string.h>

#include "util/io.h"
#include "parse/parse.h"
#include "parse/symtable.h"
#include "parse/typecheck.h"
#include "parse/ast.h"
#include "instruction.h"

#include "gen.h"


// Global variables
char *classname, *filename;


char *
get_basefilename(char *filename)
{
    char *base, *pch1, *pch2;

    base = (char *) malloc(sizeof(char) * strlen(filename));

    pch1 = strchr(filename, '.');
    pch2 = strrchr(filename, '/');
    
    if(pch2 == NULL) {
        strncpy(base, filename, pch1-filename);
        base[pch1-filename+1] = '\0'; // manually add NULL character
    } else {
        strncpy(base, pch2+1, pch1-filename);
        base[pch1-filename+1] = '\0'; // manually add NULL character
    }

    return base;
}


char
get_javatype(char *type) 
{
    if(!strcmp(type, "char")) {
        return 'C';
    } else if(!strcmp(type, "int")) {
        return 'I';
    } else if(!strcmp(type, "float")) {
        return 'F';
    } else {
        return 'V';
    }
}


void
print_instrlist(instrlist_t *list)
{
    instr_t *cur;
    cur = list->head;

    while(cur != NULL) {
        fprintf(outfile, "\t\t%s\n", cur->text);
        cur = cur->next;
    }
}



/** 
 * returns minimum required stack size
 */ 
int
gen_expr(symtable_t *table, astnode_t *expr, instrlist_t *list)
{


    // append instructions to list

    switch(expr->node_type) {
        case _ITE:
            break;
        case _ASSIGN: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _INCR: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _DECR: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _ARITH_NEG: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _LOG_NEG: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _BIT_NEG: // TODO 
            gen_expr(table, expr->left, list);
            break;
        case _TYPE: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _EQ:
        case _NEQ:
        case _GEQ:
        case _GT:
        case _LEQ:
        case _LT:
            break;
        case _LOG_AND: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _LOG_OR: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _ADD: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);

            break;
        case _SUB: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _MULT: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _DIV: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _MOD: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _BIT_AND: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _BIT_OR: // TODO
            gen_expr(table, expr->left, list);
            gen_expr(table, expr->right, list);
            break;
        case _FUN_CALL: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _ARR_ACCESS: // TODO
            gen_expr(table, expr->left, list);
            break;
        case _STRUCT_ACCESS:
            break;
        case _VAR: // TODO
            break;
        case _CHAR_LIT: // TODO

            break;
        case _INT_LIT: // TODO
            break;
        case _REAL_LIT: // TODO
            break;
        case _STR_LIT: // TODO
            break;
        default: // fail
            break;
    }
}



void
gen_statement(symtable_t *table, astnode_t *statement, instrlist_t *list)
{
    switch(statement->node_type) {
        case _BREAK:
        case _CONTINUE:
        case _RETURN:
        case _IF_STATEMENT:
        case _FOR_STATEMENT:
        case _WHILE_STATEMENT:
        case _DO_STATEMENT:
            break;
        default:
            fprintf(outfile, "\t\t;; expression %s %d\n", filename, 
                statement->line_num);
            gen_expr(table, statement, list);
            break; 
    }
}



void
gen_funbody(symtable_t *table, astnode_t *fun_body, instrlist_t *list)
{
    astnode_t *statement;

    statement = fun_body->left;
    while(statement != NULL) {
        if(statement->node_type == _VAR_DECL) {
            //gen_localvar(table, statement, list);
            ;
        } else if(statement->node_type == _TYPE_DECL) {
            ; // ignore
        } else {
            gen_expr(table, statement, list);
        }

        statement = statement->right;
    }
}



void
gen_fun(symtable_t *table, astnode_t *fun_def)
{
    astnode_t *fun_decl, *fun_body, *ret_type, *ident, *args;
    instrlist_t *list;

    fun_decl = fun_def->left;
    fun_body = fun_decl->right;
    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;
    
    list = init_instrlist();

    // print function header
    fprintf(outfile, ".method public static %s : (", ident->text);
    astnode_t *arg_type;
    arg_type = args->left;

    // print argument types
    while(arg_type != NULL) {
        if(arg_type->right->ctype.is_array) {
            fprintf(outfile, "[");
        }
        fprintf(outfile, "%c", get_javatype(arg_type->text));        
        arg_type = arg_type->right->right;
    }
    
    // print return type
    if(ret_type->ctype.is_array) {
        fprintf(outfile, "[");
    }
    fprintf(outfile, ")%c\n", get_javatype(ret_type->text));

    table->ret_type = ret_type;

    // construct instruction list
    gen_funbody(table, fun_body, list);

    // print stack and local info
    fprintf(outfile, "\t.code stack %d locals %d\n", list->stack_size, 
        list->num_locals);

    // print instruction list
    print_instrlist(list);

    // print function footer
    fprintf(outfile, "\t.end code\n.end method\n\n");

    free_instrlist(list);
}



void
gen_globalvar(symtable_t *table, astnode_t *var_decl)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    char java_type = get_javatype(type->text);

    while(var != NULL) {
        fprintf(outfile, ".field public static %s %c%c\n", var->text, 
            var->ctype.is_array ? '[' : ' ' , java_type);
        var = var->right;
    }
}



void
gen_program(symtable_t *table, astnode_t *program)
{
    astnode_t *global;
    global = program->left;

    while(global != NULL) {
        switch(global->node_type) {
            case _VAR_DECL:
                gen_globalvar(table, global);
                break;
            case _FUN_DEF:
                gen_fun(table, global);
                break;
            default: // ignore all others
                break;
        }
        global = global->right;
    }
}



void
gen_clinit(symtable_t *table)
{
    instrlist_t *list;
    varsym_t *global;
    astnode_t *var, *type;
    char buffer[256];

    list = init_instrlist();

    global = table->global_vars;

    if(global == NULL) { // no globals, no need for clinit
        return;
    }

    while(global != NULL) {
        var = global->var;
        type = global->type;

        if(var->left != NULL) {
            if(var->ctype.is_array) { // global array
                sprintf(buffer, "bipush %s", var->left->text);
                add_instr(list, BIPUSH, buffer, 0);

                sprintf(buffer, "newarray %s", type->text);
                add_instr(list, NEWARRAY, buffer, 0);

                sprintf(buffer, "putstatic Field %s %s [%c", classname, 
                    var->text, get_javatype(type->text));
                add_instr(list, PUTSTATIC, buffer, 0);
            } else { // initialized global
                gen_expr(table, var->left, list);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    var->text, get_javatype(type->text));
                add_instr(list, PUTSTATIC, buffer, 0);
            }
        }

        global = global->next;
    }

    fprintf(outfile, ".method <clinit> : ()V\n");
    fprintf(outfile, "\t.code stack %d locals 0\n", list->min_stack_size);
    print_instrlist(list);
    fprintf(outfile, "\t.end code\n.end method\n\n");

    free_instrlist(list);
}



void
gen_header()
{
    fprintf(outfile, ".class public %s\n.super java/lang/Object\n\n", 
        classname);
}



void
gen_footer(symtable_t *table)
{
    fprintf(outfile, "; Special methods\n\n");

    gen_clinit(table);

    fprintf(outfile,
        ".method <init> : ()V\n"
            "\t.code stack 1 locals 1\n"
                "\t\taload_0\n"
                "\t\tinvokespecial Method java/lang/Object <init> ()V\n"
                "\t\treturn\n"
            "\t.end code\n"
        ".end method\n\n"
        ".method public static main : ([Ljava/lang/String;)V\n"
            "\t.code stack 2 locals 2\n"
                "\t\tinvokestatic Method %s main ()I\n"
                "\t\tistore_1\n"
                "\t\tgetstatic Field java/lang/System out Ljava/io/PrintStream;\n"
                "\t\tldc 'Return code: '\n"
                "\t\tinvokevirtual Method java/io/PrintStream print (Ljava/lang/String;)V\n"
                "\t\tgetstatic Field java/lang/System out Ljava/io/PrintStream;\n"
                "\t\tiload_1\n"
                "\t\tinvokevirtual Method java/io/PrintStream println (I)V\n"
                "\t\treturn\n"
            "\t.end code\n"
        ".end method\n", classname);
}



int 
gen_code(char *fname)
{
    symtable_t *table;
    parser_t parser;
    astnode_t *program;

    // initialize filename stuff
    filename = (char *) malloc(sizeof(char) * strlen(fname) + 1);
    strcpy(filename, fname);
    classname = get_basefilename(filename);

    table = init_symtable();
    init_parser(filename, &parser);
    program = parse_program(&parser);

    typecheck_program(table, program, false);

    gen_header();

    gen_program(table, program);

    gen_footer(table);

    free_symtable(table);
    free(filename);
    free(classname);

    return 0;
}
