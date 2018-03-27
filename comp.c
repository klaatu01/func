#include <stdio.h>
#include <stdlib.h>
#include "nodes.h"
#include "tokens.h"
#include <string.h>


extern void showTree(NODE * ast, int);
extern int symb;
extern printSymb();
extern char* showSymb(int);
extern int yylex(void);
extern FILE* yyin;
extern char * yytext;
extern NODE * program(int);

//deinfing boolean
typedef int bool;
#define true 1
#define false 0

//Label Number
int label_if_n;
int label_lp_n;
int label_fn_n;
//Max Registers
#define MAXREG 16
char * registers[MAXREG];

int rp=0;
int rb=0;

// this should be $t8 ($24) and $t9 ($25)
#define E1 16
#define E2 17
#define A0 18
#define A1 19
#define E1s "$t8"
#define E2s "$t9"
#define A0s "$a0"

char * conv_lower(char * s){
	char * str = strdup(s);
	if(!strcmp(s,"Main")){
		for(int i = 0; str[i]; i++){
  			str[i] = tolower(str[i]);
		}
	}
	return str;
}

char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


checkvar(char * id)
{  int i;
   for(i=rp-1;i>=rb;i--)
    if(strcmp(id,registers[i])==0)
     return i;
   return -1;
}

findvar(char * id)
{  int i;
   for(i=rp-1;i>=0;i--)
    if(strcmp(id,registers[i])==0)
     return i;
   return -1;
}

char * regname(int r)
{
  char rnum[2];
  switch(r)
  {
    case E1:
      return E1s;
    case E2:
      return E2s;
    case A0:
      return "$a0";
	case A1:
		return "$a1";
    default:
     if (r < 8){
      sprintf(rnum, "%d", r);
      return concat("$s",rnum);
    }else{
      sprintf(rnum, "%d", r-8);
      return concat("$t",rnum);  
    } 
  }
}

code_op(int RD, NODE * n){
	NODE * args = n->f.b.n1;
	//PUSHES ARGS TO E1 E2 ...
	code_args(E1,args);
	switch(n->tag){
		case MINUS: code_min(RD,n);
					return;
		case MULT:	code_mult(RD,n);
					return;
		case PLUS:	code_plus(RD,n);
					return;
		case DIV:	code_div(RD,n);
					return;
	}
}

code_push(int r)
{
	printf("\taddi $sp,$sp,-4\n");
	printf("\tsw %s,0($sp)\n",regname(r));
}

code_pop(int r)
{
	printf("\tlw %s,0($sp)\n",regname(r));
	printf("\taddi $sp,$sp,4\n");
}

code_min(int RD, NODE * n){
    //pop top two elements from stack;
    printf("\tsub %s,%s,%s\n",regname(RD),regname(E1),regname(E2));
}

code_div(int RD, NODE * n){
    //pop top two elements from stack;
    printf("\tdiv %s,%s,%s\n",regname(RD),regname(E1),regname(E2));
}

code_mult(int RD, NODE * n){
    //pop top two elements from stack;
    printf("\tmult %s,%s\n",regname(E1),regname(E2));
	printf("\tmflo %s\n",regname(RD));
}


code_plus(int RD, NODE * n){
	//pop top two elements from stack;
	printf("\tadd %s,%s,%s\n",regname(RD),regname(E1),regname(E2));
}

code_exp(int RD, NODE * n){
	if(n->tag == SEMI){
		NODE * n2 = n->f.b.n1;
		if(n2 != NULL){
		switch(n2->tag){
				case MINUS: code_op(RD,n2);
            			    break;
        		case PLUS:  code_op(RD,n2);
            			    break;
        		case MULT:  code_op(RD,n2);
            			    break;
       			case DIV:   code_op(RD,n2);
            			    break;
        		case NUMBER:code_arg(RD,n2);
                			break;
        		case NAME:  code_arg(RD,n2);
							break;
				default:
							code_error(n,"NONE FOUND");
							break;
			}
		}
	}
	if(n->f.b.n3 != NULL){
		code_exp(RD+1,n->f.b.n3);
	}
}

char * code_inv(int tag){

switch(tag)
   {  case LT: return "ge";
      case LTE: return "gt";
      case EQ: return "ne";
      case NEQ: return "eq";
	}
}

