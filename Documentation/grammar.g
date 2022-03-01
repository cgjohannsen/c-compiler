program : global* EOF

global : (var-decl | fun-decl | fun_def)

var-decl : 'const'? Type l-value-int var-list ';'

var-list : (',' l-value-int )*

fun-proto : fun_decl ';'

fun-decl : Type Ident '(' param-list ')'

param-list : ( Type Ident '[]'? )*

fun-def : fun-decl '{' (var-decl | statement)* '}'

statement : ';'
          | expr ';'
          | 'break' ';'
          | 'continue' ';'
          | 'return' ';'
          | 'return' expr ';'
          | if-statement
          | for-statement
          | while-statement
          | do-statement

statement-block : '{' statement* '}'

if-statement : 'if' '(' expr ')' (statement-block | statement)
             | 'if' '(' expr ')' (statement-block | statement) 'else' (statement-block | statement)

for-statement : 'for' '(' for-params ')' statement-block
              | 'for' '(' for-params ')' statement

for-params : expr? ';' expr? ';' expr?

while-statement : 'while' '(' expr ')' (statement-block | statement)

do-statement : 'do' (statement-block | statement) 'while' '(' expr ')' ';'






expr : term exprP

exprP : BinaryOp expr
      | '?' expr ':' expr
      | __emptystring__

term : literal
     | Ident '(' args-list ')'
     | l-value
     | l-value AssignOp expr
     | l-value '++' 
     | l-value '--' 
     | '++' l-value 
     | '--' l-value 
     | UnaryOp expr 
     | '(' Type ')' expr 
     | '(' expr ')' 


args-list : expr (',' expr)+
          | expr
          | __emptystring__


l-value : Ident ('[' expr ']')
l-value-int : Ident ('[' Integer ']')

literal : Integer | Char | Float | String

Type : 'const'? ( 'void' | 'char' | 'int' | 'float' )

Integer : as-defined-in-lexer
Real : as-defined-in-lexer
Char : as-defined-in-lexer
Ident : as-defined-in-lexer

UnaryOp : '-' | '!' | '~' ;
BinaryOp : '==' | '!=' | '>' | '>=' | '<' | '<=' | '+' | '-' | '*' | '-' | '/' | '%' | '|' | '&' | '||' | '&&'
AssignOp : '=' | '+=' | '-=' | '*=' | '/='