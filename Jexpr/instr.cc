
#include "instr.h"
#include "lexer.h"
#include "tokens.h"
#include "parser.h"
#include "exprs.h"
#include <sstream>

using namespace std;

// #define DEBUG_PARSER

//
// Helpers
//

void consumeWord(const char* word)
{
  yylex();
  if (0==strcmp(word, yytext)) return;

  // TBD: error message
  fprintf(stderr, "Expected %s got %s\n", word, yytext);
}

char* getWord()
{
  int tk = yylex();
  if (tk != WORD) {
    fprintf(stderr, "Expected word, got token %d\n", tk);
    return 0;
  }
#ifdef DEBUG_PARSER
  std::cerr << "PARSER: got word " << yytext << "\n";
#endif
  return strdup(yytext);
}

const char* skip_spaces(const char* s)
{
	for (unsigned i=0; ; i++) {
		if (' ' == s[i]) continue;
		if ('\t' == s[i]) continue;
		return s+i;
	}
}



//
// instruction methods
//

instruction::instruction(int lid)
{
  labelid = lid;
  jumped_to = false;
}

instruction::~instruction()
{
}

void instruction::activate(bool &on, long line) const
{
	// only annotations change this
}

int instruction::getJumpTarget() const
{
  return -1;
}

//
// Labels.  I know, not really instructions.
//

class label : public instruction {
  public:
    label(const attributes &A) : instruction(A.integer) {
    }
    virtual ~label() { }

    virtual void show(ostream &s) const {
      s << "L" << getLabel() << ":";
    }

    virtual bool execute(environment &e) const {
			return false;
    }
};

//
// Unknown instructions
//

class unknown : public instruction {
    string I;
  public:
    unknown(const string &i) : I(i) {
    }
    virtual ~unknown()              { }

    virtual void show(ostream &s) const {
      s << I << " ; ";
    }
    virtual bool execute(environment &e) const {
			return false;
    }
};

//
// Load instructions
//

class load : public instruction {
    int index;
    char type;
    bool is_array;
  public:
    load(const attributes &A) {
      index = A.integer;
      type = A.type;
      is_array = A.is_array;
    }
    virtual ~load() { }

    virtual void show(ostream &s) const {
      if (is_array) {
        s << type << "aload";
      } else {
        s << type << "load";
        if (index < 4)  s << "_" << index;
        else            s << " " << index;
      }
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      if (is_array) {
        expr* indx = e.pop(this);
        expr* arr = e.pop(this);
        e.push( new arrayind(type, arr, indx, e.versions) );
      } else {
        e.push( new local(e, type, index) );
      }
			return true;
    }
};

//
// Store instructions
//

class store : public instruction {
    int index;
    char type;
    bool is_array;
  public:
    store(const attributes &A) {
      index = A.integer;
      type = A.type;
      is_array = A.is_array;
    }
    virtual ~store() { }

    virtual void show(ostream &s) const {
      if (is_array) {
        s << type << "astore";
      } else {
        s << type << "store";
        if (index < 4)  s << "_" << index;
        else            s << " " << index;
      }
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      if (is_array) {
        expr* val =  e.pop(this);
        expr* indx = e.pop(this);
        expr* arr = e.pop(this);
        arrayind::assign(e.out(), type, arr, indx, e.versions, val);
      } else {
        local::assign(e, type, index, e.pop(this));
      }
			return true;
    }
};


//
// Const instructions
//

class const_ : public instruction {
    int value;
    char type;
  public:
    const_(const attributes &A) {
      value = A.integer;
      type = A.type;
    }
    virtual ~const_() { }

    virtual void show(ostream &s) const {
      s << type << "const_";
      if (value < 0)  s << "m1";
      else            s << value;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      if ('i' == type)  e.push( new iconst(value) );
      else              e.push( new fconst(value) );
			return true;
    }
};


//
// binary instructions
//

class binary : public instruction {
    char optor;
    char type;
  public:
    binary(const attributes &A) {
      optor = A.integer;
      type = A.type;
    }
    virtual ~binary() { }

    virtual void show(ostream &s) const {
      s << type;
      switch (optor) {
        case '+'  : s << "add";   return;
        case '-'  : s << "sub";   return;
        case '*'  : s << "mul";   return;
        case '/'  : s << "div";   return;
        case '%'  : s << "rem";   return;
        case '&'  : s << "and";   return;
        case '|'  : s << "or";    return;
        case '^'  : s << "xor";   return;
        default   : s << "???";   return;
      }
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      expr* R = e.pop(this);
      expr* L = e.pop(this);
      e.push( new binary_expr(L, type, optor, R) );
			return true;
    }
};

//
// unary instructions
//

