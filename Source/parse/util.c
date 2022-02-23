
#include "util.h"

void 
push(stack_t *stack, token_t *tok)
{
    stacknode_t node = {
        .val = tok,
        .next = NULL,
        .prev = stack->top
    };

    if(stack->top != NULL) {
        stack->top->next->val = tok;
    }

    stack->top = &node;
    stack->size++;
}


token_t *
pop(stack_t *stack)
{
    token_t *ret = stack->top->val;

    stack->top->prev->next = NULL;
    stack->top = stack->top->prev;
    stack->size--;

    return ret;
}


stack_t 
init_stack(void)
{
    stack_t stack = {
        .top = NULL,
        .size = 0
    };
    return stack;
}