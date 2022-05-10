
#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "instr.h"

struct attributes;

class typeinfo {
    bool is_array;
    char letter;
    char* objname;
  public:
    typeinfo();
    ~typeinfo();
    void set(const attributes &A);
    void set(const typeinfo &T);
    void show(std::ostream &o) const;
    bool equals(const typeinfo &t) const;
    bool equals(char ltr) const;
};

inline std::ostream& operator<<(std::ostream &s, const typeinfo &T)
{
  T.show(s);
  return s;
}

class variable {
    typeinfo Type;
    char* name;
  public:
    variable(const attributes &A, char* n);
    ~variable();

    void show(std::ostream &o) const;
    bool equals(const variable* v) const;

    void writeDotj(std::ostream &s) const;
};

class method {
    char* name;
    int stack;
    int locals;
    typeinfo* Types;    // 0 is return type; the rest are parameters
    unsigned num_params;

    instruction** code;
    unsigned codelen;
  public:
    method(const char* n);
    ~method();

    const char* getName() const { return name; }

    void setReturn(const attributes &A);
    void addParam(const attributes &A);

    void setStack(int s);
    void setLocals(int s);

    void addInstruction(instruction* I);
    void doneMethod();

    void writeDotj(std::ostream &s, bool with_labels, long line) const;
    void writeExprs(environment &e, long line) const;
};

class dotjfile {
    const char* filename;
    char* classname;

    variable** globals;
    unsigned num_globals;

    method** methods;
    unsigned num_methods;
  public:
    dotjfile(const char* fn);
    ~dotjfile();

    const char* getFileName() const { return filename; }

    void setClassName(const char* cn);
    const char* getClassName() const { return classname; }

    void addGlobal(const attributes &A, char* varname);
    void addMethod(method* M);

    void doneFile();

  // Error messages

    void startError(int line=-1) const;
    void expecting(const char* what) const;
    void eofbefore(const char* what) const;

  // Front end

    void writeDotj(std::ostream &s, const char* meth, long line, bool with_labels) const;
    void writeExprs(std::ostream &s, const char* meth, long line) const;
};

dotjfile* parse(const char* fname);

#endif
