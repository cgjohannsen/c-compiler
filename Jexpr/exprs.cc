
#include "exprs.h"

#include <sstream>
#include <assert.h>

#include "instr.h"

using namespace std;

inline ostream& primes(ostream& s, int num)
{
  for (; num; num--) s << '`';
  return s;
}

// **********************************************************************
// *
// * expr methods
// *
// **********************************************************************

bool expr::show_types;
bool expr::show_version;

expr::expr()
{
}

expr::~expr()
{
}

// **********************************************************************
// *
// * iconst methods
// *
// **********************************************************************

iconst::iconst(int v)
{
  value = v;
}

iconst::~iconst()
{
}

void iconst::show(ostream &s, char, char) const
{
  s << value;
}

expr* iconst::duplicate() const
{
  return new iconst(value);
}

// **********************************************************************
// *
// * fconst methods
// *
// **********************************************************************

fconst::fconst(float v)
{
  value = v;
}

fconst::~fconst()
{
}

void fconst::show(ostream &s, char, char) const
{
  s << value;
}

expr* fconst::duplicate() const
{
  return new fconst(value);
}

// **********************************************************************
// *
// * ldcconst methods
// *
// **********************************************************************

ldcconst::ldcconst(const char* v)
{
  value = v;
}

ldcconst::~ldcconst()
{
}

void ldcconst::show(ostream &s, char, char) const
{
  s << value;
}

expr* ldcconst::duplicate() const
{
  return new ldcconst(value);
}


// **********************************************************************
// *
// * local methods
// *
// **********************************************************************

local::local(const environment &e, char t, int i)
	: E(e)
{
  type = show_types ? t : 0;
  index = i;
	updates = show_version ? E.local_versions[i] : 0;
}

local::~local()
{
}

void local::show(ostream &s, char, char) const
{
  s << E.varname(index, type);
  primes(s, updates);
}

expr* local::duplicate() const
{
	local* copy = new local(E, type, index);
	copy->updates = updates;
	return copy;
}

void local::assign(environment &E, char type, int index, expr* rhs)
{
  string V = E.varname(index, show_types ? type : 0);
  E.out() << "    " << V;
  if (show_version) ++E.local_versions[index];
  primes(E.out(), E.local_versions[index]) << " = " << rhs << "\n";
  delete rhs;
}

void local::increment(environment &E, char type, int index, int delta)
{
  string V = E.varname(index, show_types ? type : 0);
  E.out() << "    " << V;
  primes(E.out(), E.local_versions[index]+ (show_version ? 1 : 0)) 
		<< " = " << V;
  primes(E.out(), E.local_versions[index]);
  if (delta > 0) {
    E.out() << " + " << delta << "\n";
  } else {
    E.out() << " - " << -delta << "\n";
  }
  if (show_version) ++E.local_versions[index];
}

// **********************************************************************
// *
// * global methods
// *
// **********************************************************************

global::global(const char* cn, const char* fn, const typeinfo &t, map<string, int> &M)
{
  classname = cn;
  fieldname = fn;
  type.set(t);
  updates = M[varname(classname, fieldname, type)];
}

global::global(const char* cn, const char* fn, const typeinfo &t, int u)
{
  updates = u;
  classname = cn;
  fieldname = fn;
  type.set(t);
}

global::~global()
{
}

string global::varname(const char* cn, const char* fn, const typeinfo &T)
{
  stringstream s;
  s << cn << "." << fn;
  if (show_types) s << "." << T;
  return s.str();
}


void global::show(std::ostream &s, char, char) const
{
  s << varname(classname, fieldname, type);
  primes(s, updates);
}

expr* global::duplicate() const
{
  return new global(classname, fieldname, type, updates);
}

void global::assign(std::ostream &s, const char* cn, const char* fn, const typeinfo &T, map<string, int> &M, expr* rhs)
{
  string V = varname(cn, fn, T);
  s << "    " << V;
  if (show_version) ++M[V];
  primes(s, M[V]) << " = " << rhs << "\n";
  delete rhs;
}

// **********************************************************************
// *
// * arrayind methods
// *
// **********************************************************************

arrayind::arrayind(char t, expr* A, expr* i, int u)
{
  type = t;
  array = A;
  index = i;
  updates = u;
}

arrayind::arrayind(char t, expr* A, expr* i, map<string, int> &M)
{
  type = t;
  array = A;
  index = i;
  updates = M[varname(t, array, index)];
}


arrayind::~arrayind()
{
  delete array;
  delete index;
}

void arrayind::show(ostream &s, char, char) const
{
  show(s, type, array, index, updates);
}

expr* arrayind::duplicate() const
{
  return new arrayind(
    type, 
    array ? array->duplicate() : 0,
    index ? index->duplicate() : 0,
    updates
  );
}

string arrayind::varname(char type, const expr* array, const expr* index)
{
  stringstream s;
  s << array << "[" << index << "]";
  if (show_types) s << "." << type;
  return s.str();
}

void arrayind::show(ostream &s, char type, const expr* array,
  const expr* index, int updates)
{
  s << varname(type, array, index);
  primes(s, updates);
}

