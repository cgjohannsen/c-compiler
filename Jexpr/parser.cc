
#include "parser.h"
#include "lexer.h"
#include "tokens.h"
#include "instr.h"

// #define DEBUG_PARSER
// #define DEBUG_J

attributes yylval;

//
// typeinfo methods
//

typeinfo::typeinfo()
{
  is_array = false;
  letter = '?';
  objname = 0;
}

typeinfo::~typeinfo()
{
  free(objname);
}

void typeinfo::set(const attributes &A)
{
  is_array = A.is_array;
  letter = A.type;
  free(objname);
  objname = A.string ? strdup(A.string) : 0;
}

void typeinfo::set(const typeinfo &T)
{
  is_array = T.is_array;
  letter = T.letter;
  free(objname);
  objname = T.objname ? strdup(T.objname) : 0;
}

void typeinfo::show(std::ostream &out) const
{
  if (is_array) out << "[";
  out << letter;
  if (objname) out << objname;
}

bool typeinfo::equals(const typeinfo &t) const
{
  if (is_array != t.is_array) return false;
  if (letter != t.letter) return false;
  if ((0==objname) && (0==t.objname)) return true;
  if ((0==objname) || (0==t.objname)) return false;
  return 0==strcmp(objname, t.objname);
}

bool typeinfo::equals(char ltr) const
{
  if (is_array) return false;
  if (objname) return false;
  return (ltr == letter);
}

//
// variable methods
//

variable::variable(const attributes &A, char* n)
{
  name = n;
  Type.set(A);
}

variable::~variable()
{
  free(name);
}

void variable::show(std::ostream &out) const
{
  out << name << " " << Type;
}

bool variable::equals(const variable* v) const
{
  if (0==v) return false;
  if (strcmp(name, v->name)) return false;
  return Type.equals(v->Type);
}

void variable::writeDotj(std::ostream &out) const
{
  out << ".field public static " << name << " " << Type << "\n";
}

//
// method methods (ugh)
//

const unsigned max_params = 16;
const unsigned max_instrs = 65536;

method::method(const char* n)
{
  name = strdup(n);
  stack = -1;
  locals = -1;
  Types = new typeinfo[max_params];
  num_params = 0;

  code = new instruction*[max_instrs];
  for (unsigned i=0; i<max_instrs; i++) code[i] = 0;
  codelen = 0;

#ifdef DEBUG_J
  std::cout << "DEBUG_J: starting method " << name << "\n";
#endif
}

method::~method()
{
  free(name);
  delete[] Types;
  for (unsigned i=0; i<codelen; i++) delete code[i];
  delete[] code;
}

void method::setReturn(const attributes &A)
{
  Types[0].set(A);
#ifdef DEBUG_J
  std::cout << "DEBUG_J: setting return for method " << name << " to " << Types[0] << "\n";
#endif
}

void method::addParam(const attributes &A)
{
  if (num_params >= max_params) {
    std::cerr << "Internal error: too many parameters for method " << name << "\n";
    exit(1);
    return;
  }
  Types[++num_params].set(A);
#ifdef DEBUG_J
  std::cout << "DEBUG_J: setting parameter " << num_params << " for method " << name << " to " << Types[num_params] << "\n";
#endif
}

void method::setStack(int s)
{
  stack = s;
#ifdef DEBUG_J
  std::cout << "DEBUG_J: setting stack for method " << name << " to " << stack << "\n";
#endif
}

void method::setLocals(int s)
{
  locals = s;
#ifdef DEBUG_J
  std::cout << "DEBUG_J: setting locals for method " << name << " to " << locals << "\n";
#endif
}

void method::addInstruction(instruction* I)
{
  if (0==I) return;

#ifdef DEBUG_PARSER
  std::cout << "DEBUG_PARSER: got " << (I->isLabel() ? "label" : "instruction");
  std::cout << " " << I << "\n";
#endif
  if (codelen >= max_instrs) {
    std::cerr << "Internal error: too many instructions in method " << name << "\n";
    exit(1);
    return;
  }
  code[codelen++] = I;
}

void method::doneMethod()
{
  // collapse the code array
  instruction** oldcode = code;
  code = codelen ? new instruction*[codelen] : 0;
  for (unsigned i=0; i<codelen; i++) code[i] = oldcode[i];
  delete[] oldcode;
#ifdef DEBUG_PARSER
  std::cout << "DEBUG_PARSER: done parsing method " << name << "\n\n";
#endif
}

void method::writeDotj(std::ostream &s, bool with_labels, long line) const
{
	bool stream_on = line<0;

	if (stream_on) {
		s << ".method public static " << name << " : (";
		for (unsigned i=1; i<=num_params; i++) s << Types[i];
		s << ")" << Types[0] << "\n";
		s << "    .code stack " << stack << " locals " << locals << "\n";
	}
  for (unsigned i=0; i<codelen; i++) {
    if (0==code[i]) continue;
		code[i]->activate(stream_on, line);
		if (!stream_on) continue;
    if ( (!with_labels) && (code[i]->isLabel()) ) continue;
    s << code[i];
  }
	stream_on = line<0;
	if (stream_on) {
		s << "    .end code\n";
		s << ".end method\n\n";
	}
}

