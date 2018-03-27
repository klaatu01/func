%{
#include "tokens.h"
%}

number    [1-9][0-9]*|[0-9]
name	[a-zA-Z][A-Za-z0-9]*
         
%%

";" 	{ return SEMI;}
","		{ return COMMA;}
":=" 	{ return ASSIGN;}
"if"	{ return IF;}
"then"	{ return THEN;}
"else"	{ return ELSE;}
"begin"	{ return TBEGIN;}
"returns" {return RETURNS;}
"end"	{ return END;}
"while" { return WHILE;}
"loop" 	{ return LOOP;}
"read"	{ return READ;}  
"write"	{ return WRITE;} 
"variables" 	{ return VAR;}
"(" 	{ return LBRA;}
")" 	{ return RBRA;}
"Less" 	{ return LT;}
"LessEq" 	{ return LTE;}
"Eq" 	{ return EQ;}
"NEq" 	{ return NEQ;}
"function" {return FUNC;}
"Times" {return MULT;}
"Divide" {return DIV;}
"Plus" {return PLUS;}
"Minus" {return MINUS;}
{number} {return NUMBER;}
{name}	 {return NAME;}
<<EOF>>	 {return EOF;}

[ \t\n]+          /* eat up whitespace */


%%

int yywrap() { return EOF; }