code_if(NODE * n){
	int ln = label_if_n;
	label_if_n++;
	NODE * cond = n->f.b.n1;
	code_cond(cond);
	NODE * coms = n->f.b.n2;
	//code_commands(coms);
	NODE * el = n->f.b.n3;
	if(el != NULL){
	printf("\tb%s %s,%s,IFALSE%d\n",code_inv(cond->tag),E2s,E1s,ln);
	code_commands(coms);
	printf("\tj IFEND%d\n",ln);
	printf("IFALSE%d:\n",ln);
	code_commands(el);
	//code_commands(el);
	} else {
	printf("\tb%s %s,%s,IFEND%d\n",code_inv(cond->tag),E2s,E1s,ln);
    code_commands(coms);
	}
	printf("IFEND%d:\n",ln);
}

code_write(NODE * n){
	NODE * exp = n->f.b.n1;
	printf("\tli  $v0,1\n");
	code_exp(A0,exp);
	//code_pop(A0);
	printf("\tsyscall\n");
}

code_read(NODE * n){
	NODE * n2 = n->f.b.n1;
	int reg;
   reg = findvar(n2->f.b.n1->f.id);
   if(reg==-1)
    code_error(n2,"not declared");
   //prints output

   // prints INPUT>
   printf("\tli  $v0,4\n");  
   printf("\tla  $a0,sinp\n"); 
   printf("\tsyscall\n");
   // reads input
   printf("\tli  $v0,5\n");  
   printf("\tsyscall\n");
   printf("\tmove  %s,$v0\n",regname(reg));
}

code_while(NODE * n){
	int ln = label_lp_n;
	label_lp_n++;
	printf("WLOOP%d:\n",ln);
	NODE * cond = n->f.b.n1;
	code_cond(cond);
	printf("\tb%s %s,%s,WEND%d\n",code_inv(cond->tag),E1s,E2s,ln);
	NODE * com = n->f.b.n2;
	code_commands(com);
	printf("\tj WLOOP%d\n",ln);
	printf("WEND%d:\n",ln);
}

code_load(int RD, int LOC){
	printf("\tmove %s,%s\n",regname(RD),regname(LOC));
}

code_load_num(int RD, int num){
	printf("\tli %s,%s\n",regname(RD),num);
}

code_cond(NODE * n){
	NODE * arg1 = n->f.b.n1;
	NODE * arg2 = n->f.b.n1->f.b.n2->f.b.n1;
	if(arg1->f.b.n1->tag == NUMBER){
		code_load_num(E1,arg1->f.b.n1->f.value);
	} else {
		int reg1 = findvar(arg1->f.b.n1->f.id);
		code_load(E1,reg1);
	}
	if(arg2->tag==NUMBER){
		code_load_num(E2,arg2->f.value);
	} else {
		int reg2= findvar(arg2->f.id);
		code_load(E2,reg2);
	}

}

code_assign(NODE * n){
	int reg = findvar(n->f.b.n1->f.id);
	if(reg != -1){
	NODE * n2 = n->f.b.n2;
	code_exp(reg,n2);
	} else {
		code_error(n,"var not found in reg");
	}
	//printf("\n");
}

code_command(NODE * n){
	switch(n->tag){
		case IF:	code_if(n);
					break;
		case WHILE:	code_while(n);
					break;
		case READ:	code_read(n);
					break;
		case WRITE:	code_write(n);
					break;
		case ASSIGN:code_assign(n);
					break;
	}
}

code_run_fun(NODE * n){
	NODE * arg = n->f.b.n1;
	printf("\n#about to run %s\n",n->f.id);
	code_args(A0,arg);
	//code_pop(A1);
	//code_pop(A0);
	//printf("\taddi $sp,$sp,-12\n");
   // printf("\tsw $ra,8($sp)\n");
	printf("\tjal %s\n",n->f.id);
}

code_arg(int RD, NODE * n){
	if(n != NULL){
		if(n->tag == NUMBER){
			printf("\tli %s,%d\n", regname(RD),n->f.value);
		} else if(n->tag == NAME){
			if(n->f.b.n1 != NULL){
			/*int rb1;
			rb1 = rb;
			rb = rp;*/
			//code_push(E1);
			//code_push(E2);
			code_run_fun(n);
			//code_pop(E2);
			//code_pop(E1);
			/*rp = rb;
			rb = rb1;*/
			printf("\tmove %s,$v0\n", regname(RD));
			} else {
			int reg = findvar(n->f.id);
			printf("\tmove %s,%s\n", regname(RD),regname(reg));
			}
		} else {
		}
	//code_push(RD);
	}
}

