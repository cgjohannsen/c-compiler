#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../util/hash.h"
#include "../util/io.h"
#include "lexer.h"

/*
 *
 */
void 
print_token(FILE *outfile, token_t *tok) 
{
    fprintf(outfile,"File %s Line %*d Token %*d Text %s\n", 
        tok->filename, 5, tok->line_num, 3, tok->type, tok->text);
}

token_t 
init_token(char *filename, int line_num) 
{
    token_t tok = {
        .text_size = 0,
        .filename = filename,
        .line_num = line_num
    };
    return tok;
}

/*
 * 
 */
int 
append_char(lexer_t *lex, token_t *tok) 
{
    tok->text[tok->text_size] = *(lex->cur); 
    tok->text_size++;

    (lex->cur)++;

    return 0;
}

/*
 *
 */
int 
consume_c_comment(lexer_t *lex, token_t *tok) 
{
    (lex->cur)++;

    if(*(lex->cur) == 0) { // check if *cur is EOF
        print_msg(LEXER_ERR, lex->filename, lex->line_num, ' ', "Unclosed comment.");
        return 0;
    }

    if(*(lex->cur) == '\n') { // keep track of line numbers
        (lex->line_num)++;
    }

    if(*(lex->cur) == '*') {
        (lex->cur)++;

        if(*(lex->cur) == 0) { // check if *cur is EOF
            print_msg(LEXER_ERR, lex->filename, lex->line_num, ' ', "Unclosed comment.");
            return 0;
        }

        if(*(lex->cur) == '/') { // end of comment -- reenter consume
            (lex->cur)++;
            tok->line_num = lex->line_num; // update token line num
            return consume(lex, tok);
        }
    }

    consume_c_comment(lex, tok);
}

/*
 *
 */
int 
consume_cpp_comment(lexer_t *lex, token_t *tok) 
{
    (lex->cur)++;

    if(*(lex->cur) == '\n') {
        (lex->cur)++;
        (lex->line_num)++;
        tok->line_num = lex->line_num; // update token line num
        return consume(lex, tok);
    }

    consume_cpp_comment(lex, tok);
}

/*
 *
 */
int 
consume_slash(lexer_t *lex, token_t *tok) 
{
    if(*(lex->cur+1) == '*') {
        (lex->cur)++;
        return consume_c_comment(lex, tok);
    }

    if(*(lex->cur+1) == '/') {
        (lex->cur)++;
        return consume_cpp_comment(lex, tok);
    }

    if(*(lex->cur+1) == '=') {
        tok->type = SLASHASSIGN;
        append_char(lex, tok);
        append_char(lex, tok);
        return 0;
    }

    append_char(lex, tok);
    tok->type = SLASH;
    return 0;    
}

/*
 *
 */
int 
consume_digit(lexer_t *lex, token_t *tok) 
{
    if(tok->text_size == MAX_LEXEME_SIZE) {
        print_msg(LEXER_WRN, lex->filename, lex->line_num, *(lex->cur), "Max lexeme length reached, truncating.");
    }

    if(tok->text_size < MAX_LEXEME_SIZE) {
        append_char(lex, tok); // append to text only if less than max lexeme size
    }

    if(isdigit(*(lex->cur))) {
        // digit followed by digit -- keep consuming
        return consume_digit(lex, tok);
    }

    if(*(lex->cur) == '.') {
        // token already seen as real -- error
        if(tok->type == REAL_LIT) {
            print_msg(LEXER_ERR, lex->filename, lex->line_num, *(lex->cur), "Too many '.'");
            return 0;
        }

        // token is a real -- consume '.' and rest of digits
        tok->type = REAL_LIT;
        append_char(lex, tok);
        return consume_digit(lex, tok);
    }
    
    if(tok->type != REAL_LIT) tok->type = INT_LIT;
    return 0;
}

/*
 *
 */
