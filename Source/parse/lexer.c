#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "../util/hash.h"
#include "../util/io.h"
#include "lexer.h"

/**
 * Determines if input is a valid octal character i.e. [0-7]
 * Sister function to those in ctype.h (i.e. isdigit(), isalpha(), etc.)
 *
 * @param c character to check
 *
 * @return  1 if c is in [0-7]
 *          0 otherwise
 */
int
isoctal(int c)
{
    int digit = c - '0';
    return (digit >= 0 && digit <= 7);
}

/**
 * Checks if token is of valid size i.e. under the max of the token's type. If 
 * it is already equal to max size, warns user that lexeme is being truncated.
 *
 * @param lex lexer struct with useful info to print
 * @param tok token to be considered
 *
 * @return  0 if token is above max size
 *          1 otherwise
 */
int
is_valid_size(lexer_t *lex, token_t *tok)
{
    if(tok->type == STR_LIT) {
        if(tok->text_size == MAX_STR_SIZE) { // only print once token
            print_msg(LEXER_WRN, lex->filename, lex->line_num, *lex->cur, "",
                "Max string length reached, truncating.");
        }
        if(tok->text_size >= MAX_STR_SIZE) {
            return 0;
        }
    } else {
        if(tok->text_size == MAX_LEXEME_SIZE) { // only print once token
            print_msg(LEXER_WRN, lex->filename, lex->line_num, *lex->cur, "", 
                "Max lexeme length reached, truncating.");
        }
        if(tok->text_size >= MAX_LEXEME_SIZE) {
            return 0;
        }
    }

    return 1;
}

/**
 * Iterates the current character pointer of the lexer. Most importantly, checks
 * if the buffer needs to be refilled and if so, refills it.
 *
 * @param lex lexer under use
 *
 * @return 1
 */
int
iterate_cur(lexer_t *lex)
{
    if(lex->cur == lex->buffer + BUFFER_SIZE - 1) { // end of buffer
        refill_buffer(lex->infile, lex->buffer);
        lex->cur = lex->buffer; // set cur back to beginning of buffer
    } else { // otherwise, just iterate
        (lex->cur)++; 
    }

    return 1;
}


/**
 * Appends the current char to the input token if the token has available size. 
 * Also resizes the size of the text for the token if necessary
 *
 * @param lex relevant lexer
 * @param tok token to append the current char to
 *
 * @return  1 on success
 *          0 otherwise
 */
int 
append_char(lexer_t *lex, token_t *tok) 
{
    if(!is_valid_size(lex, tok)) { // tok above max size -- just iterate cur
        tok->text_size++;
        iterate_cur(lex);
        return 0;
    }
    
    if(tok->text_size == tok->text_max_size) { // need to add more memory
        tok->text = realloc(tok->text, (tok->text_max_size << 1) + 1);
        tok->text_max_size = tok->text_max_size << 1;
    }

    tok->text[tok->text_size] = *lex->cur; 
    tok->text[tok->text_size + 1] = '\0'; // update end of str 
    tok->text_size++;

    iterate_cur(lex);

    return 1;
}

/**
 * Consumes a c-style comment (i.e. same style as this commnent). Tracks
 * newlines and does not append any characters to the current token. Returns
 * an error if EOF is found before comment close. Once end of the comment is 
 * found, reenters consume().
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume_c_comment(lexer_t *lex, token_t *tok) 
{
    while(1) {
        iterate_cur(lex);

        if(*lex->cur == 0) { // check if *cur is EOF
            print_msg(LEXER_ERR, lex->filename, tok->line_num, ' ', "", 
                "Unclosed comment.");
            return 0;
        }

        if(*lex->cur == '\n') { // keep track of line numbers
            (lex->line_num)++;
        }

        if(*lex->cur == '*') {
            iterate_cur(lex);

            if(*lex->cur == 0) { // check if *cur is EOF
                print_msg(LEXER_ERR, lex->filename, tok->line_num, ' ', 
                    "", "Unclosed comment.");
                return 0;
            }

            if(*lex->cur == '/') { // end of comment -- reenter consume
                iterate_cur(lex);
                tok->line_num = lex->line_num; // update token line num
                return consume(lex, tok);
            }
        }
    }

    return 0;
}

/**
 * Consumes cpp-style comment (i.e. //). Does not append any chars to current 
 * token. Once \n is found, reenters consume().
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume_cpp_comment(lexer_t *lex, token_t *tok) 
{
    iterate_cur(lex);

    if(*lex->cur == '\n') {
        iterate_cur(lex);
        (lex->line_num)++;
        tok->line_num = lex->line_num; // update token line num
        return consume(lex, tok);
    }

    return consume_cpp_comment(lex, tok);
}

/**
 * Consumes a '/' and determines whether tok is a c-style comment, cpp-style 
 * comment, SLASHASSIGN, or SLASH.
 *
 * @param lex relevant lexer
 * @param tok token to be generated 
 *
 * @return 0
 */
