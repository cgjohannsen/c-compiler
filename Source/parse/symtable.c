#include <stdlib.h>

#include "symtable.h"


void 
init_table(symtable_t *symtable)
{
    symtable_t->head = NULL;
}



void 
add_entry(symtable_t *symtable, symentry_t *entry)
{
    symentry_t *cur = symtable->head;

    if(cur == NULL) { // table is empty
        symtable->head = entry;
    }

    // find end of linked list
    while(cur->next != NULL) {
        cur = cur->next;
    }

    cur->next = entry;
}



void
remove_entry(symtable_t *symtable, char *ident)
{
    symentry_t *cur = symtable->head;

    if(cur == NULL) { // table is empty
        return;
    }

    // find end of linked list
    while(cur->next != NULL) {
        // test against current entry
        if(!strcomp(ident,cur->next->ident)) {
            break;
        }
        cur = cur->next;
    }

    if(cur->next == NULL) { // no such entry in table
        return;
    }

    // remove matched entry
    cur->next = cur->next->next;
}




symentry_t *
get_entry(symtable_t *symtable, char *ident)
{
    symentry_t *cur = symtable->head;

    if(cur == NULL) { // table is empty
        return cur;
    }

    // find end of linked list
    while(cur->next != NULL) {
        // test against current entry
        if(!strcomp(ident,cur->next->ident)) {
            break;
        }
        cur = cur->next;
    }

    if(cur->next == NULL) { // no such entry in table
        return;
    }

    // remove matched entry
    cur->next = cur->next->next;
}