#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../util/hash.h"
#include "../util/io.h"
#include "lexer.h"

/* GLOBALS */
char *infilename;
int line_num;

/* HELPER FUNCTIONS */

/*
 *
 */
void 
print_token(FILE *outfile, token_t tok) 
{
    fprintf(outfile,"File %s Line %*d Token %*d Text %s\n", 
        tok.filename, 5, tok.line_num, 3, tok.type, tok.text);
}

token_t 
init_token() 
{
    token_t tok = {
        .text_len = 0,
        .filename = infilename,
        .line_num = line_num
    };
    return tok;
}

/*
 * 
 */
int 
append_char(char *cur, token_t *tok) 
{
    if(tok->text_len >= MAX_LEXEME_SIZE) {
        // TODO -- check for max string len
        return 1;
    }

    tok->text[tok->text_len] = *cur; 
    tok->text_len++;

    cur++;

    return 0;
}

// forward declaration
int consume(char *, token_t *);

int 
consume_c_comment(char *cur, token_t *tok) 
{
    cur++;

    if(*cur == 0) { // check if *cur is EOF
        print_msg(LEXER_ERR, infilename, line_num, ' ', "Unclosed comment.");
        return 0;
    }

    if(*cur == '\n') { // keep track of line numbers
        line_num++;
    }

    if(*cur == '*') {
        cur++;

        if(*cur == 0) { // check if *cur is EOF
            print_msg(LEXER_ERR, infilename, line_num, ' ', "Unclosed comment.");
            return 0;
        }

        if(*cur == '/') { // end of comment -- reenter consume
            cur++;
            return consume(cur, tok);
        }
    }

    consume_c_comment(cur, tok);
}

/*
 *
 */
int 
consume_cpp_comment(char *cur, token_t *tok) 
{
    cur++;

    if(*cur == '\n') {
        line_num++;
        cur++;
        return consume(cur, tok);
    }

    consume_cpp_comment(cur, tok);
}

/*
 *
 */
int 
consume_slash(char *cur, token_t *tok) 
{
    append_char(cur, tok);

    if(*cur == '*') {
        return consume_c_comment(cur, tok);
    }

    if(*cur == '/') {
        return consume_cpp_comment(cur, tok);
    }

    if(*cur == '=') {
        tok->type = SLASHASSIGN;
        append_char(cur, tok);
        return 0;
    }

    tok->type = SLASH;
    return 0;    
}

/*
 *
 */
int 
consume_digit(char *cur, token_t *tok) 
{
    if(tok->text_len == MAX_LEXEME_SIZE) {
        print_msg(LEXER_WRN, infilename, line_num, *cur, "Max lexeme length reached, truncating."); // TODO
    }

    append_char(cur, tok);

    if(isdigit(*cur)) {
        // digit followed by digit -- keep consuming
        return consume_digit(cur, tok);
    }

    if(*cur == '.') {
        // token already seen as real -- error
        if(tok->type == REAL_LIT) {
            print_msg(LEXER_ERR, infilename, line_num, *cur, "Too many '.'");
            return 0;
        }

        // token is a real -- consume '.' and rest of digits
        tok->type = REAL_LIT;
        append_char(cur, tok);
        return consume_digit(cur, tok);
    }
    
    if(tok->type != REAL_LIT) tok->type = INT_LIT;
    return 0;
}

int 
consume_alpha(char *cur, token_t *tok) 
{
    if(tok->text_len == MAX_LEXEME_SIZE) {
        print_msg(LEXER_WRN, infilename, line_num, *cur, "Max lexeme length reached, truncating.");
    }

    append_char(cur, tok);

    // if *cur is a letter, digit, or _: is valid identifier char
    if(isalpha(*cur) || isdigit(*cur) || *cur == '_') {
        consume_alpha(cur, tok);
    }

    tok->type = IDENT;
    return 0;
}

int 
consume_string(char *cur, token_t *tok) 
{
    if(tok->text_len == MAX_STR_SIZE) {
        print_msg(LEXER_WRN, infilename, line_num, *cur, "Max string length reached, truncating.");
    }

    if(tok->text_len < MAX_STR_SIZE) { // only append to tok if less than max string length
        append_char(cur, tok);
        
        if(*cur == '\\') {  // check if cur is escape char
            append_char(cur, tok); 
        
            switch(*cur) {
                case 'a':
                case 'b':
                case 'n':
                case 'r':
                case 't':
                case '\\':
                case '"': append_char(cur, tok);
                default:
                {
                    print_msg(LEXER_ERR, infilename, line_num, *cur, "Unexpected escape symbol");
                    return 0;
                }
            }
        }
    } else { // else if tok is past max string length, iterate cur
        cur++;
    }

    if(*cur == '"') {
        tok->type = STR_LIT;
        append_char(cur, tok);
        return 0;
    }

    consume_string(cur, tok);
}

