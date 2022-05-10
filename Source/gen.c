#include <string.h>
#include <stdlib.h>

#include "util/io.h"
#include "parse/parse.h"
#include "parse/symtable.h"
#include "parse/typecheck.h"
#include "parse/ast.h"
#include "instruction.h"

#include "gen.h"


// Global variables
char *classname, *filename;
char buffer[2048];
int mode;
int loop_depth = 0;


char *
get_classname(char *filename)
{
    char *class, *pch1, *pch2;

    class = (char *) malloc(strlen(filename));

    pch1 = strchr(filename, '.');
    pch2 = strrchr(filename, '/');
    
    if(pch2 == NULL) {
        strncpy(class, filename, pch1-filename);
        class[pch1-filename] = '\0'; // manually add NULL character
    } else {
        strncpy(class, pch2+1, pch1-filename);
        class[pch1-pch2-1] = '\0'; // manually add NULL character
    }

    return class;
}



char
get_ajavatype(char *type) 
{
    if(!strcmp(type, "char")) {
        return 'c';
    } else if(!strcmp(type, "int")) {
        return 'i';
    } else if(!strcmp(type, "float")) {
        return 'f';
    } else {
        return 'v';
    }
}

char
get_javatype(char *type) 
{
    if(!strcmp(type, "char") || !strcmp(type, "int")) {
        return 'i';
    } else if(!strcmp(type, "float")) {
        return 'f';
    } else {
        return 'v';
    }
}


char
get_staticjavatype(char *type) 
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
        if(cur->type != LABEL) {
            fprintf(outfile, "\t");
        }
        fprintf(outfile, "\t%s\n", cur->text);
        cur = cur->next;
    }
}



/** 
 *
 */ 
