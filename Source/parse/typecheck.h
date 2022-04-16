#ifndef TYPECHECK_H
#define TYPECHECK_H

#include <stdbool.h>

#include "symtable.h"
#include "ast.h"

void typecheck_program(symtable_t *table, astnode_t *program, bool output);
int typecheck(char *filename);

#endif