int 
consume_slash(lexer_t *lex, token_t *tok) 
{
    if(*(lex->cur+1) == '*') {
        iterate_cur(lex);
        return consume_c_comment(lex, tok);
    }

    if(*(lex->cur+1) == '/') {
        iterate_cur(lex);
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

/**
 * Consumes the exponent portion of a REAL_LIT (i.e. following 'e'). All chars 
 * should be digits. 
 *
 * @param lex relevant lexer
 * @param tok token to be generated 
 *
 * @return 0
 */
int
consume_real_exp(lexer_t *lex, token_t *tok)
{
    append_char(lex, tok);
 
    if(isdigit(*lex->cur)) {
        return consume_real_exp(lex, tok);
    }

    if(isalnum(*lex->cur) || *lex->cur == '_') { // check if invalid char
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "", 
            "Invalid character as part of real literal.");
    }

    return 0;
}

/**
 * Consumes the fractional portion of a REAL_LIT (i.e. following '.'). 
 * Characters following this can be exp-type chars or digits.
 *
 * @param lex relevant lexer
 * @param tok token to be generated 
 *
 * @return 0
 */
int
consume_real_frac(lexer_t *lex, token_t *tok)
{
    append_char(lex, tok);

    if(isdigit(*lex->cur)) {
        return consume_real_frac(lex, tok);
    }

    if(*lex->cur == 'e' || *lex->cur == 'E') { // token is real w exp part
        if(*(lex->cur+1) == '-' || *(lex->cur+1) == '+') {
            append_char(lex, tok); // append optional '-', '+'
        }
        return consume_real_exp(lex, tok);
    }

    if(isalnum(*lex->cur) || *lex->cur == '_') { // check if invalid char
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "", 
            "Invalid character as part of real literal.");
    }

    return 0;
}

/**
 * Consumes a digit in the context of an INT_LIT. If a '.' or 'e' is found,
 * starts consuming as a REAL_LIT. 
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume_int(lexer_t *lex, token_t *tok) 
{
    int status = 1;

    status = append_char(lex, tok);

    if(isdigit(*lex->cur)) { // digit followed by digit -- keep consuming
        return consume_int(lex, tok);
    }

    if(*lex->cur == '.') { // token is REAL_LIT w frac part
        if(status) { // check if truncated before '.'
            tok->type = REAL_LIT;
        }
        return consume_real_frac(lex, tok);
    }
    
    if(*lex->cur == 'e' || *lex->cur == 'E') { // token is REAL_LIT w exp
        if(status) { // check if truncated before any of 'e', 'E'
            tok->type = REAL_LIT;
        }
        if(*(lex->cur+1) == '-' || *(lex->cur+1) == '+') {
            append_char(lex, tok);
        }
        return consume_real_exp(lex, tok);
    }

    if(isalnum(*lex->cur) || *lex->cur == '_') { // check if invalid char
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "", 
            "Invalid character as part of integer literal.");
    }
    
    tok->type = INT_LIT;
    return 0;
}

/**
 * Consumes a hexidecimal literal. If a non-hex alphanumeric character is found,
 * prints an error. 
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int
consume_hex(lexer_t *lex, token_t *tok)
{
    append_char(lex, tok);

    if(isxdigit(*lex->cur)) {
        return consume_hex(lex, tok);
    }

    if(isalnum(*lex->cur) || *lex->cur == '_') { // check if invalid char
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "", 
            "Invalid character as part of hexadecimal literal.");
    }

    return 0;
}

/**
 * Consumes an octal literal. If a non-octal alphanumeric character is found,
 * prints an error. 
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int
consume_octal(lexer_t *lex, token_t *tok)
{
    append_char(lex, tok);

    if(isoctal(*lex->cur)) {
        return consume_octal(lex, tok);
    }

    if(isalnum(*lex->cur) || *lex->cur == '_') { // check if invalid char
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "", 
            "Invalid character as part of octal literal.");
    }

    return 0;
}

/**
 * Consumes an identifier token i.e. of the form ([a-zA-Z_][a-zA-Z0-9_]*). If 
 * current character is anything other than [a-zA-Z0-9], returns.
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume_ident(lexer_t *lex, token_t *tok) 
{
    append_char(lex, tok);

    // if *cur is a letter, digit, or _: is valid identifier char
    if(isalnum(*lex->cur) || *lex->cur == '_') {
        consume_ident(lex, tok);
    }

    tok->type = IDENT;
    return 0;
}

/**
 * Consumes a string token until the matching '"' is found. Also handles cases 
 * of escape characters and prints an error if an invalid escape character is 
 * used (similar to behavior of a CHAR_LIT)
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume_string(lexer_t *lex, token_t *tok) 
{
    append_char(lex, tok);

    if(*lex->cur == 0) { // check for EOF before string closed
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "",
            "EOF reached before string closed.");
        return 1;
    }

    if(*lex->cur == '\\') {  // check if cur is escape char
        append_char(lex, tok); 
    
        switch(*lex->cur) {
            case 'a':
            case 'b':
            case 'n':
            case 'r':
            case 't':
            case '\\':
            case '"': append_char(lex, tok); break;
            default:
            {
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Unexpected escape symbol");
                return 0;
            }
        }
    }

    if(*lex->cur == '"') {
        append_char(lex, tok);
        return 0;
    }

    if(!isprint(*lex->cur) && !isspace(*lex->cur)) { // cur is not printable
        print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, "",
            "Invalid symbol inside string literal, ignoring.");
        iterate_cur(lex); // skip invalid character
    }

    return consume_string(lex, tok);
}

/**
 * Consumes the current character under consideration by the lexer. Single or
 * double character tokens are handled directly by this function, else consume()
 * dispatches to the relevant function call to generate teh correct token. Note
 * that this function serves as an entry point for all new tokens and all 
 * dispatched functions are recursive i.e., all generated tokens enter and exit
 * from this function call.
 *
 * @param lex relevant lexer
 * @param tok token to be generated
 *
 * @return 0
 */
