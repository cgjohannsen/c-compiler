#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "instruction.h"

instr_t *
init_instr(char *text)
{
    instr_t *ret;

    ret = (instr_t *) malloc(sizeof(instr_t));

    strcpy(ret->text, text);
}



instrlist_t *
init_instrlist()
{
    instrlist_t *list;

    list = (instrlist_t *) malloc(sizeof(instrlist_t));

    list->min_stack_size = 0;
    list->stack_size = 0;
    list->num_locals = 0;
    list->num_labels = 0;
    list->len = 0;
    list->head = NULL;
    list->has_return = false;
}



void 
free_instrlist(instrlist_t *list) 
{
    instr_t *cur, *next;
    
    if(list->head == NULL) {
        free(list);
        return;
    }

    cur = list->head;
    next = cur->next;

    while(cur->next != NULL) {
        free(cur->text);
        free(cur);
        cur = next;
        next = cur->next;
    }

    free(cur->text);
    free(cur);
    free(list);
}


void
add_instr(instrlist_t *list, instrtype_t type, char *text, int num_params)
{
    instr_t *new, *cur;
    int prev_stack_size;

    prev_stack_size = list->stack_size;

    // maintain current stack size
    switch(type) {
        case INVOKESTATIC:
            list->stack_size -= num_params; // variable amount
            break;
        case ASTORE:
            list->stack_size -= 3;
            break;
        case IFCMP:
            list->stack_size -= 2;
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case REM:
        case STORE:
        case PUTSTATIC:
        case POP:
        case RET:
        case LCMP:
            list->stack_size -= 1;
            break;
        case LOAD:
        case BIPUSH:
        case SIPUSH:
        case LDC:
        case FCONST:
        case ICONST:
        case ALOAD:
        case GETSTATIC:
        case NEWARRAY:
        case DUP:
        case DUP_X1:
        case DUP_X2:
            list->stack_size += 1;
            break;
        case SWAP:
        case NEG:
        case F2I:
        case I2F:
        case I2C:
        case LABEL:
        default:
            break;
    }

    if(list->stack_size > list->min_stack_size) {
        list->min_stack_size = list->stack_size;
    }

    new = (instr_t *) malloc(sizeof(instr_t));
    new->text = (char *) malloc(sizeof(char) * strlen(text) + 1);

    new->type = type;
    strcpy(new->text, text);

    if(list->head == NULL) {
        list->head = new;
    } else {
        cur = list->head;
        while(cur->next != NULL) {
            cur = cur->next;
        }
        cur->next = new;
    }

    new->next = NULL;
}