void
gen_expr(symtable_t *table, astnode_t *expr, instrlist_t *list, bool is_top)
{
    // append instructions to list
    astnode_t *lhs, *rhs, *rhs2;
    varsym_t *sym;
    char java_type;

    if(expr == NULL) {
        return;
    }

    lhs = NULL;
    rhs = NULL;

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
            break;
        case _ASSIGN:

            // will store value at top of stack in LHS
            // 4 cases to handle: is_local X is_array

            sym = get_localvar(table, lhs->text);
            if(sym != NULL) { // local

                if(sym->var->ctype.is_array) { // local array
                    java_type = get_ajavatype(sym->type->ctype.name);

                    // load array address
                    sprintf(buffer, "aload%c%d ; load from %s",  
                        (sym->idx > 3) ? ' ' : '_', sym->idx,
                        sym->var->text);
                    add_instr(list, LOAD, buffer, 0);

                    // get index value
                    gen_expr(table, lhs->left, list, false);

                    // get RHS value
                    gen_expr(table, rhs, list, false);

                    // if not top-level, will need result for later
                    if(!is_top) {
                        sprintf(buffer, "dup_x2");
                        add_instr(list, DUP, buffer, 0);
                    }

                    // store value
                    sprintf(buffer, "%castore ; store to %s", java_type, 
                        sym->var->text);
                    add_instr(list, STORE, buffer, 0);
                } else { // local var
                    java_type = get_javatype(sym->type->ctype.name);

                    gen_expr(table, rhs, list, false);

                    // if not top-level, will need result for later
                    if(!is_top) {
                        sprintf(buffer, "dup");
                        add_instr(list, DUP, buffer, 0);
                    }

                    sprintf(buffer, "%cstore%c%d ; store to %s", java_type, 
                        (sym->idx > 3) ? ' ' : '_', sym->idx,
                        sym->var->text);
                    add_instr(list, STORE, buffer, 0);
                }

            } else { // global
                sym = get_globalvar(table, lhs->text);
                java_type = get_staticjavatype(sym->type->ctype.name);

                if(sym->var->ctype.is_array) { // global array
                    // load array address
                    sprintf(buffer, "getstatic Field %s %s [%c", classname,
                        sym->var->text, java_type);
                    add_instr(list, GETSTATIC, buffer, 0);

                    // get index value
                    gen_expr(table, lhs->left, list, false);

                    // get RHS value
                    gen_expr(table, rhs, list, false);

                    // if not top-level, will need result for later
                    if(!is_top) {
                        sprintf(buffer, "dup_x2");
                        add_instr(list, DUP, buffer, 0);
                    }
                    java_type = get_ajavatype(sym->type->ctype.name);

                    // store value
                    sprintf(buffer, "%castore ; store to %s", java_type, 
                        sym->var->text);
                    add_instr(list, STORE, buffer, 0);
                } else { // global var
                    gen_expr(table, rhs, list, false);

                    // if not top-level, will need result for later
                    if(!is_top) {
                        sprintf(buffer, "dup");
                        add_instr(list, DUP, buffer, 0);
                    }

                    sprintf(buffer, "putstatic Field %s %s %c", classname, 
                        lhs->text, java_type);
                    add_instr(list, PUTSTATIC, buffer, 0);
                }
            }
 
            break;
        case _ARITH_NEG: // TODO
            gen_expr(table, lhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cneg", java_type);
            add_instr(list, NEG, buffer, 0);

            break;
        case _LOG_NEG: // TODO
            gen_expr(table, lhs, list, false);
            break;
        case _BIT_NEG: // TODO 
            gen_expr(table, lhs, list, false);
            break;
        case _TYPE: // TODO
            gen_expr(table, lhs, list, false);

            if(!strcmp(expr->ctype.name,"int") && !strcmp(lhs->ctype.name,"float")) {
                sprintf(buffer, "f2i", java_type);
                add_instr(list, F2I, buffer, 0);
            } else if(!strcmp(expr->ctype.name,"float") && !strcmp(lhs->ctype.name,"int")) {
                sprintf(buffer, "i2f", java_type);
                add_instr(list, I2F, buffer, 0);
            }

            break;
        case _EQ:
        case _NEQ:
        case _GEQ:
        case _GT:
        case _LEQ:
        case _LT:
            break;
        case _LOG_AND: // TODO
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);
            break;
        case _LOG_OR: // TODO
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);
            break;
        case _ADD:
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cadd", java_type);
            add_instr(list, ADD, buffer, 0);
            
            break;
        case _SUB:
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%csub", java_type);
            add_instr(list, SUB, buffer, 0);

            break;
        case _MULT:
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cmul", java_type);
            add_instr(list, MUL, buffer, 0);

            break;
        case _DIV:
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cdiv", java_type);
            add_instr(list, DIV, buffer, 0);

            break;
        case _MOD: 
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%crem", java_type);
            add_instr(list, REM, buffer, 0);

            break;
        case _BIT_AND: // TODO
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cand", java_type);
            add_instr(list, AND, buffer, 0);

            break;
        case _BIT_OR: // TODO
            gen_expr(table, lhs, list, false);
            gen_expr(table, rhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cor", java_type);
            add_instr(list, AND, buffer, 0);

            break;
        case _PRE_INCR:
            gen_expr(table, lhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cconst_1", java_type);
            if(java_type == 'i') {
                add_instr(list, ICONST, buffer, 0);
            } else {
                add_instr(list, FCONST, buffer, 0);
            }
            sprintf(buffer, "%cadd", java_type);
            add_instr(list, ADD, buffer, 0);
            sprintf(buffer, "dup", java_type);
            add_instr(list, DUP, buffer, 0);

            sym = get_localvar(table, lhs->text);
            if(sym != NULL) {
                sprintf(buffer, "%cstore%c%d ; store to %s", java_type, 
                    sym->idx > 3 ? ' ' : '_', sym->idx, lhs->text);
                add_instr(list, STORE, buffer, 0);
            } else {
                sym = get_globalvar(table, lhs->text);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    lhs->text, get_staticjavatype(sym->type->ctype.name));
                add_instr(list, PUTSTATIC, buffer, 0);
            }

            break;
        case _PRE_DECR:
            gen_expr(table, lhs, list, false);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cconst_1", java_type);
            if(java_type == 'i') {
                add_instr(list, ICONST, buffer, 0);
            } else {
                add_instr(list, FCONST, buffer, 0);
            }
            sprintf(buffer, "%csub", java_type);
            add_instr(list, SUB, buffer, 0);
            sprintf(buffer, "dup", java_type);
            add_instr(list, DUP, buffer, 0);

            sym = get_localvar(table, lhs->text);
            if(sym != NULL) {
                sprintf(buffer, "%cstore%c%d ; store to %s", java_type, 
                    sym->idx > 3 ? ' ' : '_', sym->idx, lhs->text);
                add_instr(list, STORE, buffer, 0);
            } else {
                sym = get_globalvar(table, lhs->text);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    lhs->text, get_staticjavatype(sym->type->ctype.name));
                add_instr(list, PUTSTATIC, buffer, 0);
            }

            break;
        case _POST_INCR:
            gen_expr(table, lhs, list, false);
            
            sprintf(buffer, "dup", java_type);
            add_instr(list, DUP, buffer, 0);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cconst_1", java_type);
            if(java_type == 'i') {
                add_instr(list, ICONST, buffer, 0);
            } else {
                add_instr(list, FCONST, buffer, 0);
            }
            sprintf(buffer, "%cadd", java_type);
            add_instr(list, ADD, buffer, 0);

            sym = get_localvar(table, lhs->text);
            if(sym != NULL) {
                sprintf(buffer, "%cstore%c%d ; store to %s", java_type, 
                    sym->idx > 3 ? ' ' : '_', sym->idx, lhs->text);
                add_instr(list, STORE, buffer, 0);
            } else {
                sym = get_globalvar(table, lhs->text);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    lhs->text, get_staticjavatype(sym->type->ctype.name));
                add_instr(list, PUTSTATIC, buffer, 0);
            }

            break;
        case _POST_DECR:
            gen_expr(table, lhs, list, false);

            sprintf(buffer, "dup", java_type);
            add_instr(list, DUP, buffer, 0);

            java_type = get_javatype(expr->ctype.name);

            sprintf(buffer, "%cconst_1", java_type);
            if(java_type == 'i') {
                add_instr(list, ICONST, buffer, 0);
            } else {
                add_instr(list, FCONST, buffer, 0);
            }
            sprintf(buffer, "%csub", java_type);
            add_instr(list, SUB, buffer, 0);

            sym = get_localvar(table, lhs->text);
            if(sym != NULL) {
                sprintf(buffer, "%cstore%c%d ; store to %s", java_type, 
                    sym->idx > 3 ? ' ' : '_', sym->idx, lhs->text);
                add_instr(list, STORE, buffer, 0);
            } else {
                sym = get_globalvar(table, lhs->text);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    lhs->text, get_staticjavatype(sym->type->ctype.name));
                add_instr(list, PUTSTATIC, buffer, 0);
            }

            break;
        case _FUN_CALL:
    
            funsym_t *funsym;
            astnode_t *arg;
            int num_params;

            funsym = get_function(table, expr->text);
            arg = lhs;

            // if has return type, will push something onto stack
            num_params = (funsym->ret_type != NULL) ? -1 : 0;

            while(arg != NULL) {
                gen_expr(table, arg, list, false);
                arg = arg->right;
                num_params += 1;
            }            

            // build instruction

            // if getchar/putchar, classname is libc
            if(!strcmp(expr->text, "putchar") || !strcmp(expr->text, "getchar")) {
                sprintf(buffer, "invokestatic Method libc %s (", expr->text);
            } else {
                sprintf(buffer, "invokestatic Method %s %s (", classname, 
                    expr->text);
            }

            varsym_t *param;
            param = funsym->param;

            int instr_len = strlen(buffer);
            while(param != NULL) {
                java_type = get_staticjavatype(param->type->ctype.name);
                if(param->var->ctype.is_array) {
                    buffer[instr_len] = '[';
                    instr_len += 1;
                }
                buffer[instr_len] = java_type;
                instr_len += 1;
                param = param->next;
            }
            buffer[instr_len] = ')';

            java_type = get_staticjavatype(funsym->ret_type->ctype.name);
            buffer[instr_len+1] = java_type;
            buffer[instr_len+2] = '\0';

            add_instr(list, INVOKESTATIC, buffer, num_params);

            // if top-level and non-void return, will not need result
            if(is_top && strcmp(funsym->ret_type->ctype.name, "void")) {
                sprintf(buffer, "pop");
                add_instr(list, POP, buffer, 0);
            }

            break;
        case _ARR_ACCESS:
            sym = get_localvar(table, expr->text);

            if(sym != NULL) { // local array
                java_type = get_ajavatype(sym->type->ctype.name);

                sprintf(buffer, "aload%c%d ; load from %s", sym->idx > 3 ? ' ' : '_', 
                    sym->idx, expr->text);
                add_instr(list, LOAD, buffer, 0);

                gen_expr(table, lhs, list, false);

                sprintf(buffer, "%caload ; load from %s", java_type,
                    expr->text);
                add_instr(list, LOAD, buffer, 0);

            } else { // global array
                sym = get_globalvar(table, expr->text);
                
                java_type = get_staticjavatype(sym->type->ctype.name);

                sprintf(buffer, "getstatic Field %s %s [%c", classname, 
                    expr->text, java_type);
                add_instr(list, GETSTATIC, buffer, 0);

                gen_expr(table, lhs, list, false);

                java_type = get_ajavatype(sym->type->ctype.name);

                sprintf(buffer, "%caload ; load from %s", java_type,
                    expr->text);
                add_instr(list, LOAD, buffer, 0);
            }

            break;
        case _STRUCT_ACCESS:
            break;
        case _VAR:
            sym = get_localvar(table, expr->text);
            if(sym != NULL) {
                java_type = get_javatype(sym->type->ctype.name);

                sprintf(buffer, "%cload%c%d ; load from %s", 
                    sym->var->ctype.is_array ? 'a' : java_type, 
                    sym->idx > 3 ? ' ' : '_', sym->idx, expr->text);
                add_instr(list, LOAD, buffer, 0);
            } else {
                sym = get_globalvar(table, expr->text);

                if(sym->var->ctype.is_array) {
                    sprintf(buffer, "getstatic Field %s %s [%c", classname, 
                    expr->text, get_staticjavatype(sym->type->ctype.name));
                } else {
                    sprintf(buffer, "getstatic Field %s %s %c", classname, 
                    expr->text, get_staticjavatype(sym->type->ctype.name));
                }
                add_instr(list, GETSTATIC, buffer, 0);
            }
            break;
        case _CHAR_LIT: 
            char c = expr->text[1];

            sprintf(buffer, "bipush %hhu", c);
            add_instr(list, BIPUSH, buffer, 0);

            break;
        case _INT_LIT: 
            int i = atoi(expr->text);

            if(i == -1) {
                sprintf(buffer, "iconst_m1");
                add_instr(list, ICONST, buffer, 0);
            } else if(i >= 0 && i <= 5) {
                sprintf(buffer, "iconst_%d", i);
                add_instr(list, ICONST, buffer, 0);
            } else {
                sprintf(buffer, "ldc %d", i);
                add_instr(list, LDC, buffer, 0);
            }

            break;
        case _REAL_LIT: 
            float f = atof(expr->text);

            if(f == 0.0) {
                sprintf(buffer, "fconst_0");
                add_instr(list, FCONST, buffer, 0);
            } else if(f == 1.0) {
                sprintf(buffer, "fconst_1");
                add_instr(list, FCONST, buffer, 0);
            } else if(f == 2.0) {
                sprintf(buffer, "fconst_2");
                add_instr(list, FCONST, buffer, 0);
            } else {
                sprintf(buffer, "ldc %f", f);
                add_instr(list, LDC, buffer, 0);
            }

            break;
        case _STR_LIT: // TODO
            sprintf(buffer, "ldc '%s'", expr->text+1);
            buffer[strlen(buffer)-2] = '\'';
            buffer[strlen(buffer)-1] = '\0';
            add_instr(list, LDC, buffer, 0);
            sprintf(buffer, "invokevirtual Method java/lang/String toCharArray ()[C");
            add_instr(list, INVOKESTATIC, buffer, 0);
            break;
        default: // fail
            break;
    }
}