class unary : public instruction {
    char optor;
    char type;
  public:
    unary(const attributes &A) {
      optor = A.integer;
      type = A.type;
    }
    virtual ~unary() { }

    virtual void show(ostream &s) const {
      s << type;
      switch (optor) {
        case '-'  : s << "neg";   return;
        default   : s << "???";   return;
      }
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      e.push( new unary_expr(type, optor, e.pop(this)) );
			return true;
    }
};

//
// type conversions
//

class conversion : public instruction {
    char oldtype;
    char newtype;
  public:
    conversion(const attributes &A) {
      oldtype = A.type;
      newtype = A.integer;
    }
    virtual ~conversion() { }

    virtual void show(ostream &s) const {
      s << oldtype << "2" << newtype;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      e.push( new convert(oldtype, newtype, e.pop(this)) );
			return true;
    }
};

//
// push instructions
//

class push : public instruction {
    int value;
  public:
    push(const attributes &A) {
      int tk = yylex();
      if (tk != INTEGER) {
        value = 0;
        // TBD - error message?
      } else {
        value = yylval.integer;
      }
    }
    virtual ~push() { }

    virtual void show(ostream &s) const {
      if ((value >= -128) && (value <= 127))
        s << "bipush " << value;
      else
        s << "sipush " << value;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      e.push( new iconst(value) );
			return true;
    }
};

//
// Dup instructions
//

class dup_inst : public instruction {
    int amt;
  public:
    dup_inst(const attributes &A) {
      amt = A.integer;
    }
    virtual ~dup_inst() { }

    virtual void show(ostream &s) const {
      s << "dup";
      if (amt) s << "_x" << amt;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      assert(amt >= 0);
      assert(amt <= 2);
      expr* val[3];
      for (int i=0; i<=amt; i++) {
        val[i] = e.pop(this);
      }
      e.push(val[0] ? val[0]->duplicate() : 0);
      for (int i=amt; i>=0; i--) {
        e.push(val[i] ? val[i]->duplicate() : 0);
      }
			return true;
    }
};

//
// Swap instruction
//

class swap_instr : public instruction {
  public:
    swap_instr() {
    }
    virtual ~swap_instr() { }

    virtual void show(ostream &s) const {
      s << "swap";
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      expr* first = e.pop(this);
      e.push( e.pop(this) );
      e.push(first);
			return true;
    }
};

//
// Pop instruction
//

class pop : public instruction {
  public:
    pop() {
    }
    virtual ~pop() { }

    virtual void show(ostream &s) const {
      s << "pop";
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      expr* item = e.pop(this);
      e << "    pop " << item << "\n";
      delete item;
			return true;
    }
};

//
// iinc instruction
//

class iinc_instr : public instruction {
    int index;
    int delta;
  public:
    iinc_instr() {
      int tk = yylex();
      if (tk != INTEGER) {
        index = 0;
        // TBD - error message?
      } else {
        index = yylval.integer;
      }
      tk = yylex();
      if (tk != INTEGER) {
        delta = 0;
        // TBD - error message?
      } else {
        delta = yylval.integer;
      }
    }
    virtual ~iinc_instr() { }

    virtual void show(ostream &s) const {
      s << "iinc " << index << " " << delta;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      local::increment(e, 'i', index, delta);
			return true;
    }
};

//
// Return instructions
//

class ret_instr : public instruction {
    char type;
  public:
    ret_instr(const attributes &A) {
      type = A.type;
    }
    virtual ~ret_instr() { }

    virtual void show(ostream &s) const {
      if (' ' != type) s << type;
      s << "return";
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      if (' ' == type) return false;
      expr* item = e.pop(this);
      e << "    " << type << "return " << item << "\n";
      delete item;
			return true;
    }
};

//
// ldc
//

class ldc_instr : public instruction {
    char* item;
  public:
    ldc_instr() {
      int tk = yylex();
      if ((INTEGER == tk) || (FLOAT == tk) || (STRING == tk)) {
        item = strdup(yytext);
      } else {
        // TBD: error
        item = 0;
      }
    }

    virtual ~ldc_instr() {
      free(item);
    }

    virtual void show(ostream &s) const {
      s << "ldc " << ( item ? item : "(null)" );
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      e.push( new ldcconst(item) );
			return true;
    }
};

//
// getputstatic
//

class getputstatic : public instruction {
    char* classname;
    char* fieldname;
    typeinfo Type;
    bool getting;
  public:
    getputstatic(int tid) {
      getting = (GETSTATIC == tid);
      consumeWord("Field");
      classname = getWord();
      fieldname = getWord();
      types_On();
      int tk = yylex();
      if (tk != TYPE) {
        // TBD error
      }
      Type.set(yylval);
      types_Off();
    }

