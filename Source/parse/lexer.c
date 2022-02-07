#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../util/hash.h"
#include "lexer.h"

// global variables
char buffer[BUFFER_SIZE];
int cur = 0;
int line_num = 1;


void print_token(FILE *outfile, token_t tok) {
    fprintf(outfile,"File %s Line %*d Token %*d Text %s\n", 
        tok.filename, 5, tok.line_num, 3, tok.type, tok.text);
}

void print_lexer_error(token_t *tok, char *msg) {
    fprintf(stderr, "Lexer error in file %s line %d near text %s\n\t%s",
        tok->filename, tok->line_num, tok->text, msg);
}

void refill_buffer(FILE *infile) {
    int bytes_read = fread(buffer,1,BUFFER_SIZE,infile);
    
    if(ferror(infile))
        fprintf(stderr,"error while reading file\n");
    
    if(feof(infile)) 
        buffer[bytes_read-1] = 0;
    
    cur = 0;
}

token_t init_token(char *filename, int line_num) {
    token_t tok = {
        .text_len = 0,
        .filename = filename,
        .line_num = line_num,
        .is_comment = 0
    };
    return tok;
}

void iterate_cur(FILE *infile) {
    cur++;
    if(cur >= BUFFER_SIZE) 
        refill_buffer(infile);
    printf("char: %c\n",buffer[cur]);
    if(buffer[cur] == '\n') 
        line_num++;
}

// appends buffer[cur] to text of tok then iterates cur
// return 1 if token length too long to append c
// return 0 otherwise
int append_cur(FILE *infile, token_t *tok) { 

    if(tok->text_len >= MAX_LEXEME_SIZE) 
        return 1;

    tok->text[tok->text_len] = buffer[cur]; 
    tok->text_len++;

    iterate_cur(infile);

    return 0;
}

int consume_c_comment(FILE *infile) {
    iterate_cur(infile);

    if(buffer[cur] == '*') {
        iterate_cur(infile);
        if(buffer[cur] == '/') {
            iterate_cur(infile);
            return 0;
        }
    }

    consume_c_comment(infile);
}

int consume_cpp_comment(FILE *infile) {
    iterate_cur(infile);

    if(buffer[cur] == '\n') {
        iterate_cur(infile);
        return 0;
    }

    consume_cpp_comment(infile);
}


int consume_slash(FILE *infile, token_t *tok) {

    append_cur(infile,tok);

    if(buffer[cur] == '*') {
        tok->is_comment = 1;
        consume_c_comment(infile);
        return 0;
    }

    if(buffer[cur] == '/') {
        tok->is_comment = 1;
        consume_cpp_comment(infile);
        return 0;
    }

    if(buffer[cur] == '=') {
        tok->type = SLASHASSIGN;
        append_cur(infile,tok);
        return 0;
    }

    tok->type = SLASH;
    return 0;    
}

int consume_digit(FILE *infile, token_t *tok) {
    // append digit to lexeme
    append_cur(infile,tok);

    if(isdigit(buffer[cur])) {
        // digit followed by digit -- keep consuming
        return consume_digit(infile,tok);
    }

    if(buffer[cur] == '.') {
        // token already seen as real -- stop
        if(tok->type == REAL_LIT)
            return 0;

        // token is a real -- consume '.' and rest of digits
        tok->type = REAL_LIT;
        append_cur(infile,tok);
        return consume_digit(infile,tok);
    }
    
    if(tok->type != REAL_LIT) tok->type = INT_LIT;
    return 0;
}

int consume_alpha(FILE *infile, token_t *tok) {
    tok->type = IDENT;
    append_cur(infile,tok);

    if(isalpha(buffer[cur]) || isdigit(buffer[cur]) || buffer[cur] == '_') {
        consume_alpha(infile,tok);
    }

    return 0;
}

int consume_string(FILE *infile, token_t *tok) {
    append_cur(infile,tok);

    if(buffer[cur] == '\\') {
        append_cur(infile,tok);
        switch(buffer[cur]) {
            case 'a':
            case 'b':
            case 'n':
            case 'r':
            case 't':
            case '\\':
            case '"': append_cur(infile,tok);
            default: break; // TODO -- error
        }
    }

    if(buffer[cur] == '"') {
        tok->type = STR_LIT;
        append_cur(infile,tok);
        return 0;
    }

    consume_string(infile,tok);
}

