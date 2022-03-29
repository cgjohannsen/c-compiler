
#include "parse.h"
#include "typecheck.h"

int 
typecheck(char *infilename, FILE *outfile)
{
    parser_t parser;
    astnode_t *program;

    init_parser(infilename, &parser);
    program = parse_program(&parser);

    typecheck_program(program);

    return 0;
}