void method::writeExprs(environment &e, long line) const
{
  e.startMethod(name, stack);

	if (line<0) {
		e << Types[0] << " " << name << " ( ";
		for (unsigned i=1; i<=num_params; i++) {
			if (i>1) e << ", ";
			e << Types[i];
		}
		e << " )\n{\n";
	}

	bool printed = false;
  for (unsigned i=0; i<codelen; i++) {
    if (0==code[i]) continue;
    if (code[i]->execute(e)) printed = true;
  }

  e.endMethod(printed);
  if (line<0) e << "}\n\n";
}

//
// dotjfile methods
//

const unsigned max_globals = 1024;
const unsigned max_methods = 1024;

dotjfile::dotjfile(const char* fn)
{
  filename = fn;
  classname = 0;

  globals = new variable*[max_globals];
  for (unsigned i=0; i<max_globals; i++) {
    globals[i] = 0;
  }
  num_globals = 0;

  methods = new method*[max_methods];
  for (unsigned i=0; i<max_methods; i++) {
    methods[i] = 0;
  }
  num_methods = 0;

#ifdef DEBUG_J
  std::cout << "DEBUG_J: starting object for ";
  if (fn) std::cout << "file " << fn << "\n";
  else    std::cout << "standard input\n";
#endif
}

dotjfile::~dotjfile()
{
  free(classname);
  for (unsigned i=0; i<num_globals; i++) {
    delete globals[i];
  }
  delete[] globals;
  for (unsigned i=0; i<num_methods; i++) {
    delete methods[i];
  }
  delete[] methods;
}

void dotjfile::setClassName(const char* cn)
{
  classname = strdup(cn);
#ifdef DEBUG_J
  std::cout << "DEBUG_J: set class name '" << classname << "'\n";
#endif
}

void dotjfile::addGlobal(const attributes &A, char* varname)
{
  variable* V = new variable(A, varname);
  if (num_globals >= max_globals) {
    std::cerr << "Internal error: too many global variables\n";
    delete V;
    exit(1);
    return; // nice
  }
  globals[num_globals++] = V;

#ifdef DEBUG_J
  std::cout << "DEBUG_J: added global variable ";
  V->show(std::cout);
  std::cout << "\n";
#endif
}

void dotjfile::addMethod(method* M)
{
  if (num_methods >= max_methods) {
    std::cerr << "Internal error: too many global variables\n";
    delete M;
    exit(1);
    return; // nice
  }
  methods[num_methods++] = M;
}

void dotjfile::doneFile()
{
  // Compress arrays
  variable** oldglobals = globals;
  globals = num_globals ? new variable*[num_globals] : 0;
  for (unsigned i=0; i<num_globals; i++) globals[i] = oldglobals[i];
  delete[] oldglobals;

  method** oldmethods = methods;
  methods = num_methods ? new method*[num_methods] : 0;
  for (unsigned i=0; i<num_methods; i++) methods[i] = oldmethods[i];
  delete[] oldmethods;
}

void dotjfile::startError(int line) const
{
  if (filename) std::cerr << "Error in file " << filename;
  else          std::cerr << "Error in standard input";
  if (line>0)   std::cerr << " line " << line << ": ";
  else          std::cerr << ": ";
}

void dotjfile::expecting(const char* what) const
{
  std::cerr << "expecting " << what << ", got ";
  if ('\n' != yytext[0]) {
    std::cerr << "'" << yytext << "'\n";
  } else {
    std::cerr << "end of line\n";
  }
}

void dotjfile::eofbefore(const char* what) const
{
  std::cerr << "end of input file before expected " << what << "\n";
}

void dotjfile::writeDotj(std::ostream& s, const char* meth, long line,
		bool with_labels) const
{
	if (line<0) {
		if (classname) s << ".class public " << classname << "\n";
		s << ".super java/lang/Object\n";
		s << "\n";

		if (num_globals) {
			s << "; Global variables\n\n";
			for (unsigned i=0; i<num_globals; i++)
				if (globals[i]) globals[i]->writeDotj(s);
			s << "\n";
		}
	}
  for (unsigned i=0; i<num_methods; i++) {
    if (methods[i]) {
      if (meth) {
        if (strcmp(meth, methods[i]->getName())) continue;
      }
      methods[i]->writeDotj(s, with_labels, line);
    }
  }

	if (line<0) {
		s << ".end class\n";
	}
}

void dotjfile::writeExprs(std::ostream& s, const char* meth, long line) const
{
  environment E(s, line);

  for (unsigned i=0; i<num_methods; i++)
    if (methods[i]) {
      if (meth) {
        if (strcmp(meth, methods[i]->getName())) continue;
      }
      methods[i]->writeExprs(E, line);
    }
}

//
// Back end
//

inline void showToken(const char* what, int tk)
{
#ifdef DEBUG_PARSER
  std::cout << "DEBUG_PARSER: " << what << " token " << tk;
  if (tk != '\n') std::cout << " text " << yytext;
  std::cout << '\n';
#endif
}

