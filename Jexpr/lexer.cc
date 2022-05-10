
#include "lexer.h"

int processType(const char* text)
{
  if ('[' == text[0]) {
    yylval.is_array = true;
    yylval.type = text[1];
    if ('L' == yylval.type) {
      yylval.string = text+2;
    } else {
      yylval.string = 0;
    }
  } else {
    yylval.is_array = false;
    yylval.type = text[0];
    if ('L' == yylval.type) {
      yylval.string = text+1;
    } else {
      yylval.string = 0;
    }
  }
  return TYPE;
}

int trailingInt(const char* text)
{
  if (0==text) return 0;

  // Go forward until 0 terminator
  for (; *text; text++);

  int value = 0;
  int mult = 1;
  // Go backward until non-digit
  for (;;) {
    text--;
    if ('m' == *text) {
      // special case
      return -value;
    }
    if (*text > '9') return value;
    if (*text < '0') return value;
    value += (*text - '0') * mult;
    mult *= 10;
  }
}

int processLoad(const char* text)
{
  yylval.type = text[0];
  yylval.is_array = ('a' == text[1]);
  yylval.integer = trailingInt(text);
  return LOAD;
}

int processStore(const char* text)
{
  yylval.type = text[0];
  yylval.is_array = ('a' == text[1]);
  yylval.integer = trailingInt(text);
  return STORE;
}

int processConst(const char* text)
{
  yylval.type = text[0];
  yylval.integer = trailingInt(text);
  return CONST;
}

int processPush(const char* text)
{
  yylval.type = text[0];
  return PUSH;
}

int processDup(const char* text)
{
  yylval.integer = trailingInt(text);
  return DUP;
}

