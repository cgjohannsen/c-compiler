#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../util/hash.h"
#include "../util/io.h"
#include "lexer.h"

bool preprocess = false;
bool in_else = false;

/** 
 *
 */
void 
push_ifdef(ifdef_stack_t *stack, bool b)
{
    ifdef_t *new;
    new = (ifdef_t *) malloc(sizeof(ifdef_t));
    new->cond = b;
    if(stack->size != 0) {
        new->prev = stack->top;
    } else {
        new->prev = NULL;
    }
    stack->size++;
    stack->top = new;
}

/**
 *
 */
bool 
pop_ifdef(ifdef_stack_t *stack)
{
    ifdef_t *tmp;
    bool b;
    tmp = stack->top;
    stack->top = stack->top->prev;
    stack->size--;
    b = tmp->cond;
    free(tmp);
    return b;
}

/**
 *
 */
int
add_macro(lexer_t *lex, char *name, char *text)
{
    if(lex->macros == NULL) {
        lex->macros = (macro_t *) malloc(sizeof(macro_t));
        lex->macros->name = (char *) malloc(strlen(name) + 1);
        strcpy(lex->macros->name, name);
        lex->macros->text = (char *) malloc(strlen(text) + 1);
        strcpy(lex->macros->text, text);
        lex->macros->text[strlen(text)] = 0;
        lex->macros->next = NULL;
        lex->macros->tmp = NULL;
        return 0;
    }

    macro_t *cur;
    cur = lex->macros;

    while(cur->next != NULL) {
        if(!strcmp(cur->name, name)) {
            fprintf(stderr, "error, redefining macro %s\n", name);
            exit(1);
        }
        cur = cur->next;
    }

    cur->next = (macro_t *) malloc(sizeof(macro_t));
    cur->next->name = (char *) malloc(strlen(name) + 1);
    strcpy(cur->next->name, name);
    cur->next->text = (char *) malloc(strlen(text) + 1);
    strcpy(cur->next->text, text);
    cur->next->text[strlen(text)] = 0;
    cur->next->next = NULL;
    cur->next->tmp = NULL;

    return 0;
}

/**
 *
 */
int 
is_macro(lexer_t *lex, char *name)
{
    if(lex->macros == NULL) {
        return 0;
    }

    macro_t *cur;
    cur = lex->macros;

    while(cur->next != NULL) {
        if(!strcmp(cur->name, name)) {
            return 1;
        }
        cur = cur->next;
    }

    // handle case of a single macro
    if(!strcmp(cur->name, name)) {
        return 1;
    }

    return 0;
}


/**
 *
 */
void 
remove_macro(lexer_t *lex, char *name)
{
    if(lex->macros == NULL) {
        return;
    }

    macro_t *cur, *next;
    cur = lex->macros;
    next = cur->next;

    while(next != NULL) {
        if(!strcmp(next->name, name)) {
            free(next->text);
            free(next->name);
            cur->next = next->next;
            free(next);
            return;
        }
        cur = cur->next;
        next = cur->next;
    }

    // case where only one macro defined
    if(!strcmp(cur->name, name)) {
        free(cur->text);
        free(cur->name);
        free(cur);
        lex->macros = NULL;
    }
}


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
    if(tok->tok_type == STR_LIT) {
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
                return 0;
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
        return 0;
    }

    if(*lex->cur == 0) { // check if *cur is EOF
        return 0;
    }

    return consume_cpp_comment(lex, tok);
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

    tok->tok_type = IDENT;
    return 0;
}

/** 
 *
 */
int
consume_preprocess_space(lexer_t *lex, token_t *tok)
{
    if(*(lex->cur) == ' ') {
        iterate_cur(lex);
        return consume_preprocess_space(lex, tok);
    } 

    if(*(lex->cur) == '/') {
        if(*(lex->cur+1) == '*') {
            iterate_cur(lex);
            consume_c_comment(lex, tok);
            return consume_preprocess_space(lex, tok);
        } else if(*(lex->cur+1) == '/') {
            iterate_cur(lex);
            consume_cpp_comment(lex, tok);
            return consume_preprocess_space(lex, tok);
        } else {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                "", "Invalid character in preprocess directive");
            exit(1);
        }
    }

    return 0;
}