int 
consume(lexer_t *lex, token_t *tok) 
{
    if(isspace(*lex->cur)) { // if current char is whitespace, skip
        if(*lex->cur == '\n') { // keep track of line numbers
            (lex->line_num)++;
            tok->line_num = lex->line_num; // update token line num
        }
        iterate_cur(lex);
        return consume(lex, tok);
    }

    switch(*lex->cur) {
        case 0:
            tok->type = END; 
            break;
        case ',':
            tok->type = COMMA;
            append_char(lex, tok);
            break;
        case '.':
            if(isdigit(*(lex->cur+1))) { // lexeme of the form .[0-9]+
                return consume_real_frac(lex, tok);
            }
            tok->type = DOT;
            append_char(lex, tok);
            break;
        case ';':
            tok->type = SEMI;
            append_char(lex, tok);
            break;
        case '(':
            tok->type = LPAR;
            append_char(lex, tok);
            break;
        case ')':
            tok->type = RPAR;
            append_char(lex, tok);
            break;
        case '[':
            tok->type = LBRAK;
            append_char(lex, tok);
            break;
        case ']':
            tok->type = RBRAK;
            append_char(lex, tok);
            break;
        case '{':
            tok->type = LBRACE;
            append_char(lex, tok);
            break;
        case '}':
            tok->type = RBRACE;
            append_char(lex, tok);
            break;
        case '>':
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = GEQ;
                append_char(lex, tok);
                break;
            }
            tok->type = GT;
            break;
        case '<':
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = LEQ;
                append_char(lex, tok);
                break;
            }
            tok->type = LT;
            break;
        case '=': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = EQ;
                append_char(lex, tok);
                break;
            }
            tok->type = ASSIGN;
            break;
        case '+': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = PLUSASSIGN;
                append_char(lex, tok);
                break;
            }
            if(*lex->cur == '+') {
                tok->type = INCR;
                append_char(lex, tok);
                break;
            }
            tok->type = PLUS;
            break;
        case '-': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = MINUSASSIGN;
                append_char(lex, tok);
                break;
            }
            if(*lex->cur == '-') {
                tok->type = DECR;
                append_char(lex, tok);
                break;
            }
            tok->type = MINUS;
            break;
        case '*': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = STARASSIGN;
                append_char(lex, tok);
                break;
            }
            tok->type = STAR;
            break;
        case '%': 
            tok->type = MOD;
            append_char(lex, tok);
            break;
        case ':':
            tok->type = COLON;
            append_char(lex, tok);
            break;
        case '?':
            tok->type = QUEST;
            append_char(lex, tok);
            break;
        case '~': 
            tok->type = TILDE;
            append_char(lex, tok);
            break;
        case '|': 
            append_char(lex, tok);
            if(*lex->cur == '|') {
                tok->type = DPIPE;
                append_char(lex, tok);
                break;
            }
            tok->type = PIPE;
            break;
        case '&': 
            append_char(lex, tok);
            if(*lex->cur == '&') {
                tok->type = DAMP;
                append_char(lex, tok);
                break;
            }
            tok->type = AMP;
            break;
        case '!': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->type = NEQ;
                append_char(lex, tok);
                break;
            }
            tok->type = BANG;
            break;
        case '\'': 
            tok->type = CHAR_LIT;
            if(*(lex->cur+1) == '\\') {
                switch(*(lex->cur+2)) {
                    case 'a':
                    case 'b':
                    case 'n':
                    case 'r':
                    case 't':
                    case '\'':
                    case '\\':
                    {
                        if(*(lex->cur+3) != '\'') { // check if close quote missing
                            print_msg(LEXER_ERR, lex->filename, lex->line_num, 
                                *lex->cur, "", "Expected closing quote for " 
                                               "character literal.");
                            lex->cur = lex->cur + 3; // skip garbage characters
                            return consume(lex, tok);
                        }

                        // consume entire lexeme here (of form '\a')
                        append_char(lex, tok);
                        append_char(lex, tok);
                        append_char(lex, tok);
                        append_char(lex, tok);

                        return 0;
                    }
                    default: 
                    { 
                        print_msg(LEXER_ERR, lex->filename, lex->line_num, 
                            *(lex->cur+2), "", "Unexpected escape symbol, "
                                               "ignoring.");
                        lex->cur = lex->cur + 3; // skip garbage characters
                        return consume(lex, tok);
                    }
                }
            }

            if(*(lex->cur+1) == '\'') { // empty char i.e. '' -- error
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Empty character literal, ignoring.");
                lex->cur = lex->cur + 2; // skip garbage characters
                return consume(lex, tok);
            }

            if(!isprint(*(lex->cur+1))) { // invalid character literal
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Invalid character literal, ignoring.");
                lex->cur = lex->cur + 3; // skip garbage characters
                return consume(lex, tok);
            }
            
            if(*(lex->cur+2) != '\'') { // check if close quote missing
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Expected closing quote for character literal.");
                lex->cur = lex->cur + 3; // skip garbage characters
                return consume(lex, tok);
            }

            // consume entire lexeme (of form 'c')
            append_char(lex, tok);
            append_char(lex, tok);
            append_char(lex, tok);

            break;
        case '/': return consume_slash(lex, tok);
        case '"': 
            tok->type = STR_LIT; 
            return consume_string(lex, tok);
        case '0':
            if(*(lex->cur+1) == 'x' || *(lex->cur+1) == 'X') { // form 0[xX]
                if(!isxdigit(*(lex->cur+2))) { // not valid lexeme
                    print_msg(LEXER_ERR, lex->filename, lex->line_num, 
                        *lex->cur, "", 
                        "Invalid hexademical constant, ignoring.");
                    lex->cur = lex->cur + 2;
                    return consume(lex, tok);
                }

                // append '0x' or '0X' to lexeme
                append_char(lex, tok);
                append_char(lex, tok);
                tok->type = INT_LIT;

                return consume_hex(lex, tok);
            } 
            
            if(isoctal(*(lex->cur+1))) { // check if next char is digit 1-7
                append_char(lex, tok); // append '0' to lexeme
                tok->type = INT_LIT;

                return consume_octal(lex, tok);
            }

            if(*(lex->cur+1) == '.') {
                // append '0.' to lexeme
                append_char(lex, tok); 
                append_char(lex, tok);
                tok->type = REAL_LIT;

                return consume_real_frac(lex, tok);
            } 

            // else is an int/real -- fall through to default case
        default:
            if(isdigit(*lex->cur)) {
                return consume_int(lex, tok); // int or real
            } else if(isalpha(*lex->cur) || *lex->cur == '_') {
                return consume_ident(lex, tok);
            } else {
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Unexpected symbol, ignoring.");
                iterate_cur(lex);
                return consume(lex, tok);
            }
    }

    return 0;
}