int 
consume(char *cur, token_t *tok) 
{
    // keep track of line numbers
    if(*cur == '\n') {
        line_num++;
    }

    // if *cur is whitespace, skip char
    if(isspace(*cur)) {
        cur++;
        return 1;
    }

    switch(*cur) {
        case 0:
        {
            tok->type = END; 
            return 0;
        }    
        case ',':
        {
            tok->type = COMMA;
            append_char(cur, tok);
            return 0;
        }
        case '.':
        {
            tok->type = DOT;
            append_char(cur, tok);
            return 0;
        }
        case ';':
        {
            tok->type = SEMI;
            append_char(cur, tok);
            return 0;
        }   
        case '(':
        {
            tok->type = LPAR;
            append_char(cur, tok);
            return 0;
        }
        case ')':
        {
            tok->type = RPAR;
            append_char(cur, tok);
            return 0;
        }
        case '[':
        {    
            tok->type = LBRAK;
            append_char(cur, tok);
            return 0;
        }
        case ']':
        {    
            tok->type = RBRAK;
            append_char(cur, tok);
            return 0;
        }
        case '{':
        {    
            tok->type = LBRACE;
            append_char(cur, tok);
            return 0;
        }
        case '}':
        {
            tok->type = RBRACE;
            append_char(cur, tok);
            return 0;
        }
        case '>':
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = GEQ;
                append_char(cur, tok);
                return 0;
            }
            tok->type = GT;
            return 0;
        }
        case '<':
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = LEQ;
                append_char(cur, tok);
                return 0;
            }
            tok->type = LT;
            return 0;
        }
        case '=': 
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = EQ;
                append_char(cur, tok);
                return 0;
            }
            tok->type = ASSIGN;
            return 0;
        }
        case '+': 
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = PLUSASSIGN;
                append_char(cur, tok);
                return 0;
            }
            if(*cur == '+') {
                tok->type = INCR;
                append_char(cur, tok);
                return 0;
            }
            tok->type = PLUS;
            return 0;
        }
        case '-': 
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = MINUSASSIGN;
                append_char(cur, tok);
                return 0;
            }
            if(*cur == '-') {
                tok->type = DECR;
                append_char(cur, tok);
                return 0;
            }
            tok->type = MINUS;
            return 0;
        }
        case '*': 
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = STARASSIGN;
                append_char(cur, tok);
                return 0;
            }
            tok->type = STAR;
            return 0;
        }
        case '%': 
        {
            tok->type = MOD;
            append_char(cur, tok);
            return 0;
        }
        case ':':
        { 
            tok->type = COLON;
            append_char(cur, tok);
            return 0;
        }
        case '?':
        {
            tok->type = QUEST;
            append_char(cur, tok);
            return 0;
        }
        case '~': 
        {
            tok->type = TILDE;
            append_char(cur, tok);
            return 0;
        }
        case '|': 
        {
            append_char(cur, tok);
            if(*cur == '|') {
                tok->type = DPIPE;
                append_char(cur, tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        }
        case '&': 
        {
            append_char(cur, tok);
            if(*cur == '&') {
                tok->type = DAMP;
                append_char(cur, tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        }
        case '!': 
        {
            append_char(cur, tok);
            if(*cur == '=') {
                tok->type = NEQ;
                append_char(cur, tok);
                return 0;
            }
            tok->type = BANG;
            return 0;
        }
        case '/': return consume_slash(cur, tok);
        case '"': return consume_string(cur, tok);
        case '\'': 
        {
            tok->type = CHAR_LIT;
            append_char(cur, tok);
            if(*cur == '\\') {
                append_char(cur, tok);
                switch(*cur) {
                    case 'a':
                    case 'b':
                    case 'n':
                    case 'r':
                    case 't':
                    case '\\':
                    case '"': append_char(cur, tok);
                    default: 
                    {
                        print_msg(LEXER_ERR, infilename, line_num, *cur, "Unexpected escape symbol");
                    }
                }
            }
            append_char(cur, tok);
            if(*cur != '\'') {
                fprintf(stderr,"error, unexpected lexeme\n");
                return 0;
            }
            append_char(cur, tok);
            return 0;
        }
        default:
        {
            if(isdigit(*cur)) {
                return consume_digit(cur, tok);
            } else if(isalpha(*cur) || *cur == '_') {
                return consume_alpha(cur, tok);
            } else {
                append_char(cur, tok);
                print_msg(LEXER_ERR, infilename, line_num, *cur, "Unexpected symbol, ignoring.");
            }
        }
    }

    return 0;
}

/*
 *
 */
void 
keyword_check(token_t *tok) 
{
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

/*
 *
 */
token_t 
next_token(char *cur) 
{
    token_t tok = init_token();

    while(consume(cur, &tok));

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

/*
 *  Must call free(buffer) after calling
 */
char *
init_tokenizer(char *filename)
{
    char *buffer;

    // initialize globals
    infilename = filename;
    line_num = 1;

    buffer = read_file(filename);
    return buffer;
}


/*
 *
 */
void 
tokenize(char *filename, FILE *outfile) 
{
    char *buffer, *cur;
    token_t tok;

    // remember to call free(buffer) !!!
    buffer = init_tokenizer(filename);
    cur = buffer; // set cur to beginning of buffer

    tok = next_token(cur);

    while(tok.type != END) {
        tok = next_token(cur);
        print_token(outfile,tok);
    }

    free(buffer);
}