code_commands(NODE * n){
	if(n!=NULL && n->tag == SEMI){
			NODE * com = n->f.b.n1;
			code_command(com);
			NODE * n2 = n->f.b.n2;
		if(n2 != NULL){
			code_commands(n2);
		}
	}
}

code_args(int RD, NODE * n){
	NODE * arg = n->f.b.n1;
	if(arg != NULL){
		code_arg(RD,arg);
	}
	NODE * args = n->f.b.n2;
	if(args != NULL){
		code_args(RD+1, args);
	}
}

code_error(NODE * n, char * msg){
	printf("%s\n",msg);
}

code_dec(NODE * n){
	code_var(n);
}

code_decs(NODE * n){
	NODE * n1 = n->f.b.n1;
	code_dec(n1);
	NODE *	n2 = n->f.b.n2;
	if(n2 != NULL){
		code_decs(n2);
	}
}

code_var(NODE * t)
{  if(rp==MAXREG)
    code_error(NULL,"too many variables");
   if(checkvar(t->f.id)!=-1)
	printf("%s declared already\n",t->f.id);
    //code_error(t,"declared already");
   registers[rp] = t->f.id;
   rp++;
}

code_get_arg(NODE * n, int c){
	if(n != NULL){
        if(n->tag ==NAME){
			code_var(n);
            int reg = findvar(n->f.id);
            printf("\tmove %s,$a%d\n", regname(reg),c);
        }
    }
}

code_get_args(NODE * n, int c){
	NODE * arg = n->f.b.n1;
    if(arg != NULL){
        code_get_arg(arg,c);
		c++;
    }
    NODE * args = n->f.b.n2;
    if(args != NULL){
        code_get_args(args,c);
    }

}

code_function(NODE * n){
	bool is_return = false;
	NODE * ret = n->f.b.n4;
	if(ret != NULL){
		is_return = true;
		rb = 8;
		rp = 8;
	} else {
		rb = 0;
		rp = 0;
	}
	printf("%s:\n",conv_lower(n->f.b.n0->f.id));
	NODE * args = n->f.b.n1;
	//PUSH ARGS TO STACK;
	code_get_args(args,0);
	if(ret != NULL){
		is_return = true;
		NODE * retn = ret->f.b.n1;
		code_var(retn);
	}
	NODE * decs = n->f.b.n2;
	if(decs != NULL){
		code_decs(decs);
	}
	NODE * coms = n->f.b.n3;
	code_commands(coms);
	if(is_return){
		int reg = findvar(ret->f.b.n1->f.id);
		printf("\tmove $v0,%s\n",regname(reg));
		//printf("\tlw $ra,0($sp)\n");
		//printf("\taddi $sp,$sp,12\n");
		printf("\tjr $ra\n");
	}
}

code_functions(NODE * n){
	NODE * f = n->f.b.n1;
	if(f->tag == FUNC){
		//rp = rb;
		code_function(f);
	}
	NODE * fs = n->f.b.n2;
	if(fs != NULL && fs->tag == SEMI){
		code_functions(fs);
	}
}

code_program(NODE * n){
	printf("\t.data\n");
	printf("sinp:   .asciiz \"INPUT> \"\n");
	printf("\n\t.text\n");
	printf("\t j main\n");
	if(n->tag == SEMI){
		code_functions(n);
	}
    printf("\tli $v0, 10\n"); 
    printf("\tsyscall");

}


main(int c, char ** argv){
	#ifdef OUTPUT
	printf("=====Parsing     Starting=====\n");
	#endif
	if((yyin=fopen(argv[1],"r"))==NULL){
		printf("can't open %s\n",argv[1]);
		exit(0);
   	}
	lex();
	NODE * ast = program(0);
	#ifdef OUTPUT
	showTree(ast,0);
	printf("=====Parsing   Finished=====\n");
	printf("=====Compiling Starting======\n");
	#endif
	code_program(ast);
	#ifdef OUTPUT
	printf("=====Compiling Finished=====\n");
	#endif
}

