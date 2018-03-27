#include <stdio.h>
#include <stdlib.h>
#include "nodes.h"
#include "tokens.h"
#include <string.h>

int symb;
int func_args;
extern printSymb();
extern char* showSymb(int);
extern int yylex(void);
extern FILE* yyin;
extern char * yytext;

lex(){
	symb = yylex();
}

NODE * newName(char * i)
{  NODE * n;
   char * cur = strdup(i);
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = NAME;
   n->f.id = cur;
   return n;
}

NODE * newNum(char * i)
{  NODE * n;
   char * cur = strdup(i);
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = NUMBER;
   n->f.value = atoi(cur);
   return n;
}

NODE * newNode(int tag)
{  NODE * n;
   n = (NODE *)malloc(sizeof(NODE));
   n->tag = tag;
   n->f.b.n0 = NULL;
   n->f.b.n1 = NULL;
   n->f.b.n2 = NULL;
   n->f.b.n3 = NULL;
   n->f.b.n4 = NULL;
   return n;
}

showTree(NODE * tree,int depth)
{  int i;
   if(tree==NULL)
    return;
   for(i=0;i<depth;i++)
    putchar('-');
   switch(tree->tag)
   {  case NAME: printf("%s\n",tree->f.id);
               return;
      case NUMBER: printf("%d\n",tree->f.value);
               return;
      default: printf("%s\n",showSymb(tree->tag));
			   showTree(tree->f.b.n0,depth+1);
               showTree(tree->f.b.n1,depth+1);
               showTree(tree->f.b.n2,depth+1);
			   showTree(tree->f.b.n3,depth+1);
               showTree(tree->f.b.n4,depth+1);
   }
}

error(char * msg){
	printf("ERROR: ");
	printf(msg);
	printf("\n");
	exit(0);
}

NODE * condexpr(){
	extern NODE * args();
	NODE * n = newNode(symb);
	switch(symb){
		case LT:
		case LTE:
		case EQ:
		case NEQ:	printSymb();
				lex();
				if(symb==LBRA){
					printSymb();
					lex();
					n->f.b.n1 = args();
				}
				if(symb!=RBRA)
					error("expected )");
				printSymb();
				break;
		default: error("expected Lt/LtE/Eq/NEq");
	}
	return n;
}

NODE * op(){
	extern NODE * args();
	NODE * n = newNode(symb);
	printSymb();
	lex();
	if(symb!=LBRA)
		error("expected (");
	printSymb();
	lex();
	n->f.b.n1 = args();
	if(symb!=RBRA)
		error("expected )");
	printSymb();
	lex();
	return n;
}


NODE * expr(){
	extern NODE * args();
	extern NODE * expr();
	extern NODE * op();
	NODE * n = newNode(SEMI);
	switch(symb){
		case MINUS: n->f.b.n1 = op();
				break;
		case PLUS:	n->f.b.n1 = op();
				break;
		case MULT:	n->f.b.n1 = op();
				break;
		case DIV:	n->f.b.n1 = op();
				break;
		case NUMBER: n->f.b.n1 = newNum(yytext);
				printSymb();
				lex();
				break;
		case NAME:	break;
		default: error("expected a name/number/operator");
	}
	if(symb==NAME){
		n->f.b.n1 = newName(yytext);
		printSymb();
		lex();
		if(symb==LBRA){
			printSymb();
			lex();
			n->f.b.n1->f.b.n1 = args();
			if(symb!=RBRA)
				error("expected )");
			printSymb();
			lex();
		}
	}
	if(symb == COMMA){
		printSymb();
		lex();
		n->f.b.n3 = expr();
	}
	if(symb != SEMI)
		error("expected ;");
	printSymb();
	lex();
	return n;
}

NODE * command_if(){
	extern NODE * condexpr();
	extern NODE * commands();
	NODE * n = newNode(IF);
	printSymb();
	lex();
	n->f.b.n1 = condexpr();
	lex();
	if(symb != THEN)
		error("expecting then");
	printSymb();
	lex();
	n->f.b.n2 = commands();
	if(symb == ELSE){
		printSymb();
		lex();
		n->f.b.n3 = commands();
	}
	if(symb != END)
		error("expecting end");
	printSymb();
	lex();
	if(symb != IF)
		error("expecting if");
	printSymb();
	lex();
	if(symb != SEMI)
		error("expecting ;");
	printSymb();
	lex();
	return n;
}

