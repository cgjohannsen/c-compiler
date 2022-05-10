
%{

#include "lexer.h"

%}

%option yylineno noyywrap
%x TYPES COMMENT ANNOTE

whitespace    [ \t\r]
space         {whitespace}+

digit         [0-9]
dec           ("."{digit}+)
exp           ([eE][-+]?{digit}+)

letter        [A-Za-z_<]
alphanum      [A-Za-z_0-9.>]
ident         {letter}{alphanum}*
pathname      {ident}("/"{ident})*

dqstring      (\"([^\"\n]|("\\"[abnrt\\\"]))*\")
sqstring      (\'([^\'\n]|("\\"[abnrt\\\']))*\')

type          \[?([VICF]|("L"{pathname}";"))

%%

<TYPES>{space}            { /* ignore */ }
<TYPES>"("                { return '('; }
<TYPES>")"                { return ')'; }
<TYPES>{type}             { return processType(yytext); }
<TYPES>"\n"               { return '\n'; }

";;".*                    { return processNote(yytext); }

{space}                   { /* ignore */ }

[-+]?{digit}+                 { return processInteger(yytext); }

[-+]?{digit}+"f"              { return buildToken(FLOAT, yytext); }
[-+]?{digit}+{dec}?{exp}?"f"  { return buildToken(FLOAT, yytext); }
[-+]?{dec}{exp}?"f"           { return buildToken(FLOAT, yytext); }

{dqstring}                { return buildToken(STRING, yytext); }
{sqstring}                { return buildToken(STRING, yytext); }

".class"                  { return dCLASS; }
".version"                { return dVERSION; }
".super"                  { return dSUPER; }
".implements"             { return dIMPLEMENTS; }
".field"                  { return dFIELD; }
".method"                 { return dMETHOD; }
".code"                   { return dCODE; }
".linenumbertable"        { return dLNT; }
".sourcefile"             { return dSRCFILE; }

".end"{space}"class"            { return eCLASS; }
".end"{space}"code"             { return eCODE; }
".end"{space}"method"           { return eMETHOD; }
".end"{space}"linenumbertable"  { return eLNT; }

"public"                  { return buildToken(FLAG, yytext); }
"private"                 { return buildToken(FLAG, yytext); }
"protected"               { return buildToken(FLAG, yytext); }
"static"                  { return buildToken(FLAG, yytext); }
"final"                   { return buildToken(FLAG, yytext); }
"super"                   { return buildToken(FLAG, yytext); }
"synchronized"            { return buildToken(FLAG, yytext); }
"volatile"                { return buildToken(FLAG, yytext); }
"bridge"                  { return buildToken(FLAG, yytext); }
"transient"               { return buildToken(FLAG, yytext); }
"varargs"                 { return buildToken(FLAG, yytext); }
"native"                  { return buildToken(FLAG, yytext); }
"interface"               { return buildToken(FLAG, yytext); }
"abstract"                { return buildToken(FLAG, yytext); }
"strict"                  { return buildToken(FLAG, yytext); }
"synthetic"               { return buildToken(FLAG, yytext); }
"annotation"              { return buildToken(FLAG, yytext); }
"enum"                    { return buildToken(FLAG, yytext); }
"mandated"                { return buildToken(FLAG, yytext); }

"stack"                   { return STACK; }
"locals"                  { return LOCALS; }

"L"{digit}+":"?           { return processLabel(yytext); }

[acif]"load"([_]|{space}){digit}+   { return processLoad(yytext); }
[acif]"store"([_]|{space}){digit}+  { return processStore(yytext); }
[cif]"aload"                        { return processLoad(yytext); }
[cif]"astore"                       { return processStore(yytext); }

("fconst_"[012])|("iconst_"[0-5]|"iconst_m1")  { return processConst(yytext); }

[if]"add"                 { return processBinary('+', yytext); }
[if]"sub"                 { return processBinary('-', yytext); }
[if]"mul"                 { return processBinary('*', yytext); }
[if]"div"                 { return processBinary('/', yytext); }
[if]"rem"                 { return processBinary('%', yytext); }
"iand"                    { return processBinary('&', yytext); }
"ior"                     { return processBinary('|', yytext); }
"ixor"                    { return processBinary('^', yytext); }

[if]"neg"                 { return processUnary('-', yytext); }
"i2c"|"i2f"|"f2i"         { return processConv(yytext); }

[bs]"ipush"               { return processPush(yytext); }
"dup"("_x"[12])?          { return processDup(yytext); }
"swap"                    { return SWAP; }
"pop"                     { return POP; }

"iinc"                    { return IINC; }

[if]?"return"             { return processReturn(yytext); }

"ldc"                     { return LDC; }
"getstatic"               { return GETSTATIC; }
"putstatic"               { return PUTSTATIC; }
"invokestatic"            { return INVOKESTATIC; }
"invokespecial"           { return INVOKESPECIAL; }
"invokevirtual"           { return INVOKEVIRTUAL; }

"newarray"                { return NEWARRAY; }

"goto"                    { return GOTO; }

{pathname}                { return buildToken(WORD, yytext); }

;                         { BEGIN(COMMENT); }
<COMMENT>.*               { /* ignore */ }
<COMMENT>\n               { BEGIN(INITIAL); return '\n'; }
\n                        { return '\n'; }

.                         { return yytext[0]; }

%%

void types_On()
{
  BEGIN(TYPES);
}

void types_Off()
{
  BEGIN(INITIAL);
}

