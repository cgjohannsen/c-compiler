all: jexpr

FLAGS= -Wall -ggdb
# FLAGS= -Wall

GEN= tokens.cc tokens.h jexpr

OBJS= jexpr.o tokens.o parser.o lexer.o instr.o exprs.o

SOURCES= jexpr.cc parser.cc lexer.cc instr.cc exprs.cc \
         tokens.l parser.h  lexer.h  instr.h  exprs.h

clean:
	rm $(OBJS) $(GEN)

depend:
	makedepend -DSKIP_SYSTEM_INCLUDES $(SOURCES)


# TUC

jexpr: $(OBJS)
	g++ -o jexpr $(OBJS)

%.o: %.cc
	g++ $(FLAGS) -c $<

tokens.cc tokens.h: tokens.l
	flex -o tokens.cc --header-file=tokens.h tokens.l

# DO NOT DELETE THIS LINE -- make depend needs it

jexpr.o: parser.h instr.h exprs.h
parser.o: parser.h instr.h lexer.h tokens.h
lexer.o: lexer.h
instr.o: instr.h lexer.h tokens.h parser.h exprs.h
exprs.o: exprs.h parser.h instr.h
tokens.o: lexer.h
parser.o: instr.h
exprs.o: parser.h instr.h