int consume(FILE *infile, token_t *tok) {

    if(isspace(buffer[cur])) {
        iterate_cur(infile);
        return 1;
    }

    switch(buffer[cur]) {
        case 0: tok->type = END; return 0;
        case ',':
            tok->type = COMMA;
            append_cur(infile,tok);
            return 0;
        case '.':
            tok->type = DOT;
            append_cur(infile,tok);
            return 0;
        case ';':
            tok->type = SEMI;
            append_cur(infile,tok);
            return 0;
        case '(':
            tok->type = LPAR;
            append_cur(infile,tok);
            return 0;
        case ')':
            tok->type = RPAR;
            append_cur(infile,tok);
            return 0;
        case '[':
            tok->type = LBRAK;
            append_cur(infile,tok);
            return 0;
        case ']':
            tok->type = RBRAK;
            append_cur(infile,tok);
            return 0;
        case '{':
            tok->type = LBRACE;
            append_cur(infile,tok);
            return 0;
        case '}':
            tok->type = RBRACE;
            append_cur(infile,tok);
            return 0;
        case '>':
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = GEQ;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = GT;
            return 0;
        case '<':
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = LEQ;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = LT;
            return 0;
        case '=': 
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = EQ;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = ASSIGN;
            return 0;
        case '+': 
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = PLUSASSIGN;
                append_cur(infile,tok);
                return 0;
            }
            if(buffer[cur] == '+') {
                tok->type = INCR;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = PLUS;
            return 0;
        case '-': 
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = MINUSASSIGN;
                append_cur(infile,tok);
                return 0;
            }
            if(buffer[cur] == '-') {
                tok->type = DECR;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = MINUS;
            return 0;
        case '*': 
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = STARASSIGN;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = STAR;
            return 0;
        case '%': 
            tok->type = MOD;
            append_cur(infile,tok);
            return 0;
        case ':': 
            tok->type = COLON;
            append_cur(infile,tok);
            return 0;
        case '?':
            tok->type = QUEST;
            append_cur(infile,tok);
            return 0;
        case '~': 
            tok->type = TILDE;
            append_cur(infile,tok);
            return 0;
        case '|': 
            append_cur(infile,tok);
            if(buffer[cur] == '|') {
                tok->type = DPIPE;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        case '&': 
            append_cur(infile,tok);
            if(buffer[cur] == '&') {
                tok->type = DAMP;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        case '!': 
            append_cur(infile,tok);
            if(buffer[cur] == '=') {
                tok->type = NEQ;
                append_cur(infile,tok);
                return 0;
            }
            tok->type = BANG;
            return 0;
        case '/': return consume_slash(infile,tok);
        case '"': return consume_string(infile,tok);
        case '\'': 
            tok->type = CHAR_LIT;
            append_cur(infile,tok);
            if(buffer[cur] == '\\') {
                append_cur(infile,tok);
                switch(buffer[cur]) {
                    case 'a':
                    case 'b':
                    case 'n':
                    case 'r':
                    case 't':
                    case '\\':
                    case '"': append_cur(infile,tok);
                    default: break; // TODO -- error
                }
            }
            append_cur(infile,tok);
            if(buffer[cur] != '\'') {
                fprintf(stderr,"error, unexpected lexeme\n");
                return 0;
            }
            append_cur(infile,tok);
            return 0;
        default:
            if(isdigit(buffer[cur])) {
                return consume_digit(infile, tok);
            } else if(isalpha(buffer[cur]) || buffer[cur] == '_') {
                return consume_alpha(infile, tok);
            } else {
                append_cur(infile,tok);
                print_lexer_error(tok,"Unexpected symbol, ignoring.");
            }

    }

    

    return 0;

}

// tok must be of type IDENT
void keyword_check(token_t *tok) {

    uint64_t h = hash(tok->value.s);

    switch(h) {
        case CONST_HASH: 
            if(strcmp(tok->value.s,"const") == 0)
                tok->type = CONST;
            break;
        case STRUCT_HASH: 
            if(strcmp(tok->value.s,"struct") == 0)
                tok->type = STRUCT;
            break;
        case FOR_HASH:       
            if(strcmp(tok->value.s,"for") == 0)
                tok->type = FOR;
            break;
        case WHILE_HASH:     
            if(strcmp(tok->value.s,"while") == 0)
                tok->type = WHILE;
            break;
        case DO_HASH:
            if(strcmp(tok->value.s,"do") == 0)
                tok->type = DO;
            break;
        case IF_HASH:
            if(strcmp(tok->value.s,"if") == 0)
                tok->type = IF;
            break;
        case ELSE_HASH:
            if(strcmp(tok->value.s,"else") == 0)
                tok->type = ELSE;
            break;
        case BREAK_HASH:
            if(strcmp(tok->value.s,"break") == 0)
                tok->type = BREAK;
            break;
        case CONTINUE_HASH:
            if(strcmp(tok->value.s,"continue") == 0)
                tok->type = CONTINUE;
            break;
        case RETURN_HASH: 
            if(strcmp(tok->value.s,"return") == 0)
                tok->type = RETURN;
            break;
        case SWITCH_HASH:
            if(strcmp(tok->value.s,"switch") == 0)
                tok->type = SWITCH;
            break;
        case CASE_HASH: 
            if(strcmp(tok->value.s,"case") == 0)
                tok->type = CASE;
            break;
        case DEFAULT_HASH: 
            if(strcmp(tok->value.s,"default") == 0)
                tok->type = DEFAULT;
            break;
        default: break;
    }

}


token_t next_token(FILE *infile, char *infilename) {
    
    token_t tok = init_token(infilename,line_num); // TODO -- implement

    while(consume(infile,&tok));

    if(tok.is_comment) return tok;

    switch(tok.type) {
        case CHAR_LIT:  tok.value.c = *(tok.text); break;
        case INT_LIT:   tok.value.i = atoi(tok.text); break;
        case REAL_LIT:  tok.value.d = atof(tok.text); break;
        case STR_LIT:   tok.value.s = tok.text; break;
        case IDENT:     tok.value.s = tok.text; keyword_check(&tok); break;
        default: break;
    }

    return tok;
}



void tokenize(FILE *infile, FILE *outfile, char *infilename) {
    token_t tok;
    refill_buffer(infile);
    do {
        tok = next_token(infile,infilename);
        if(!tok.is_comment) print_token(outfile,tok);
    } while(tok.type != END || tok.is_comment);
}