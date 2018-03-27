all: comp.c func.lex parse.c tokens.h nodes.h
	flex func.lex
	gcc -g -c lex.yy.c
	gcc -g -c parse.c
	gcc -g -c print.c
	gcc -g -c comp.c
	gcc -g -o comp print.o lex.yy.o parse.o comp.o

out: comp.c func.lex parse.c tokens.h nodes.h
	flex func.lex
	gcc -c lex.yy.c -DOUTPUT
	gcc -c parse.c -DOUTPUT
	gcc -c print.c -DOUTPUT
	gcc -c comp.c -DOUTPUT
	gcc -o out print.o lex.yy.o parse.o comp.o


clean:
	rm -rf *.o lex.yy.c parser
