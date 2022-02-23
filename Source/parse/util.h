#ifndef UTIL_H
#define UTIL_H

#include "token.h"

typedef struct stacknode {
    token_t *val;
    struct stacknode *next;
    struct stacknode *prev;
} stacknode_t;

typedef struct stack {
    stacknode_t *top;
    int size;
} stack_t;

void push(stack_t *, token_t *);
token_t *pop(stack_t *);
stack_t init_stack(void);



#endif