NODE * command_while(){
	extern NODE * condexpr();
	extern NODE * commands();
	NODE * n = newNode(WHILE);
	printSymb();
	lex();
	n->f.b.n1 = condexpr();
	lex();
	if(symb != LOOP){
		error("expected loop");
	}
	printSymb();
	lex();
	n->f.b.n2 = commands();
	if(symb!=END)
		error("expected end");
	printSymb();
	lex();
	if(symb!=LOOP)
		error("expected loop");
	printSymb();
	lex();
	if(symb!=SEMI)
		error("expected semi");
	printSymb();
	lex();
	return n;
}

NODE * command_assign(){
	extern NODE * expr();
	NODE * n = newNode(ASSIGN);
	n->f.b.n1 = newName(yytext);
	printSymb();
	lex();
	if(symb!=ASSIGN)
		error("expecting :=");
	printSymb();
	lex();
	n->f.b.n2 = expr();
	return n;
}

NODE * command_read(){
	extern NODE * expr();
	NODE * n = newNode(READ);
	printSymb();
	lex();
	n->f.b.n1 = expr();
	return n;
}

NODE * command_write(){
	extern NODE * expr();
	NODE * n = newNode(WRITE);
	printSymb();
	lex();
	n->f.b.n1 = expr();
	return n;
}

NODE * commands(){
	extern NODE * command_assign();
	extern NODE * command_if();
	extern NODE * command_while();
	extern NODE * command_read();
	extern NODE * command_write();
	NODE * n = newNode(SEMI);

	switch(symb)
	{
	case NAME:	n->f.b.n1 = command_assign();
			break;
	case IF:	n->f.b.n1 = command_if();
			break;
	case WHILE:	n->f.b.n1 = command_while();
			break;
	case READ:	n->f.b.n1 = command_read();
			break;
	case WRITE:	n->f.b.n1 = command_write();
			break;
	default: error("expected if/while/read/write/name");

	}
	if(symb!=END && symb!=ELSE){
		n->f.b.n2 = commands();
	}
	return n;
}

NODE * arg(){

	if(symb != NAME && symb != NUMBER)
		error("expected a name/number");
	NODE * n;
	if(symb == NAME){
		n = newName(yytext);
	} else if(symb == NUMBER){
		n = newNum(yytext);
	} else {
		n = NULL;
	}
	printSymb();
	return n;
}

NODE * args(){
	extern NODE * arg();
	NODE * n = newNode(SEMI);
	if(symb!=RBRA)
	{
	n->f.b.n1 = arg();
	lex();
	func_args++;
		if(symb == COMMA)
		{
			printSymb();
			lex();
			n->f.b.n2 = args();
		}
	}
	return n;
}

NODE * func(){
	extern NODE * commands();
	extern NODE * args();
	extern NODE * decs();
	func_args=0;
	lex();
	if(symb != NAME)
		error("expected a function name");
	NODE * f = newNode(FUNC);
	f->f.b.n0 = newName(yytext);
	lex();
	if(symb != LBRA)
		error("expected a (");
	printSymb();
	lex();
	f->f.b.n1 = args();
	//f->f.value = func_args;
	if(symb != RBRA)
		error("expected a )");
	printSymb();
	lex();
	if(symb == RETURNS){
		printSymb();
		lex();
		if(symb != NAME){
			error("expected a name");
		}
		f->f.b.n4 = newNode(RETURNS);
		f->f.b.n4->f.b.n1 = newName(yytext);
		printSymb();
		lex();
	}
	if(symb == VAR){
		printSymb();
	f->f.b.n2 = decs();
	}
	if(symb != TBEGIN)
		error("expected begin");
	printSymb();
	lex();
	f->f.b.n3 = commands();
	if(symb != END)
		error("expected end");
	printSymb();
	lex();
	if(symb != FUNC)
		error("expected function");
	printSymb();
	lex();
	if(symb != SEMI)
		error("expected ;");
	printSymb();
	lex();
	return f;
}

NODE * funcs(){
	extern NODE * func();
	printSymb();
	NODE * n = newNode(SEMI);
	NODE * f = func();
	n->f.b.n1=f;
	if(symb==FUNC){
		n->f.b.n2 = funcs();
	}
	return n;
}

NODE * dec(){
	lex();
	if(symb != NAME)
		error("expected a name");
	printSymb();
	NODE * n = newName(yytext);
	return n;
}

NODE * decs(){
	extern NODE * dec();
	NODE * n = newNode(SEMI);
	n->f.b.n1 = dec();
	lex();
	if(symb == COMMA){
		printSymb();
		n->f.b.n2 = decs();
	}
	return n;
}

NODE * program(){
	extern NODE * funcs();
	NODE * n = NULL;
	if(symb==FUNC){
		 n = funcs();
	}
	if(symb!=EOF){
		error("expected EOF");
	}
	return n;
}