/**
 *
 */
int
consume_preprocess_end(lexer_t *lex, token_t *tok)
{
    consume_preprocess_space(lex, tok);

    if(*lex->cur != '\n' && *lex->cur != 0) {
        print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
            "", "Invalid character in preprocess directive");
        exit(1);
    }

    iterate_cur(lex);

    return 0;
}

/**
 *
 */
int 
consume_include(lexer_t *lex, token_t *tok) 
{
    consume_preprocess_space(lex, tok);

    // reset token text, use for filename
    tok->text[0] = '\0';
    tok->text_size = 0;

    if(*lex->cur == '"') {
        iterate_cur(lex);

        while(isalnum(*lex->cur) || *lex->cur == '_' || *lex->cur == '.') {
            append_char(lex, tok);
        }

        if(*lex->cur != '"') {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', "", "Invalid filename");
            exit(1);
        }

        iterate_cur(lex);
    }

    if(lex->include_depth > 255) {
        print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', "", "Include depth reached");
        exit(1);
    }

    int i = 0;
    while(i < tok->include_depth) {
        fprintf(outfile, "..");
        i++;
    }

    fprintf(outfile, "File %s Line %*d include expansion\n", tok->filename, 5, 
        tok->line_num);

    includes_t *inc;
    inc = lex->includes;
    while(inc != NULL) {
        if(!strcmp(inc->filename, tok->text)) {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', "", "Include cycle detected");
            exit(1);
        }
        inc = inc->prev;
    }

    // handle include
    lexer_t *new_lex;
    token_t *new_tok;

    new_lex = init_lexer(tok->text, true, lex->include_depth+1, lex->macro_depth, lex->macros, lex->includes);

    new_tok = next_token(new_lex);
    while(new_tok->tok_type != END) { // get tokens until EOF
        print_token(new_tok);
        free_token(new_tok);
        new_tok = next_token(new_lex);
    }

    free_token(new_tok);
    free_lexer(new_lex);

    consume_preprocess_space(lex, tok);

    if(*lex->cur != '\n') {
        print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', "", "Invalid character following include directive");
        exit(1);
    }

    return 0;
}

/**
 *
 */
int
consume_define(lexer_t *lex, token_t *tok)
{
    consume_preprocess_space(lex, tok);

    // reset token text, use for macro name
    tok->text[0] = '\0';
    tok->text_size = 0;

    consume_ident(lex, tok);

    // create second token to store macro text
    token_t *tok2;
    tok2 = init_token(lex->filename, lex->line_num, lex->include_depth, lex->macro_depth);

    while(*lex->cur != '\n' && *lex->cur != 0) {
        append_char(lex, tok2);
    }

    add_macro(lex, tok->text, tok2->text);

    // reset token text
    tok->text[0] = '\0';
    tok->text_size = 0;

    return consume(lex, tok);
}

/** 
 *
 */
int 
consume_undef(lexer_t *lex, token_t *tok)
{
    consume_preprocess_space(lex, tok);

    // reset token text, use for macro name
    tok->text[0] = '\0';
    tok->text_size = 0;

    consume_ident(lex, tok);

    remove_macro(lex, tok->text);

    // reset token text
    tok->text[0] = '\0';
    tok->text_size = 0;

    consume_preprocess_end(lex, tok);

    return consume(lex, tok);
}

/**
 *
 */