// forward declare
void gen_statement(symtable_t *table, astnode_t *statement, instrlist_t *list);


void
gen_if(symtable_t *table, astnode_t *if_statement, instrlist_t *list)
{
    astnode_t *if_cond, *if_body, *else_body, *statement;
    int L1, L2, L3;

    L1 = list->num_labels+1;
    L2 = list->num_labels+2;
    L3 = list->num_labels+3;

    if_cond = if_statement->left;
    if_body = if_cond->right;
    else_body = if_body->right;

    // gen code for expression
    gen_expr(table, if_cond->left, list, false);

    // if leq 0, goto L2
    sprintf(buffer, "ifeq L%d", L2);
    add_instr(list, ICONST, buffer, 0);

    // drop L1
    sprintf(buffer, "L%d:", L1);
    add_instr(list, LABEL, buffer, 0);

    // then block
    statement = if_body->left;
    while(statement != NULL) {
        gen_statement(table, statement, list);
        statement = statement->right;
    }

    if(else_body != NULL) {
        sprintf(buffer, "goto L%d", L3);
        add_instr(list, GOTO, buffer, 0);
    }

    // drop L2
    sprintf(buffer, "L%d:", L2);
    add_instr(list, LABEL, buffer, 0);

    // else block
    if(else_body != NULL) {
        statement = else_body->left;
        while(statement != NULL) {
            gen_statement(table, statement, list);
            statement = statement->right;
        }

        // drop L3
        sprintf(buffer, "L%d:", L3);
        add_instr(list, LABEL, buffer, 0);
    }
}