/**
 * Checks if the text of the token is the same as a keyword. Hashes for each 
 * keyword are predetermined by a run of the hash() function and defined in
 * lexer.h. If the hash of the token text matches that of a keyword, we use
 * strcmp to verify that the token is a keyword.
 *
 * @param tok token to be checked
 *
 * @return void
 */
void 
keyword_check(token_t *tok) 
{
    uint64_t h = hash(tok->value.s);

    switch(h) {
        case VOID_HASH:
            if(strcmp(tok->value.s,"void") == 0) {
                tok->type = TYPE;
            }
            break;
        case CHAR_HASH:
            if(strcmp(tok->value.s,"char") == 0) {
                tok->type = TYPE;
            }
            break;
        case INT_HASH:
            if(strcmp(tok->value.s,"int") == 0) {
                tok->type = TYPE;
            }
            break;
        case FLOAT_HASH:
            if(strcmp(tok->value.s,"float") == 0) {
                tok->type = TYPE;
            }
            break;
        case CONST_HASH:
            if(strcmp(tok->value.s,"const") == 0) {
                tok->type = CONST;
            }
            break;
        case STRUCT_HASH: 
            if(strcmp(tok->value.s,"struct") == 0) {
                tok->type = STRUCT;
            }
            break;
        case FOR_HASH:       
            if(strcmp(tok->value.s,"for") == 0) {
                tok->type = FOR;
            }
            break;
        case WHILE_HASH:     
            if(strcmp(tok->value.s,"while") == 0) {
                tok->type = WHILE;
            }
            break;
        case DO_HASH:
            if(strcmp(tok->value.s,"do") == 0) {
                tok->type = DO;
            }
            break;
        case IF_HASH:
            if(strcmp(tok->value.s,"if") == 0) {
                tok->type = IF;
            }
            break;
        case ELSE_HASH:
            if(strcmp(tok->value.s,"else") == 0) {
                tok->type = ELSE;
            }
            break;
        case BREAK_HASH:
            if(strcmp(tok->value.s,"break") == 0) {
                tok->type = BREAK;
            }
            break;
        case CONTINUE_HASH:
            if(strcmp(tok->value.s,"continue") == 0) {
                tok->type = CONTINUE;
            }
            break;
        case RETURN_HASH: 
            if(strcmp(tok->value.s,"return") == 0) {
                tok->type = RETURN;
            }
            break;
        case SWITCH_HASH:
            if(strcmp(tok->value.s,"switch") == 0) {
                tok->type = SWITCH;
            }
            break;
        case CASE_HASH: 
            if(strcmp(tok->value.s,"case") == 0) {
                tok->type = CASE;
            }
            break;
        case DEFAULT_HASH: 
            if(strcmp(tok->value.s,"default") == 0) {
                tok->type = DEFAULT;
            }
            break;
        default: break; // else is a true identifier
    }

}