    virtual ~getputstatic() {
      free(classname);
      free(fieldname);
    }

    virtual void show(ostream &s) const {
      if (getting)  s << "get";
      else          s << "put";
      s << "static Field " << classname << " " << fieldname << " " << Type;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      if (getting) {
        e.push( new global(classname, fieldname, Type, e.versions) );
      } else {
        expr* rhs = e.pop(this);
        global::assign(e.out(), classname, fieldname, Type, e.versions, rhs);
      }
			return true;
    }
};

//
// invoke
//

class invoke : public instruction {
    char* classname;
    char* methodname;

    int kind;  // token id
    typeinfo Types[32]; // 0 is return type; others are param types
    unsigned num_params;
  public:
    invoke(int k) {
      kind = k;
      consumeWord("Method");
      classname = getWord();
      methodname = getWord();
      types_On();
      int tk = yylex();
      if (tk != '(') fprintf(stderr, "missing (\n");
      num_params = 0;
      for (;;) {
        tk = yylex();
        if (')' == tk) break;
        if (TYPE != tk) {
          fprintf(stderr, "missing )\n");
          break;
        }
        if (num_params >= 31) {
          fprintf(stderr, "Internal error: too many params on method call\n");
          break;
        }
        Types[++num_params].set(yylval);
      }
      tk = yylex();
      if (TYPE != tk) {
        fprintf(stderr, "missing return type\n");
      }
      Types[0].set(yylval);
      types_Off();
    }

    virtual ~invoke() {
      free(classname);
      free(methodname);
    }

    virtual void show(ostream &s) const {
      s << "invoke";
      if (INVOKEVIRTUAL == kind) s << "virtual";
      if (INVOKESPECIAL == kind) s << "special";
      if (INVOKESTATIC  == kind) s << "static";
      s << " Method " << classname << " " << methodname << " (";
      for (unsigned i=1; i<=num_params; i++) s << Types[i];
      s << ")" << Types[0];
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      methcall* mc = new methcall(classname, methodname, Types, num_params);
      unsigned i = num_params;
      while (i) {
        i--;
        mc->setParam(i, e.pop(this));
      }
      if (INVOKESTATIC != kind) {
        mc->setObject(e.pop(this));
      }
      e << "    ";
      mc->showcall(e.out());
      e << '\n';
      if (Types[0].equals('V')) {
        delete mc;
      } else {
        e.push(mc);
      }
			return true;
    }
};

//
// our annotations
//

class annotate : public instruction {
		long linenum;
    char* info;
  public:
    annotate(const char* text) : instruction(-2) {
      info = strdup(text);
			linenum = 0;
			// grab line number
			unsigned len = strlen(text);
			unsigned units = 1;
			while (len) {
				--len;
        if (1==units && ' ' == text[len]) continue;
        if (1==units && '\t' == text[len]) continue;
        if (1==units && '\r' == text[len]) continue;
				if (text[len] < '0') break;
				if (text[len] > '9') break;
				linenum += units * (text[len]-'0');
				units *= 10;
			}
    }
    virtual ~annotate() {
      free(info);
    }

		virtual void activate(bool &on, long line) const
		{
			if (line>0) on = (linenum == line);
		}


    virtual void show(ostream &s) const {
			s << info;
    }

    virtual bool execute(environment &e) const {
      return e.annotate(info, linenum);
    }
};

//
// array builder
//

class newarray : public instruction {
    char* type;
  public:
    newarray() {
      int tk = yylex();
      if (WORD != tk) {
        // TBD error
        type = 0;
      } else {
        type = strdup(yytext);
      }
    }
    virtual ~newarray() {
      free(type);
    }
    virtual void show(ostream &s) const {
      s << "newarray " << type;
    }

    virtual bool execute(environment &e) const {
			if (!e) return false;
      e.push( new newarray_expr(type, e.pop(this)) );
			return true;
    }
};

//
// unconditional goto
//

class plain_goto : public instruction {
    int target;
  public:
    plain_goto() {
      int tk = yylex();
      if (tk != LABEL) {
        target = -1;
        // TBD - error message?
      } else {
        target = yylval.integer;
      }
    }

    virtual void show(ostream &s) const {
      s << "goto L" << target;
    }

    virtual bool execute(environment &e) const {
			return false;
    }
};

//
// Front end
//