void
gen_for(symtable_t *table, astnode_t *for_statement, instrlist_t *list)
{
    astnode_t *for_init, *for_cond, *for_update, *for_body, *statement;
    int L1, L2;

    L1 = list->num_labels+1;
    L2 = list->num_labels+2;

    for_init = for_statement->left;
    for_cond = for_init->right;
    for_update = for_cond->right;
    for_body = for_update->right;

    // gen code for init
    gen_expr(table, for_init->left, list, false);

    // drop L1
    sprintf(buffer, "L%d:", L1);
    add_instr(list, LABEL, buffer, 0);

    // gen code for condition
    gen_expr(table, for_cond->left, list, false);

    if(for_cond->left != NULL) {
        sprintf(buffer, "ifeq L%d", L2);
        add_instr(list, ICONST, buffer, 0);
    }    

    // gen loop body
    statement = for_body->left;
    while(statement != NULL) {
        gen_statement(table, statement, list);
        statement = statement->right;
    }

    // gen code for update
    gen_expr(table, for_update->left, list, false);

    sprintf(buffer, "goto L%d", L1);
    add_instr(list, GOTO, buffer, 0);

    // drop L2
    sprintf(buffer, "L%d:", L2);
    add_instr(list, LABEL, buffer, 0);
}


void
gen_while(symtable_t *table, astnode_t *while_statement, instrlist_t *list)
{
    astnode_t *while_cond, *while_body, *statement;
    int L1, L2;

    L1 = list->num_labels+1;
    L2 = list->num_labels+2;

    while_cond = while_statement->left;
    while_body = while_cond->right;

    // drop L1
    sprintf(buffer, "L%d:", L1);
    add_instr(list, LABEL, buffer, 0);

    // gen code for condition
    gen_expr(table, while_cond->left, list, false);

    sprintf(buffer, "ifeq L%d", L2);
    add_instr(list, ICONST, buffer, 0);

    // gen loop body
    statement = while_body->left;
    while(statement != NULL) {
        gen_statement(table, statement, list);
        statement = statement->right;
    }

    sprintf(buffer, "goto L%d", L1);
    add_instr(list, GOTO, buffer, 0);

    // drop L2
    sprintf(buffer, "L%d:", L2);
    add_instr(list, LABEL, buffer, 0);
}