int skipUntilToken(int match)
{
  for (;;) {
    int tk = yylex();
    if (tk == match) {
      showToken("Got", tk);
      return tk;
    }
    showToken("Skipping", tk);
    if (0 == tk) return 0;
  }
}

int skipOnLineUntil(int match)
{
  for (;;) {
    int tk = yylex();
    showToken("Got", tk);
    if (tk == match) return tk;
    if (0==tk) return 0;
    if ('\n' == tk) return 0;
  }
}

int nextLine()
{
  for (;;) {
    int tk = yylex();
    showToken("Skipped", tk);
    if ('\n' == tk) break;
    if (0==tk) return 0;
  }
  for (;;) {
    int tk = yylex();
    if (tk != '\n') {
      showToken("Got", tk);
      return tk;
    }
    showToken("Skipped", tk);
  }
}

void parseField(dotjfile* J)
{
  skipOnLineUntil(WORD);
#ifdef DEBUG_PARSER
  std::cout << "DEBUG_PARSER: parsing global variable " << yylval.string << "\n";
#endif
  char* varname = strdup(yylval.string);
  types_On();
  if (TYPE != yylex()) {
    J->startError(yylineno);
    J->expecting("type");
    yylval.is_array = false;
    yylval.type = '?';
    yylval.string = 0;
  }
  // get the type
  // add it to J
  types_Off();
  J->addGlobal(yylval, varname);
}

void parseMethod(dotjfile* J)
{
  skipOnLineUntil(WORD);
#ifdef DEBUG_PARSER
  std::cout << "DEBUG_PARSER: parsing method " << yylval.string << "\n";
#endif
  method* M = new method(yylval.string);
  J->addMethod(M);
  if (':' != yylex()) {
    J->startError(yylineno);
    J->expecting("':'");
  }
  types_On();
  if ('(' != yylex()) {
    J->startError(yylineno);
    J->expecting("'('");
  }
  int tk = yylex();
  while (TYPE == tk) {
    M->addParam(yylval);
    tk = yylex();
  }
  if (')' != tk) {
    J->startError(yylineno);
    J->expecting("')'");
  }
  if (TYPE != yylex()) {
    J->startError(yylineno);
    J->expecting("type");
    yylval.is_array = false;
    yylval.type = '?';
    yylval.string = 0;
  }
  M->setReturn(yylval);
  types_Off();

  //
  // Done with method prototype.
  // Now read the code block
  // Format is
  //  .code stack INT locals INT
  //
  int line = yylineno;
  if (0==skipUntilToken(dCODE)) {
    J->startError(line);
    J->eofbefore(".code");
    return;
  }
  if (STACK != yylex()) {
    J->startError(yylineno);
    J->expecting("'stack'");
  }
  if (INTEGER != yylex()) {
    J->startError(yylineno);
    J->expecting("integer");
  } else {
    M->setStack(yylval.integer);
  }
  if (LOCALS != yylex()) {
    J->startError(yylineno);
    J->expecting("'locals'");
  }
  if (INTEGER != yylex()) {
    J->startError(yylineno);
    J->expecting("integer");
  } else {
    M->setLocals(yylval.integer);
  }

  //
  // Read instructions until .end code
  //
  for (;;) {
    tk = yylex();
    if ('\n' == tk) continue;
    if (0 == tk) break;
    if (eCODE == tk) break;

    if (dLNT == tk) {
      // .linenumbertable block; skip this
      line = yylineno;
      tk = skipUntilToken(eLNT);
      if (0==tk) {
        J->startError(line);
        J->eofbefore(".end linenumbertable");
        return;
      }
      continue;
    }

    M->addInstruction( buildInstruction(tk, yylval) );
  }

  //
  // Skip anything else until .end method
  //
  line = yylineno;
  if (0==skipUntilToken(eMETHOD)) {
    J->startError(line);
    J->eofbefore(".end method");
  }

  M->doneMethod();
}

dotjfile* parse(const char* fname)
{
  FILE* fin = 0;
  if (0==fname) {
    yyset_in(stdin);
  } else {
    fin = fopen(fname, "r");
    if (0==fin) {
      std::cerr << "Error: couldn't open file " << fname << "\n";
      return 0;
    }
    yyset_in(fin);
  }

  dotjfile* J = new dotjfile(fname);

  int tk;

  //
  // Get class name
  //
  int line = yylineno;
  tk = skipUntilToken(dCLASS);
  if (0==tk) {
    J->startError(line);
    J->eofbefore(".class");
    delete J;
    return 0;
  }
  line = yylineno;
  tk = skipOnLineUntil(WORD);
  if (0==tk) {
    J->startError(line);
    J->eofbefore("class name");
    delete J;
    return 0;
  }
  J->setClassName(yylval.string);

  //
  // Read fields and methods until eof
  //
  for (;;) {
    tk = nextLine();
    if (0==tk) break;

    if (dFIELD == tk) {
      parseField(J);
      continue;
    }

    if (dMETHOD == tk) {
      parseMethod(J);
      continue;
    }
  }
  J->doneFile();

  yyset_in(0);
  fclose(fin);

  return J;
}
