
#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

const int INTEGER     = 300;
const int FLOAT       = 301;
const int STRING      = 302;

const int dCLASS      = 303;
const int dVERSION    = 304;
const int dSUPER      = 305;
const int dIMPLEMENTS = 306;
const int dFIELD      = 307;
const int dMETHOD     = 308;
const int dCODE       = 309;
const int dLNT        = 310;
const int dSRCFILE    = 311;

const int eCLASS      = 312;
const int eCODE       = 313;
const int eMETHOD     = 314;
const int eLNT        = 315;

const int LABEL       = 316;
// const int OBJECT      = 317;
const int WORD        = 318;

const int NOTE        = 319;  /* ;; */

const int FLAG        = 321;
const int TYPE        = 322;

const int STACK       = 323;
const int LOCALS      = 324;

// Instructions

const int LOAD        = 400;
const int STORE       = 401;
const int CONST       = 402;
const int BINARY      = 403;
const int UNARY       = 404;
const int CONV        = 405;

const int PUSH        = 406;
const int DUP         = 407;
const int SWAP        = 408;
const int POP         = 409;

const int IINC        = 410;

const int RETURN      = 411;

const int LDC         = 412;

const int GETSTATIC     = 413;
const int PUTSTATIC     = 414;
const int INVOKESTATIC  = 415;
const int INVOKESPECIAL = 416;
const int INVOKEVIRTUAL = 417;

const int NEWARRAY = 418;

// Jumping instructions

const int GOTO  =   450;


struct attributes {
  int integer;
  const char* string;
  bool is_array;
  char type;
};

extern attributes yylval;

inline int buildToken(int tokenid, const char* text) 
{
  yylval.string = text;
  return tokenid;
}

inline int processInteger(const char* text)
{
  yylval.integer = atol(text);
  return buildToken(INTEGER, text);
}

inline int processLabel(const char* text)
{
  yylval.integer = atol(text+1);
  return buildToken(LABEL, text);
}

int processType(const char* text);

int processLoad(const char* text);
int processStore(const char* text);

int processConst(const char* text);
int processPush(const char* text);

inline int processBinary(char op, const char* text)
{
  yylval.integer = op;
  yylval.type = text[0];
  return BINARY;
}

inline int processUnary(char op, const char* text)
{
  yylval.integer = op;
  yylval.type = text[0];
  return UNARY;
}

inline int processConv(const char* text)
{
  yylval.type = text[0];
  yylval.integer = text[2];
  return CONV;
}

int processDup(const char* text);

inline int processReturn(const char* text) 
{
  yylval.type = ('r' != text[0]) ? text[0] : ' ';
  return RETURN;
}

inline int processNote(const char* text)
{
  yylval.string = text;
  return NOTE;
}

/*
  These are provided in tokens.l
*/

void types_On();
void types_Off();


#endif