void
gen_do(symtable_t *table, astnode_t *do_statement, instrlist_t *list)
{
    astnode_t *do_cond, *do_body, *statement;
    int L1;

    L1 = list->num_labels+1;

    do_body = do_statement->right;
    do_cond = do_body->left;

    // drop L1
    sprintf(buffer, "L%d:", L1);
    add_instr(list, LABEL, buffer, 0);

    // gen loop body
    statement = do_body->left;
    while(statement != NULL) {
        gen_statement(table, statement, list);
        statement = statement->right;
    }

    // gen code for condition
    gen_expr(table, do_cond->left, list, false);

    sprintf(buffer, "ifeq L%d", L2);
    add_instr(list, ICONST, buffer, 0);
}



void
gen_statement(symtable_t *table, astnode_t *statement, instrlist_t *list)
{
    switch(statement->node_type) {
        case _BREAK:
        case _CONTINUE:
            break;
        case _RETURN:
            sprintf(buffer, ";; return %s %d", filename, statement->line_num);
            add_instr(list, COMMENT, buffer, 0);
            
            if(statement->left != NULL) {
                char java_type;
                java_type = get_javatype(statement->left->ctype.name);

                gen_expr(table, statement->left, list, false);

                sprintf(buffer, "%creturn", java_type);
                add_instr(list, RET, buffer, 0);
            } else {
                sprintf(buffer, "return");
                add_instr(list, RET, buffer, 0);
            }

            list->has_return = true;
            
            break;
        case _IF_STATEMENT:
            gen_if(table, statement, list);
            break;
        case _FOR_STATEMENT:
            gen_for(table, statement, list);
            break;
        case _WHILE_STATEMENT:
            gen_while(table, statement, list);
            break;
        case _DO_STATEMENT:
            gen_do(table, statement, list);
            break;
        default:
            sprintf(buffer, ";; expression %s %d", classname, 
                statement->line_num);
            add_instr(list, COMMENT, buffer, 0);
            gen_expr(table, statement, list, true);
            break; 
    }
}



