
#ifndef EXPRS_H
#define EXPRS_H

#include <iostream>
#include <map>
#include <cassert>
#include "parser.h"

class environment;

class expr {
  public:
    expr();
    virtual ~expr();

    /**
      left, right are operators appearing to the left/right of the expression.
        -,+,*,/,% : binary ops
        -         : unary minus
        space     : none
    */
    virtual void show(std::ostream &s, char left, char right) const = 0;

    virtual expr* duplicate() const = 0;

    inline static void setShowTypes(bool st) { show_types = st; }
    inline static void setShowVersion(bool sv) { show_version = sv; }
  protected:
    static bool show_types;
    static bool show_version;
};

inline std::ostream& operator<<(std::ostream& out, const expr* E)
{
  if (E)  E->show(out, ' ', ' ');
  else    out << "(null)";
  return out;
}

class iconst : public expr {
    int value;
  public:
    iconst(int v);
    virtual ~iconst();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

class fconst : public expr {
    float value;
  public:
    fconst(float v);
    virtual ~fconst();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

class ldcconst : public expr {
    const char* value;
  public:
    ldcconst(const char* v);
    virtual ~ldcconst();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

class local : public expr {
    char type;
    int index;
    int updates;
		const environment &E;
  public:
    local(const environment &e, char t, int index);
    virtual ~local();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;

    static void assign(environment &E, char type, int index, expr* rhs);
    static void increment(environment &E, char type, int index, int delta);
};

class global : public expr {
    const char* classname;
    const char* fieldname;
    typeinfo type;
    int updates;
  public:
    global(const char* cn, const char* fn, const typeinfo &T, std::map<std::string, int> &M);
    virtual ~global();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;

    static void assign(std::ostream &s, const char* cn, const char* fn, const typeinfo &T, std::map<std::string, int> &M, expr* rhs);

  private:
    static std::string varname(const char* cn, const char* fn, const typeinfo &t);
    global(const char* cn, const char* fn, const typeinfo &T, int u);
};

class arrayind : public expr {
    char type;
    expr* array;
    expr* index;
    int updates;
  public:
    arrayind(char t, expr* A, expr* i, int u);
    arrayind(char t, expr* A, expr* i, std::map<std::string, int> &M);
    virtual ~arrayind();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;

    static std::string varname(char type, const expr* array, const expr* index);
    static void assign(std::ostream &s, char type, const expr* array,
      const expr* index, std::map<std::string, int> &M, expr* rhs);
	private:
    static void show(std::ostream &s, char type, const expr* array,
      const expr* index, int updates);
};


class convert : public expr {
    expr* opnd;
    char oldtype;
    char newtype;
  public:
    convert(char ot, char nt, expr* x);
    virtual ~convert();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

class unary_expr : public expr {
    expr* opnd;
    char type;    // i or f
    char optor;   // operator
  public:
    unary_expr(char T, char op, expr* opnd);
    virtual ~unary_expr();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};


class binary_expr : public expr {
    expr* left;
    expr* right;
    char type;
    char optor;
  public:
    binary_expr(expr* L, char T, char op, expr* R);
    virtual ~binary_expr();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};


class methcall : public expr {
    const char* classname;
    const char* methodname;
    const typeinfo* Types;
    expr* object;
    expr** params;
    unsigned num_params;
    bool is_static;
  public:
    methcall(const char* cn, const char* mn, const typeinfo* T, unsigned np);
    virtual ~methcall();

    inline void setParam(unsigned i, expr* pass) {
      assert(i<num_params);
      assert(0==params[i]);
      params[i] = pass;
    }
    
    inline void setObject(expr* obj) {
      assert(0==object);
      object = obj;
      is_static = false;
    }

    void showcall(std::ostream &s) const;
    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

class newarray_expr : public expr {
    const char* type;
    expr* size;
  public:
    newarray_expr(const char* t, expr* sz);
    virtual ~newarray_expr();

    virtual void show(std::ostream &s, char, char) const;
    virtual expr* duplicate() const;
};

#endif