int
consume_ifdef(lexer_t *lex, token_t *tok, bool ifdef)
{
    consume_preprocess_space(lex, tok);

    // reset token text, use for macro name
    tok->text[0] = '\0';
    tok->text_size = 0;

    consume_ident(lex, tok);

    bool cond = (is_macro(lex, tok->text) && ifdef) || 
                (!is_macro(lex, tok->text) && !ifdef);

    push_ifdef(lex->ifdef_stack, cond);

    if(!cond) {
        consume_preprocess_end(lex, tok);

        // reset token text, use for directive
        tok->text[0] = '\0';
        tok->text_size = 0;

        while(1) {
            if(*lex->cur == '#') {
                iterate_cur(lex);
                consume_preprocess_space(lex, tok);
                consume_ident(lex, tok);

                if(!strcmp(tok->text, "else")) {
                    consume_preprocess_end(lex, tok);

                    // reset token text
                    tok->text[0] = '\0';
                    tok->text_size = 0;

                    in_else = true;

                    return consume(lex, tok);
                } else if(!strcmp(tok->text, "endif")) {
                    consume_preprocess_end(lex, tok);

                    // reset token text
                    tok->text[0] = '\0';
                    tok->text_size = 0;

                    pop_ifdef(lex->ifdef_stack);

                    in_else = false;

                    return consume(lex, tok);
                }

                // reset token text
                tok->text[0] = '\0';
                tok->text_size = 0;
            }
            
            if(*lex->cur == 0) {
                print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                    "", "ifdef not closed");
                exit(1);
            }

            iterate_cur(lex);
        }
    }

    // reset token text
    tok->text[0] = '\0';
    tok->text_size = 0;

    return consume(lex, tok);
}

/**
 *
 */
int
consume_directive(lexer_t *lex, token_t *tok)
{
    // get directive
    consume_ident(lex, tok);

    if(!strcmp(tok->text, "include")) {
        return consume_include(lex, tok);
    } else if(!strcmp(tok->text, "define")) {
        return consume_define(lex, tok);
    } else if(!strcmp(tok->text, "undef")) {
        return consume_undef(lex, tok);
    } else if(!strcmp(tok->text, "ifdef")) {
        return consume_ifdef(lex, tok, true);
    } else if(!strcmp(tok->text, "ifndef")) {
        return consume_ifdef(lex, tok, false);
    } else if(!strcmp(tok->text, "else")) {
        if(lex->ifdef_stack->size == 0) {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                "", "#else without #if");
            exit(1);
        }

        if(in_else) {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                "", "second #else");
            exit(1);
        }

        consume_preprocess_end(lex, tok);

        // reset token text
        tok->text[0] = '\0';
        tok->text_size = 0;

        in_else = true;

        if(!lex->ifdef_stack->top->cond) {
            return consume(lex, tok);
        }

        while(1) {
            if(*lex->cur == '#') {
                iterate_cur(lex);
                consume_ident(lex, tok);

                if(!strcmp(tok->text, "endif")) {
                    consume_preprocess_end(lex, tok);

                    // reset token text
                    tok->text[0] = '\0';
                    tok->text_size = 0;

                    pop_ifdef(lex->ifdef_stack);

                    in_else = false;

                    return consume(lex, tok);
                }

                // reset token text
                tok->text[0] = '\0';
                tok->text_size = 0;
            }
            
            if(*lex->cur == 0) {
                print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                    "", "#if not closed");
                exit(1);
            }

            iterate_cur(lex);
        }

        return consume(lex, tok);
    } else if(!strcmp(tok->text, "endif")) {
        if(lex->ifdef_stack->size == 0) {
            print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
                "", "#endif without #if");
            exit(1);
        }

        // reset token text
        tok->text[0] = '\0';
        tok->text_size = 0;

        pop_ifdef(lex->ifdef_stack);

        in_else = false;

        return consume(lex, tok);
    } 
        
    print_msg(PREPROCESS_ERR, lex->filename,  lex->line_num, ' ', 
        "", "unknown preprocess directive");
    exit(1);
}

/**
 *
 */