void
gen_localvar(symtable_t *table, astnode_t *var_decl, instrlist_t *list)
{
    astnode_t *type, *var;
    varsym_t *sym;
    char java_type;

    type = var_decl->left;
    var = type->right;

    java_type = get_javatype(type->ctype.name);

    while(var != NULL) {
        varsym_t *sym;

        add_localvar(table, type, var);
        sym = get_localvar(table, var->text);

        sprintf(buffer, ";; local %d %s %s %d", sym->idx, var->text, filename, var->line_num);
        add_instr(list, COMMENT, buffer, 0);

        if(var->ctype.is_array) { // is array -- needs extra init
            sprintf(buffer, "ldc %d", var->arr_dim);
            add_instr(list, LDC, buffer, 0);

            sprintf(buffer, "newarray %s", var->ctype.name);
            add_instr(list, NEWARRAY, buffer, 0);

            sprintf(buffer, "astore%c%d", (sym->idx > 3) ? ' ' : '_', sym->idx);
            add_instr(list, STORE, buffer, 0);
        }

        if(var->left != NULL) { // includes initialization
            gen_expr(table, var->left, list, true);

            sprintf(buffer, "%cstore %d ; store to %s", java_type, sym->idx,
                var->text);
            add_instr(list, STORE, buffer, 0);
        }

        var = var->right;
    }
}



void
gen_funbody(symtable_t *table, astnode_t *fun_body, instrlist_t *list)
{
    astnode_t *statement;

    statement = fun_body->left;
    while(statement != NULL) {
        if(statement->node_type == _VAR_DECL) {
            gen_localvar(table, statement, list);
        } else if(statement->node_type == _TYPE_DECL) {
            // ignore
        } else {
            gen_statement(table, statement, list);
        }

        if(mode == 5 && statement->node_type == _RETURN) {
            break;
        }

        statement = statement->right;
    }
}