void arrayind::assign(ostream &s, char type, const expr* array,
  const expr* index, map<string,int> &M, expr* rhs)
{
  string V = varname(type, array, index);
  s << "    " << V;
  if (show_version) ++M[V];
  primes(s, M[V]) << " = " << rhs << "\n";
  delete rhs;
}


// **********************************************************************
// *
// * convert methods
// *
// **********************************************************************

convert::convert(char ot, char nt, expr* x)
{
  oldtype = ot;
  newtype = nt;
  opnd = x;
}

convert::~convert()
{
  delete opnd;
}

void convert::show(std::ostream &s, char, char) const
{
  s << oldtype << "2" << newtype << "(" << opnd << ")";
}

expr* convert::duplicate() const
{
  return new convert(oldtype, newtype, opnd ? opnd->duplicate() : 0);
}

// **********************************************************************
// *
// * unary_expr methods
// *
// **********************************************************************

unary_expr::unary_expr(char T, char op, expr* _opnd)
{
  type = T;
  optor = op;
  opnd = _opnd;
}

unary_expr::~unary_expr()
{
  delete opnd;
}

void unary_expr::show(ostream &s, char left, char right) const
{
  s << optor;
  if (show_types) s << type;
  s << ' ';
  if (opnd) opnd->show(s, optor, right);
  else      s << "null";
}

expr* unary_expr::duplicate() const
{
  return new unary_expr(type, optor, opnd ? opnd->duplicate() : 0);
}

// **********************************************************************
// *
// * binary_expr methods
// *
// **********************************************************************

binary_expr::binary_expr(expr* L, char T, char op, expr* R)
{
  type = T;
  optor = op;
  left = L;
  right = R;
}

binary_expr::~binary_expr()
{
  delete left;
  delete right;
}

void binary_expr::show(ostream &s, char lop, char rop) const
{
  // Should we add parens?
  bool parens = false;
  switch (optor) {
    case '-': 
    case '+': parens = 
                  ((' ' != lop) && ('+' != lop)) 
                  ||
                  ((' ' != rop) && ('+' != rop) && ('-' != rop));
              break;

    case '*': parens = 
                  (('/' == lop) || ('%' == lop));
              break;

    case '/':
    case '%': parens =
                  (('/' == lop) || ('%' == lop) 
                    || ('*' == rop) || ('/' == rop) || ('%' == rop));
              break;
  }

  if (parens) s << "(";

  if (left) left->show(s, lop, optor);
  else      s << "null";
  s << ' ' << optor;
  if (show_types) s << type;
  s << ' ';
  if (right) right->show(s, optor, rop);
  else      s << "null";

  if (parens) s << ")";
}

expr* binary_expr::duplicate() const
{
  return new binary_expr(
    left ? left->duplicate() : 0, 
    type, 
    optor, 
    right ? right->duplicate() : 0
  );
}


// **********************************************************************
// *
// * methcall methods
// *
// **********************************************************************

methcall::methcall(const char* cn, const char* mn, const typeinfo* T, unsigned np)
{
  classname = cn;
  methodname = mn;
  Types = T;
  num_params = np;
  if (num_params) {
    params = new expr* [num_params];
    for (unsigned i=0; i<num_params; i++) {
      params[i] = 0;
    }
  } else {
    params = 0;
  }
  object = 0;
  is_static = true;
}

methcall::~methcall()
{
  for (unsigned i=0; i<num_params; i++) {
    delete params[i];
  }
  delete[] params;
}

void methcall::showcall(std::ostream &s) const
{
  if (show_types) {
    s << "calling " << classname << "." << methodname << " : (";
    for (unsigned i=1; i<=num_params; i++) {
      s << Types[i];
    }
    s << ")" << Types[0];
    if (!is_static) {
      s << " on object " << object;
    }
    if (num_params) {
      s << " with parameters ";
      for (unsigned i=0; i<num_params; i++) {
        if (i) s << ", ";
        s << params[i];
      }
    } else {
      s << " with no parameters";
    }
  } else {
    s << "calling ";
    show(s, ' ', ' ');
  }
}

void methcall::show(std::ostream &s, char, char) const
{
  if (is_static) {
    s << classname << "::" << methodname << "(";
  } else {
    s << object << "." << methodname << "(";
  }
  for (unsigned i=0; i<num_params; i++) {
    if (i) s << ", ";
    s << params[i];
  }
  s << ")";
}

expr* methcall::duplicate() const
{
  methcall* mc = new methcall(classname, methodname, Types, num_params);
  for (unsigned i=0; i<num_params; i++) {
    mc->setParam(i, params[i] ? params[i]->duplicate() : 0 ); 
  }
  return mc;
}

// **********************************************************************
// *
// * newarray_expr methods
// *
// **********************************************************************

newarray_expr::newarray_expr(const char* t, expr* sz)
{
  type = t;
  size = sz;
}

newarray_expr::~newarray_expr()
{
  delete size;
}

void newarray_expr::show(ostream &s, char, char) const
{
  s << "new " << type << "[" << size << "]";
}

expr* newarray_expr::duplicate() const
{
  return new newarray_expr(type, size ? size->duplicate() : 0);
}