/**
 * Generates the next token according to the current state of the lexer.
 * At a high level, this function calls consume() to generate the token then
 * categorizes the token if consume() could not.
 *
 * @param lex lexer currently under use
 *
 * @return void
 */
token_t *
next_token(lexer_t *lex) 
{
    token_t *tok;

    tok = init_token(lex->filename, lex->line_num);

    consume(lex, tok);

    switch(tok->type) {
        case CHAR_LIT:  tok->value.c = *(tok->text); break;
        case INT_LIT:   tok->value.i = atoi(tok->text); break;
        case REAL_LIT:  tok->value.d = atof(tok->text); break;
        case STR_LIT:   tok->value.s = tok->text; break;
        case IDENT:     tok->value.s = tok->text; keyword_check(tok); break;
        default: break;
    }

    return tok;
}


/**
 * Initializes and returns a lexer structure that starts at the begnning of
 * the file "filename". Opens the input file and reads data into buffer.
 *
 * filename: name of input file
 *
 * @return void
 */
lexer_t *
init_lexer(char *filename)
{
    lexer_t *lex;

    lex = (lexer_t *) malloc(sizeof(lexer_t));

    FILE *fp = open_file(filename);

    lex->filename = filename;
    lex->infile = fp;
    lex->line_num = 1;

    // fill buffer
    refill_buffer(lex->infile, lex->buffer);

    lex->cur = lex->buffer; // set cur to beginning of buffer

    return lex;
}

/**
 *
 */
void
free_lexer(lexer_t *lex)
{
    free(lex);
}


/**
 * Prints a stream of tokens corresponding to the contents of filename and
 * outputs thie stream of tokens to outfile.
 *
 * filename: name of input file
 * outfile: file ptr to print stream of tokens to
 *
 * @return void
 */
void 
tokenize(char *filename) 
{
    lexer_t *lex;
    token_t *tok;

    lex = init_lexer(filename);

    tok = next_token(lex);
    while(tok->type != END) { // get tokens until EOF
        print_token(tok);
        free_token(tok);
        tok = next_token(lex);
    }

    free_token(tok);
    fclose(lex->infile);
    free_lexer(lex);
}


