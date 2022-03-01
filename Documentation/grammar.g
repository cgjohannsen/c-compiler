program : global* EOF

global : (var-decl | type-decl | fun-decl | fun_def)

var-decl : type var-list ';'

var-list : Ident '[' Integer ']' (',' var-list)?
         | Ident '=' expr (',' var-list)?
         | Ident (',' var-list)?

type-decl : 'struct' Ident '{' var-decl-no-init '}' ';'

var-decl-no-init : type var-list-no-init ';'

var-list-no-init : Ident '[' Integer ']' (',' var-list)?
                 | Ident (',' var-list)?

fun-proto : fun-decl ';'

fun-decl : type Ident '(' param-list ')'

param-list : ( type Ident '[]'? )*

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
     | '(' type ')' expr 
     | '(' expr ')' 

args-list : expr (',' expr)+
          | expr
          | __emptystring__

l-value : Ident ('[' expr ']')? ('.' l-value)?

literal : Integer | Char | Float | String

type : 'const'? TypeName
     | TypeName 'const'? 
     | 'const'? 'struct' Ident
     | 'struct' Ident 'const'?

TypeName : 'void' | 'char' | 'int' | 'float'

Integer : as-defined-in-lexer
Real : as-defined-in-lexer
Char : as-defined-in-lexer
Ident : as-defined-in-lexer

UnaryOp : '-' | '!' | '~' ;
BinaryOp : '==' | '!=' | '>' | '>=' | '<' | '<=' | '+' | '-' | '*' | '-' | '/' | '%' | '|' | '&' | '||' | '&&'
AssignOp : '=' | '+=' | '-=' | '*=' | '/='