int 
consume_alpha(lexer_t *lex, token_t *tok) 
{
    if(tok->text_size == MAX_LEXEME_SIZE) {
        print_msg(LEXER_WRN, lex->filename, lex->line_num, *(lex->cur), "Max lexeme length reached, truncating.");
    }

    if(tok->text_size < MAX_LEXEME_SIZE) {
        append_char(lex, tok); // append to text only if less than max lexeme size
    }

    // if *cur is a letter, digit, or _: is valid identifier char
    if(isalpha(*(lex->cur)) || isdigit(*(lex->cur)) || *(lex->cur) == '_') {
        consume_alpha(lex, tok);
    }

    tok->type = IDENT;
    return 0;
}

/*
 *
 */
int 
consume_string(lexer_t *lex, token_t *tok) 
{
    if(tok->text_size == MAX_LEXEME_SIZE) {
        print_msg(LEXER_WRN, lex->filename, lex->line_num, *(lex->cur), "Max string length reached, truncating.");
    }

    if(tok->text_size < MAX_LEXEME_SIZE) { // only append to tok if less than max string length
        append_char(lex, tok);
        
        if(*(lex->cur) == '\\') {  // check if cur is escape char
            append_char(lex, tok); 
        
            switch(*(lex->cur)) {
                case 'a':
                case 'b':
                case 'n':
                case 'r':
                case 't':
                case '\\':
                case '"': append_char(lex, tok); break;
                default:
                {
                    print_msg(LEXER_ERR, lex->filename, lex->line_num, *(lex->cur), "Unexpected escape symbol");
                    return 0;
                }
            }
        }
    } else { // else if tok is past max string length, iterate cur
        (lex->cur)++;
    }

    if(*(lex->cur) == '"') {
        tok->type = STR_LIT;
        append_char(lex, tok);
        return 0;
    }

    consume_string(lex, tok);
}

/*
 *
 */
