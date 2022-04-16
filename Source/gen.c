#include <string.h>

#include "parse/parse.h"
#include "parse/symtable.h"
#include "parse/typecheck.h"
#include "parse/ast.h"
#include "gen.h"


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


void 
append_str(char *text, int *max_len, char *str)
{
    // check if past max size - realloc if so
    if((*max_len) - strlen(text) < strlen(str)) {
        // ensure we make enough room for the appended string
        strlen(str) > *max_len ? realloc(text, (*max_len)+strlen(str)*2) 
                              : realloc(text, (*max_len)*2);
    }

    strcat(text, str);
}



void
add_progheader(char *text, char *classname)
{
    strcpy(text, ".class public ");
    strcat(text, classname);
    strcat(text, "\n.super java/lang/Object\n\n");
}



void
add_progclinit(char *text, int *max_len, symtable_t *table)
{


    append_str(text, max_len,
        ".method <clinit> : ()V\n"
            "\t.code stack 1 locals 0");

    // initialie globals
    // TODO

    append_str(text, max_len,
                "\t\treturn\n"
            "\t.end code\n"
        ".end method\n\n");

}



void
add_progfooter(char *text, int *max_len, char *classname, symtable_t *table)
{
    append_str(text, max_len, "; Special methods\n\n");

    add_progclinit(text, max_len, table);

    append_str(text, max_len,
        ".method <init> : ()V\n"
            "\t.code stack 1 locals 1\n"
                "\t\taload_0\n"
                "\t\tinvokespecial Method java/lang/Object <init> ()V\n"
                "\t\treturn\n"
            "\t.end code\n"
        ".end method\n\n"
        ".method public static main : ([Ljava/lang/String;)V\n"
            "\t.code stack 2 locals 2\n"
                "\t\tinvokestatic Method ");
    append_str(text, max_len, classname);
    append_str(text, max_len, " main ()I\n"
                "\t\tistore_1\n"
                "\t\tgetstatic Field java/lang/System out Ljava/io/PrintStream;\n"
                "\t\tldc 'Return code: '\n"
                "\t\tinvokevirtual Method java/io/PrintStream print (Ljava/lang/String;)V\n"
                "\t\tgetstatic Field java/lang/System out Ljava/io/PrintStream;\n"
                "\t\tiload_1\n"
                "\t\tinvokevirtual Method java/io/PrintStream println (I)V\n"
                "\t\treturn\n"
            "\t.end code\n"
        ".end method\n");
}





int 
gen_code(char *filename)
{
    symtable_t *table;
    parser_t parser;
    astnode_t *program;
    char *prog_text, *classname;
    int *prog_max_len;

    // initialize program text attributes
    prog_text = (char *) malloc(sizeof(char) * MIN_PROG_LEN);
    prog_max_len = (int *) malloc(sizeof(int));
    *prog_max_len = MIN_PROG_LEN;
    classname = get_basefilename(filename);

    table = init_symtable();
    init_parser(filename, &parser);
    program = parse_program(&parser);

    typecheck_program(table, program, false);

    add_progheader(prog_text, classname);

    // get bulk of program text here

    add_progfooter(prog_text, prog_max_len, classname, table);

    printf("%s\n", prog_text);

    free_symtable(table);
    free(prog_text);
    free(prog_max_len);
    free(classname);

    return 0;
}