int
consume_preprocess(lexer_t *lex, token_t *tok)
{
    iterate_cur(lex);
    consume_preprocess_space(lex, tok);
    return consume_directive(lex, tok);
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
        consume_c_comment(lex, tok);
        return consume(lex, tok);
    }

    if(*(lex->cur+1) == '/') {
        iterate_cur(lex);
        consume_cpp_comment(lex, tok);
        return consume(lex, tok);
    }

    if(*(lex->cur+1) == '=') {
        tok->tok_type = SLASHASSIGN;
        append_char(lex, tok);
        append_char(lex, tok);
        return 0;
    }

    append_char(lex, tok);
    tok->tok_type = SLASH;
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
            tok->tok_type = REAL_LIT;
        }
        return consume_real_frac(lex, tok);
    }
    
    if(*lex->cur == 'e' || *lex->cur == 'E') { // token is REAL_LIT w exp
        if(status) { // check if truncated before any of 'e', 'E'
            tok->tok_type = REAL_LIT;
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
    
    tok->tok_type = INT_LIT;
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
            if(lex->ifdef_stack->size > 0 || in_else) {
                print_msg(PREPROCESS_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "#ifdef not closed");
                exit(1);
            }

            tok->tok_type = END; 
            break;
        case '#':
            if(preprocess) {
                return consume_preprocess(lex, tok);
            } else {
                print_msg(LEXER_ERR, lex->filename, lex->line_num, *lex->cur, 
                    "", "Invalid symbol, ignoring.");
                iterate_cur(lex); // skip invalid character
                return consume(lex, tok);
            }
        case ',':
            tok->tok_type = COMMA;
            append_char(lex, tok);
            break;
        case '.':
            if(isdigit(*(lex->cur+1))) { // lexeme of the form .[0-9]+
                return consume_real_frac(lex, tok);
            }
            tok->tok_type = DOT;
            append_char(lex, tok);
            break;
        case ';':
            tok->tok_type = SEMI;
            append_char(lex, tok);
            break;
        case '(':
            tok->tok_type = LPAR;
            append_char(lex, tok);
            break;
        case ')':
            tok->tok_type = RPAR;
            append_char(lex, tok);
            break;
        case '[':
            tok->tok_type = LBRAK;
            append_char(lex, tok);
            break;
        case ']':
            tok->tok_type = RBRAK;
            append_char(lex, tok);
            break;
        case '{':
            tok->tok_type = LBRACE;
            append_char(lex, tok);
            break;
        case '}':
            tok->tok_type = RBRACE;
            append_char(lex, tok);
            break;
        case '>':
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = GEQ;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = GT;
            break;
        case '<':
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = LEQ;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = LT;
            break;
        case '=': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = EQ;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = ASSIGN;
            break;
        case '+': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = PLUSASSIGN;
                append_char(lex, tok);
                break;
            }
            if(*lex->cur == '+') {
                tok->tok_type = INCR;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = PLUS;
            break;
        case '-': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = MINUSASSIGN;
                append_char(lex, tok);
                break;
            }
            if(*lex->cur == '-') {
                tok->tok_type = DECR;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = MINUS;
            break;
        case '*': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = STARASSIGN;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = STAR;
            break;
        case '%': 
            tok->tok_type = MOD;
            append_char(lex, tok);
            break;
        case ':':
            tok->tok_type = COLON;
            append_char(lex, tok);
            break;
        case '?':
            tok->tok_type = QUEST;
            append_char(lex, tok);
            break;
        case '~': 
            tok->tok_type = TILDE;
            append_char(lex, tok);
            break;
        case '|': 
            append_char(lex, tok);
            if(*lex->cur == '|') {
                tok->tok_type = DPIPE;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = PIPE;
            break;
        case '&': 
            append_char(lex, tok);
            if(*lex->cur == '&') {
                tok->tok_type = DAMP;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = AMP;
            break;
        case '!': 
            append_char(lex, tok);
            if(*lex->cur == '=') {
                tok->tok_type = NEQ;
                append_char(lex, tok);
                break;
            }
            tok->tok_type = BANG;
            break;
        case '\'': 
            tok->tok_type = CHAR_LIT;
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
            tok->tok_type = STR_LIT; 
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
                tok->tok_type = INT_LIT;

                return consume_hex(lex, tok);
            } 
            
            if(isoctal(*(lex->cur+1))) { // check if next char is digit 1-7
                append_char(lex, tok); // append '0' to lexeme
                tok->tok_type = INT_LIT;

                return consume_octal(lex, tok);
            }

            if(*(lex->cur+1) == '.') {
                // append '0.' to lexeme
                append_char(lex, tok); 
                append_char(lex, tok);
                tok->tok_type = REAL_LIT;

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
keyword_check(lexer_t *lex, token_t *tok) 
{
    uint64_t h = hash(tok->value.s);
    macro_t *cur;

    switch(h) {
        case VOID_HASH:
            if(strcmp(tok->value.s,"void") == 0) {
                tok->tok_type = TYPE;
            }
            break;
        case CHAR_HASH:
            if(strcmp(tok->value.s,"char") == 0) {
                tok->tok_type = TYPE;
            }
            break;
        case INT_HASH:
            if(strcmp(tok->value.s,"int") == 0) {
                tok->tok_type = TYPE;
            }
            break;
        case FLOAT_HASH:
            if(strcmp(tok->value.s,"float") == 0) {
                tok->tok_type = TYPE;
            }
            break;
        case CONST_HASH:
            if(strcmp(tok->value.s,"const") == 0) {
                tok->tok_type = CONST;
            }
            break;
        case STRUCT_HASH: 
            if(strcmp(tok->value.s,"struct") == 0) {
                tok->tok_type = STRUCT;
            }
            break;
        case FOR_HASH:       
            if(strcmp(tok->value.s,"for") == 0) {
                tok->tok_type = FOR;
            }
            break;
        case WHILE_HASH:     
            if(strcmp(tok->value.s,"while") == 0) {
                tok->tok_type = WHILE;
            }
            break;
        case DO_HASH:
            if(strcmp(tok->value.s,"do") == 0) {
                tok->tok_type = DO;
            }
            break;
        case IF_HASH:
            if(strcmp(tok->value.s,"if") == 0) {
                tok->tok_type = IF;
            }
            break;
        case ELSE_HASH:
            if(strcmp(tok->value.s,"else") == 0) {
                tok->tok_type = ELSE;
            }
            break;
        case BREAK_HASH:
            if(strcmp(tok->value.s,"break") == 0) {
                tok->tok_type = BREAK;
            }
            break;
        case CONTINUE_HASH:
            if(strcmp(tok->value.s,"continue") == 0) {
                tok->tok_type = CONTINUE;
            }
            break;
        case RETURN_HASH: 
            if(strcmp(tok->value.s,"return") == 0) {
                tok->tok_type = RETURN;
            }
            break;
        case SWITCH_HASH:
            if(strcmp(tok->value.s,"switch") == 0) {
                tok->tok_type = SWITCH;
            }
            break;
        case CASE_HASH: 
            if(strcmp(tok->value.s,"case") == 0) {
                tok->tok_type = CASE;
            }
            break;
        case DEFAULT_HASH: 
            if(strcmp(tok->value.s,"default") == 0) {
                tok->tok_type = DEFAULT;
            }
            break;
        default:
            if(lex->macros == NULL) {
                break;
            }

            cur = lex->macros;
            while(cur != NULL) {
                if(!strcmp(cur->name, tok->text)) {
                    if(cur->tmp_macro != NULL) {
                        fprintf(stderr, "error, macro expansion cycle detected\n");
                        exit(1);
                    }
                    if(lex->macro_depth >= 255) {
                        fprintf(stderr, "error, macro expansion depth exceeded\n");
                        exit(1);
                    }
                    if(lex->cur_macro != NULL) {
                        cur->tmp_macro = lex->cur_macro;
                    } 
                    cur->tmp = lex->cur;
                    lex->macro_depth++;
                    lex->cur_macro = cur;
                    lex->cur = cur->text;
                    tok->tok_type = MACRO;
                    break;
                }
                cur = cur->next;
            }

            break;
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

    tok = init_token(lex->filename, lex->line_num, lex->include_depth, lex->macro_depth);

    macro_t *tmp1;

    if(lex->cur_macro != NULL) {
        if(*lex->cur == 0) {
            lex->macro_depth--;
            tok->macro_depth--;
            if(lex->cur_macro->tmp_macro != NULL) {
                tmp1 = lex->cur_macro;
                lex->cur = lex->cur_macro->tmp;
                lex->cur_macro = lex->cur_macro->tmp_macro;
                tmp1->tmp_macro = NULL;
                tmp1->tmp = NULL;
                tok->macro_name = lex->cur_macro->name;
            } else {
                lex->cur = lex->cur_macro->tmp;
                lex->cur_macro->tmp = NULL;
                lex->cur_macro = NULL;
            }
        } else {
            tok->macro_name = lex->cur_macro->name;
        }
    }

    consume(lex, tok);

    switch(tok->tok_type) {
        case CHAR_LIT:  
            tok->value.c = *(tok->text); 
            break;
        case INT_LIT:   
            tok->value.i = atoi(tok->text); 
            break;
        case REAL_LIT:  
            tok->value.d = atof(tok->text); 
            break;
        case STR_LIT:   
            tok->value.s = tok->text; 
            break;
        case IDENT:     
            tok->value.s = tok->text; 
            keyword_check(lex, tok); 
            break;
        default: 
            break;
    }

    return tok;
}


/**
 * Initializes and returns a lexer structure that starts at the beginning of
 * the file "filename". Opens the input file and reads data into buffer.
 *
 * filename: name of input file
 *
 * @return void
 */
lexer_t *
init_lexer(char *filename, bool p, int include_depth, int macro_depth, macro_t *macros, includes_t *includes)
{
    preprocess = p;

    lexer_t *lex;

    lex = (lexer_t *) malloc(sizeof(lexer_t));

    FILE *fp = open_file(filename);

    lex->filename = filename;
    lex->infile = fp;
    lex->line_num = 1;
    lex->include_depth = include_depth;
    lex->macro_depth = macro_depth;
    lex->macros = macros;
    lex->cur_macro = NULL;
    lex->ifdef_stack = (ifdef_stack_t *) malloc(sizeof(ifdef_stack_t));
    lex->ifdef_stack->size = 0;

    includes_t *inc;

    inc = (includes_t *) malloc(sizeof(includes_t));
    inc->filename = (char *) malloc(strlen(filename) + 1);
    strcpy(inc->filename, filename);
    if(includes != NULL) {
        includes->next = inc;
    }
    inc->prev = includes;
    inc->next = NULL;

    lex->includes = inc;

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
    free(lex->includes->filename);
    free(lex->includes);
    free(lex->ifdef_stack);
    free(lex);
}


/**
 * Prints a stream of tokens corresponding to the contents of filename and
 * outputs the stream of tokens to outfile.
 *
 * filename: name of input file
 * outfile: file ptr to print stream of tokens to
 *
 * @return void
 */
void 
tokenize(char *filename, bool p) 
{
    lexer_t *lex;
    token_t *tok;

    lex = init_lexer(filename, p, 0, 0, NULL, NULL);

    tok = next_token(lex);
    while(tok->tok_type != END) { // get tokens until EOF
        print_token(tok);
        free_token(tok);
        tok = next_token(lex);
    }

    free_token(tok);
    free_lexer(lex);
}


