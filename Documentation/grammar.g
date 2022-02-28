program : statement* EOF ;

statement : (var_dec | fun_decl | fun_def) ;

var_decl : 'const'? Type Ident ('[' Integer ']')? (',' Ident ('[' Integer ']')?)* ';' ;

fun_proto : fun_decl ';' ;

fun_decl : Type Ident '(' fun_param (',' fun_param)* ')'

fun_param : Type Ident ('[]')?

fun_def : fun_decl '{' (var_decl | statement)* '}' ;

statement : ';'
          | expr ';'
          | 'break' ';'
          | 'continue' ';'
          | 'return' expr? ';'
          | if-statement
          | for-statement
          | while-statement
          | do-statement
          ;

statement-block : '{' statement* '}'

if-statement : 'if' '(' expr ')' (statement-block | statement ';') ('else' (statement-block | statement ';'))? ;

for-statement : 'for' '(' expr? ';' expr? ';' expr? ')' (statement-block | statement ';') ;

while-statement : 'while' '(' expr ')' (statement-block | statement ';') ;

do-statement : 'do' (statement-block | statement ';') 'while' '(' expr ')' ';' ;

expr : Integer ;



Type : 'void'
     | 'char'
     | 'int'
     | 'float'
     ;

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;





UnaryOp : '-' | '!' | '~' ;
BinaryOp : '==' | '!=' | '>' | '>=' | '<' | '<=' | '+' | '-' | '*' | '-' | '/' | '%' | '|' | '&' | '||' | '&&' ;
AssignOp : '=' | '+=' | '-=' | '*=' | '/=' ;