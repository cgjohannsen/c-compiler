#ifndef INSTR_H
#define INSTR_H

#include <iostream>
#include <string>
#include <map>

struct attributes;  // in lexer.h

class expr;
class environment;

class instruction {
    int labelid;
    bool jumped_to;
  public:
    instruction(int labelid = -1);
    virtual ~instruction();

		virtual void activate(bool &on, long line) const;
    virtual void show(std::ostream &s) const = 0;
    virtual bool execute(environment &e) const = 0;

    inline bool isLabel() const { return labelid >= 0; }
    inline int getLabel() const { return labelid; }
		/*
    inline bool isInvisible() const {
      if (labelid < -1) return true;
      if (labelid > 0) return !jumped_to;
      return false;
    }
		*/

    inline void jumpTo(int target) {
      if (target == labelid) jumped_to = true;
    }

    virtual int getJumpTarget() const;
};

inline std::ostream& operator<< (std::ostream &s, const instruction* i)
{
  if (0==i) return s;
  // if (i->isInvisible()) return s;
  if (!i->isLabel()) {
    s << "        ";
  } else {
    s << "      ";
  }
  i->show(s);
  return s << "\n";
}

/*
  Build an appropriate instruction for the current token.
  May use the lexer to read additional info, as needed.

  @param  tid     Token id of instruction
  @param  A       Token attributes

*/
instruction* buildInstruction(int tid, const attributes &A);



/*
  Runtime environment for analyzing instructions.
*/
class environment {
  private:
    struct node {
      expr* data;
      node* next;
    }; // struct environment::node

    node* head;
    unsigned depth;
    unsigned max_depth;
    std::ostream& sout;

    const char* current_method;
    int stack_req;

		char** local_names;

		long restrict_to_line;
		bool active;
  public:
		unsigned* local_versions;
    std::map <std::string, int> versions;

  public:
    environment(std::ostream &s, long line);
    ~environment();

		inline operator bool() const { return active; }

    void startMethod(const char* methname, int maxstack);
    void endMethod(bool print);

		unsigned& version(int index) { return local_versions[index]; }
		std::string varname(int index, char type) const;

    bool annotate(const char* info, long line);

    inline std::ostream& out() const { return sout; }

    inline bool is_empty() const {
      return 0==head;
    }
    inline expr* top() const {
      if (0==head) return 0;
      return head->data;
    }
    expr* pop(const instruction* instr);
    void push(expr* d);
	private:
		void process_variable(const char* which, const char* info);
};

template <class T>
inline std::ostream& operator<< (environment &e, const T &x)
{
  return e.out() << x;
}


#endif
