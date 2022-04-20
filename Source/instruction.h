#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#define MAX_INSTR_LEN 512

typedef enum instrtype {
    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    NEG,
    LOAD,
    STORE,
    BIPUSH,
    SIPUSH,
    LDC,
    FCONST,
    LCONST,
    ALOAD,
    ASTORE,
    GETSTATIC,
    PUTSTATIC,
    INVOKESTATIC,
    NEWARRAY,
    RET,
    SWAP,
    POP,
    DUP,
    DUP_X1,
    DUP_X2,
    F2I,
    I2F,
    I2C,
    OTHER
} instrtype_t;

typedef struct instr {
    instrtype_t type;
    char *text;
    struct instr *next;
} instr_t;

typedef struct instrlist {
    int min_stack_size;
    int stack_size;
    int num_locals;
    int len;
    instr_t *head;
} instrlist_t;

instr_t *init_instr(char *text);
instrlist_t *init_instrlist(void);
void free_instrlist(instrlist_t *list);

void add_instr(instrlist_t *list, instrtype_t type, char *text, int num_params);

#endif