int 
consume(lexer_t *lex, token_t *tok) 
{
    if(isspace(*(lex->cur))) { // if current char is whitespace, skip
        if(*(lex->cur) == '\n') { // keep track of line numbers
            (lex->line_num)++;
            tok->line_num = lex->line_num; // update token line num
        }
        (lex->cur)++;
        return consume(lex, tok);
    }

    switch(*(lex->cur)) {
        case 0:
        {
            tok->type = END; 
            return 0;
        }    
        case ',':
        {
            tok->type = COMMA;
            append_char(lex, tok);
            return 0;
        }
        case '.':
        {
            tok->type = DOT;
            append_char(lex, tok);
            return 0;
        }
        case ';':
        {
            tok->type = SEMI;
            append_char(lex, tok);
            return 0;
        }   
        case '(':
        {
            tok->type = LPAR;
            append_char(lex, tok);
            return 0;
        }
        case ')':
        {
            tok->type = RPAR;
            append_char(lex, tok);
            return 0;
        }
        case '[':
        {    
            tok->type = LBRAK;
            append_char(lex, tok);
            return 0;
        }
        case ']':
        {    
            tok->type = RBRAK;
            append_char(lex, tok);
            return 0;
        }
        case '{':
        {    
            tok->type = LBRACE;
            append_char(lex, tok);
            return 0;
        }
        case '}':
        {
            tok->type = RBRACE;
            append_char(lex, tok);
            return 0;
        }
        case '>':
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = GEQ;
                append_char(lex, tok);
                return 0;
            }
            tok->type = GT;
            return 0;
        }
        case '<':
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = LEQ;
                append_char(lex, tok);
                return 0;
            }
            tok->type = LT;
            return 0;
        }
        case '=': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = EQ;
                append_char(lex, tok);
                return 0;
            }
            tok->type = ASSIGN;
            return 0;
        }
        case '+': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = PLUSASSIGN;
                append_char(lex, tok);
                return 0;
            }
            if(*(lex->cur) == '+') {
                tok->type = INCR;
                append_char(lex, tok);
                return 0;
            }
            tok->type = PLUS;
            return 0;
        }
        case '-': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = MINUSASSIGN;
                append_char(lex, tok);
                return 0;
            }
            if(*(lex->cur) == '-') {
                tok->type = DECR;
                append_char(lex, tok);
                return 0;
            }
            tok->type = MINUS;
            return 0;
        }
        case '*': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = STARASSIGN;
                append_char(lex, tok);
                return 0;
            }
            tok->type = STAR;
            return 0;
        }
        case '%': 
        {
            tok->type = MOD;
            append_char(lex, tok);
            return 0;
        }
        case ':':
        { 
            tok->type = COLON;
            append_char(lex, tok);
            return 0;
        }
        case '?':
        {
            tok->type = QUEST;
            append_char(lex, tok);
            return 0;
        }
        case '~': 
        {
            tok->type = TILDE;
            append_char(lex, tok);
            return 0;
        }
        case '|': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '|') {
                tok->type = DPIPE;
                append_char(lex, tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        }
        case '&': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '&') {
                tok->type = DAMP;
                append_char(lex, tok);
                return 0;
            }
            tok->type = PIPE;
            return 0;
        }
        case '!': 
        {
            append_char(lex, tok);
            if(*(lex->cur) == '=') {
                tok->type = NEQ;
                append_char(lex, tok);
                return 0;
            }
            tok->type = BANG;
            return 0;
        }
        case '/': return consume_slash(lex, tok);
        case '"': return consume_string(lex, tok);
        case '\'': 
        {
            tok->type = CHAR_LIT;
            if(*(lex->cur+1) == '\\') {
                switch(*(lex->cur+2)) {
                    case 'a':
                    case 'b':
                    case 'n':
                    case 'r':
                    case 't':
                    case '\\':
                    {
                        // consume entire lexeme here (of form '\a')
                        append_char(lex, tok);
                        append_char(lex, tok);
                        append_char(lex, tok);
                        append_char(lex, tok);
                        return 0;
                    }
                    default: 
                    {
                        print_msg(LEXER_ERR, lex->filename, lex->line_num, *(lex->cur+2), "Unexpected escape symbol, ignoring.");
                        lex->cur = lex->cur + 4; // skip garbage characters
                        return consume(lex, tok);
                    }
                }
            }
            
            if(*(lex->cur+2) != '\'') {
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *(lex->cur), "Unexpected symbol, ignoring.");
                lex->cur = lex->cur + 3; // skip garbage characters
                return consume(lex, tok);
            }

            // consume entire lexeme (of form 'c')
            append_char(lex, tok);
            append_char(lex, tok);
            append_char(lex, tok);

            return 0;
        }
        default:
        {
            if(isdigit(*(lex->cur))) {
                return consume_digit(lex, tok);
            } else if(isalpha(*(lex->cur)) || *(lex->cur) == '_') {
                return consume_alpha(lex, tok);
            } else {
                append_char(lex, tok);
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *(lex->cur), "Unexpected symbol, ignoring.");
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
next_token(lexer_t *lex) 
{
    token_t tok = init_token(lex->filename, lex->line_num);

    consume(lex, &tok);

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
 *
 */
void
free_lexer(lexer_t *lex)
{
    free(lex->buffer);
}


/*
 *  Must call free_lexer after calling
 */
lexer_t
init_lexer(char *filename)
{
    char *buf = read_file(filename);

    lexer_t lex = {
        .filename = filename,
        .buffer = buf,
        .cur = buf, // initialize cur to start of buffer
        .line_num = 1
    };

    return lex;
}


/*
 *
 */
void 
tokenize(char *filename, FILE *outfile) 
{
    lexer_t lex;
    token_t tok;

    // remember to call free(buffer) !!!
    lex = init_lexer(filename);

    tok = next_token(&lex);
    while(tok.type != END) { // get tokens until EOF
        print_token(outfile, &tok);
        tok = next_token(&lex);
    }

    free_lexer(&lex);
}