instruction* buildInstruction(int tid, const attributes &A)
{
  // Sanity checks
  if ('\n' == tid) return 0;
  if (0 == tid) return 0;

  switch (tid) {
    case LABEL:   return new label(A);

    case LOAD:    return new load(A);
    case STORE:   return new store(A);
    case CONST:   return new const_(A);
    case BINARY:  return new binary(A);
    case UNARY:   return new unary(A);
    case CONV:    return new conversion(A);
    case PUSH:    return new push(A);
    case DUP:     return new dup_inst(A);
    case SWAP:    return new swap_instr();
    case POP:     return new pop();
    case IINC:    return new iinc_instr();
    case RETURN:  return new ret_instr(A);

    case LDC:       return new ldc_instr();

    case GETSTATIC:
    case PUTSTATIC: return new getputstatic(tid);

    case INVOKESPECIAL:
    case INVOKESTATIC:
    case INVOKEVIRTUAL: return new invoke(tid);

    case NOTE:    return new annotate(yylval.string);

    case NEWARRAY:  return new newarray();

    case GOTO:    return new plain_goto();

    default:      break;  // in case any compilers complain
  }

  // Generic catch-all
  stringstream s;
  s << yytext;
  for (;;) {
    tid = yylex();
    if ('\n' == tid) break;
    if (0 == tid) break;
    s << ' ' << yytext;
  }
  return new unknown(s.str());
}



//
// Environment methods
//

environment::environment(std::ostream &s, long line) : sout(s)
{
  current_method = 0;
  head = 0;
  depth = 0;
  max_depth = 0;

	local_names = new char*[256];
	local_versions = new unsigned[256];
	for (unsigned i=0; i<256; i++) {
		local_names[i] = 0;
		local_versions[i] = 0;
	}

	restrict_to_line = line;
	active = (line<0);
}

environment::~environment()
{
  for (expr* d = pop(0); d; d = pop(0)) delete d;
}

void environment::startMethod(const char* methname, int maxstack)
{
  current_method = methname;
  stack_req = maxstack;
  versions.clear();
	for (unsigned i=0; i<256; i++) {
		free(local_names[i]);
		local_names[i] = 0;
		local_versions[i] = 0;
	}
}

void environment::endMethod(bool print)
{
  // Display any remaining stack entries
	if (print) {
		sout << "    ;\n";
		if (restrict_to_line<0) {
			if (stack_req < 0) {
				sout << "    ; Stack size not specified\n";
			} else {
				sout << "    ; Requested stack size " << stack_req << "\n";
			}
		}
		sout << "    ; Required  stack size " << max_depth << "\n";
		sout << "    ; " << depth << " items remaining on the stack\n";
		while (depth) {
			expr* item = pop(0);
			sout << "    ;STACK: " << item << "\n";
		}
		sout << "    ;\n";
	}

  current_method = 0;
  max_depth = 0;

	if (restrict_to_line>0) {
		active = false;
	}
}

std::string environment::varname(int index, char type) const
{
	std::stringstream s;
	if (index < 256 && local_names[index]) {
		s << local_names[index];
	} else {
		s << "{local " << index << "}";
	}
	if (type) s << "." << type;
	return s.str();
}

bool environment::annotate(const char* info, long line)
{
	if (restrict_to_line > 0) {
		active = (line == restrict_to_line);
	}
	const char* strip = skip_spaces(info+2);
	if (0==strip) return false;
	if ('l' == strip[0]) {
		process_variable("local", strip);
	}
	if ('p' == strip[0]) {
		process_variable("parameter", strip);
	}
	if (active) {
		sout << "    " << info << "\n";
		return true;
	}
	return false;
}

expr* environment::pop(const instruction* instr)
{
  if (0==head) {
    if (instr) std::cerr << "\tstack underflow on " << instr << "\n";
    return 0;
  }
  depth--;
  node* curr = head;
  head = head->next;
  expr* d = curr->data;
  delete curr;
  return d;
}

void environment::push(expr* d)
{
  node* curr = new node;
  curr->data = d;
  curr->next = head;
  head = curr;
  depth++;
  if (depth > max_depth) max_depth = depth;
}

void environment::process_variable(const char* which, const char* note)
{
	unsigned i;
	for (i=0; which[i]; ++i) {
		if (which[i] != *note) return;
		++note;
	}

	note = skip_spaces(note);
	unsigned slot = 0;
	for (; *note; ++note) {
		if (' ' == *note) break;
		if ((*note >= '0') && (*note <= '9')) {
			slot = 10*slot + *note - '0';
			continue;
		}
		return;
	}

	note = skip_spaces(note);

	static char buffer[256];
	buffer[255] = 0;
	for (unsigned i=0; i<254; ++i) {
		if ((' ' == note[i]) ||
				('\t' == note[i]) ||
				('\r' == note[i]) ||
				('\n' == note[i]) ||
				(';' == note[i]) ||
				(0 == note[i]))
		{
			buffer[i] = 0;
			break;
		}
		buffer[i] = note[i];
	}

	local_names[slot] = strdup(buffer);
	local_versions[slot] = 0;
}

// TBD