void
gen_fun(symtable_t *table, astnode_t *fun_def)
{
    astnode_t *fun_decl, *fun_body, *ret_type, *ident, *args;
    instrlist_t *list;
    int idx;

    fun_decl = fun_def->left;
    fun_body = fun_decl->right;
    ret_type = fun_decl->left;
    ident = ret_type->right;
    args = ident->right;
    
    list = init_instrlist();

    // print function header
    fprintf(outfile, ".method public static %s : (", ident->text);
    astnode_t *arg_type, *arg_var;
    arg_type = args->left;

    // print argument types, assign first indexes
    while(arg_type != NULL) {
        arg_var = arg_type->right;
        
        if(arg_type->right->ctype.is_array) {
            fprintf(outfile, "[");
        }
        fprintf(outfile, "%c", get_staticjavatype(arg_type->text));        

        add_localvar(table, arg_type, arg_var);

        sprintf(buffer, ";; parameter %d %s %s %d", table->num_locals-1, 
            arg_var->text, filename, arg_var->line_num);
        add_instr(list, COMMENT, buffer, 0);

        arg_type = arg_type->right->right;
    }

    
    // print return type
    if(ret_type->ctype.is_array) {
        fprintf(outfile, "[");
    }
    fprintf(outfile, ")%c\n", get_staticjavatype(ret_type->text));

    table->ret_type = ret_type;

    // construct instruction list
    gen_funbody(table, fun_body, list);

    // check for return
    if(strcmp(ret_type->text, "void")) { // non-void return
        if(!list->has_return) {
            print_msg(GEN_ERR, fun_body->filename, fun_body->line_num, 0, "", "");
            fprintf(stderr, "\tMissing return in non-void function %s\n", 
                fun_decl->text);
            exit(1);
        }
    }

    // print stack and local info
    fprintf(outfile, "\t.code stack %d locals %d\n", list->min_stack_size, 
        table->num_locals);

    // print instruction list
    print_instrlist(list);

    // print function footer
    fprintf(outfile, "\t.end code\n.end method\n\n");

    free_instrlist(list);
    free_locals(table);
}



void
gen_globalvar(symtable_t *table, astnode_t *var_decl)
{
    astnode_t *type, *var;

    type = var_decl->left;
    var = type->right;

    char java_type = get_staticjavatype(type->text);

    while(var != NULL) {
        fprintf(outfile, ".field public static %s ", var->text); 
        if(var->ctype.is_array) {
            fprintf(outfile, "[");
        }
        fprintf(outfile, "%c\n\n", java_type);
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
                sprintf(buffer, "ldc %s", var->left->text);
                add_instr(list, LDC, buffer, 0);

                sprintf(buffer, "newarray %s", type->text);
                add_instr(list, NEWARRAY, buffer, 0);

                sprintf(buffer, "putstatic Field %s %s [%c", classname, 
                    var->text, get_staticjavatype(type->text));
                add_instr(list, PUTSTATIC, buffer, 0);
            } else { // initialized global
                gen_expr(table, var->left, list, true);

                sprintf(buffer, "putstatic Field %s %s %c", classname, 
                    var->text, get_staticjavatype(type->text));
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
            ".end method\n\n");

    if(mode == 5) {
        fprintf(outfile,
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
    } else {
        fprintf(outfile,
            ".method public static main : ([Ljava/lang/String;)V\n"
                "\t.code stack 1 locals 1\n"
                    "\t\tinvokestatic Method %s main ()I\n"
                    "\t\tpop\n"
                    "\t\treturn\n"
                "\t.end code\n"
            ".end method\n", classname);
    }
}



int 
gen_code(char *fname, int m)
{
    symtable_t *table;
    parser_t parser;
    astnode_t *program;

    mode = m;

    // initialize classname stuff
    filename = fname;
    classname = get_classname(fname);

    table = init_symtable();
    init_parser(fname, &parser);
    program = parse_program(&parser);

    typecheck_program(table, program, false);

    gen_header();

    gen_program(table, program);

    gen_footer(table);

    free_symtable(table);
    free(classname);

    return 0;
}
