#include <stdio.h>
#include <stdlib.h>
#include "tokens.h"

extern int symb;
extern char * yytext;

char * showSymb(int symb)
{  switch(symb)
   {  case  TBEGIN: return "BEGIN";
      case  LOOP: return "LOOP";
      case  ELSE: return "ELSE";
      case  END: return "END";
      case  NAME: return "NAME";
      case  IF: return "IF";
      case  NUMBER: return "NUMBER";
      case  READ: return "READ";
      case  WRITE: return "WRITE";
      case  THEN: return "THEN";
      case  VAR: return "VAR";
      case  WHILE: return "WHILE";
      case  PLUS: return "PLUS";
      case  MINUS: return "MINUS";
      case  MULT: return "MULT";
      case  DIV: return "DIV";
      case  LBRA: return "(";
      case  RBRA: return ")";
      case  LT: return "LESS";
      case  LTE: return "LESSEQ";
      case  EQ: return "EQ";
      case  NEQ: return "NEQ";
      case  ASSIGN: return ":=";
      case SEMI: return ";";
      case EOF: return "EOF";
      case COMMA: return ",";
      case FUNC:return "FUNC";
      case RETURNS:return "RETURNS";
      default: printf("bad symbol: %d",symb);
   }
}

char * printSymb()
{  char * s;
   #ifdef OUTPUT
   printf("%s ",showSymb(symb));
   if(symb==NAME || symb==NUMBER){
    if(yytext == NULL)
      printf("Error: yytext is null");
    else
	  printf("%s \n",yytext);
   } else 
	printf("\n");
   #endif
}

