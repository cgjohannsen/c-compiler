#include <stdlib.h>

#include "symtable.h"

void 
init_table(symtable_t *table)
{
    table = (symtable_t *) malloc(sizeof(symentry_t *) * MIN_SYMTABLE_SIZE);
    table->max_size = MIN_SYMTABLE_SIZE;
    table->size = 0;
}

void 
add_entry(symtable_t *table, symentry_t *entry)
{
    if(table->size == table->max_size) {
        // realloc
    }

    table[table->size] = entry;
    table->size++;
}