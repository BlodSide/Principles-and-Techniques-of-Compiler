%{
	#include"node.h"
	#include"lex.yy.c"
	#include"tables.h"
	#include"semantic.h"
	Node* root;
	char errorStr[200];
%}

%union {
	int type_int;
	float type_float;
	double type_double;
	Node* node;
};

%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%right ASSIGNOP  
%left OR 
%left AND 
%left RELOP
%left PLUS MINUS 
%left STAR DIV
%right NOT
%left DOT LB RB LP RP
%nonassoc LOWER_THAN_ELSE 
%nonassoc ELSE

%type <node> Program ExtDefList ExtDef ExtDecList Specifier
%type <node> StructSpecifier OptTag Tag VarDec FunDec VarList
%type <node> ParamDec CompSt StmtList Stmt DefList Def DecList
%type <node> Dec Exp Args

%%

/*--------------------High-level Definitions--------------------*/
Program : ExtDefList {$$=initNode("Program","");addChild($$,$1);root=$$;}
	;
ExtDefList : ExtDef ExtDefList {$$=initNode("ExtDefList","");addChild($$,$2);addChild($$,$1);}
	|  /*empty*/ {$$=initNode("ExtDefList","");}
	;
ExtDef : Specifier ExtDecList SEMI {$$=initNode("ExtDef","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}	
	| Specifier SEMI {$$=initNode("ExtDef","");addChild($$,$2);addChild($$,$1);}
	| Specifier FunDec SEMI {$$=initNode("ExtDef","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Specifier FunDec CompSt {$$=initNode("ExtDef","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| error SEMI {errorCount++;}
	;
ExtDecList : VarDec {$$=initNode("ExtDecList","");addChild($$,$1);}
	| VarDec COMMA ExtDecList {$$=initNode("ExtDecList","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;

/*--------------------Specifiers--------------------*/
Specifier : TYPE {$$=initNode("Specifier","");addChild($$,$1);}
	| StructSpecifier {$$=initNode("Specifier","");addChild($$,$1);}
	;
StructSpecifier : STRUCT OptTag LC DefList RC {$$=initNode("StructSpecifier","");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| STRUCT Tag {$$=initNode("StructSpecifier","");addChild($$,$2);addChild($$,$1);}
	;
OptTag : ID {$$=initNode("OptTag","");addChild($$,$1);}
	|  /*empty*/ {$$=initNode("OptTag","");}
	;
Tag : ID {$$=initNode("Tag","");addChild($$,$1);}
	;

/*--------------------Declarators--------------------*/
VarDec : ID {$$=initNode("VarDec","");addChild($$,$1);}
	| VarDec LB INT RB {$$=initNode("VarDec","");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| VarDec LB error SEMI {myerror("Missing \"]\"");errorCount++;}
	;
FunDec : ID LP VarList RP {$$=initNode("FunDec","");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| ID LP RP {$$=initNode("FunDec","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| error RP {errorCount++;}
	;
VarList : ParamDec COMMA VarList {$$=initNode("VarList","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| ParamDec {$$=initNode("VarList","");addChild($$,$1);}
	;
ParamDec : Specifier VarDec {$$=initNode("ParamDec","");addChild($$,$2);addChild($$,$1);}
	;

/*--------------------Statements--------------------*/
CompSt : LC DefList StmtList RC {$$=initNode("CompSt","");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| LC DefList error RC {errorCount++;}
	;
StmtList : Stmt StmtList {$$=initNode("StmtList","");addChild($$,$2);addChild($$,$1);}
	|  /*empty*/ {$$=initNode("StmtList","");}	
	;
Stmt : Exp SEMI {$$=initNode("Stmt","");addChild($$,$2);addChild($$,$1);}
	| CompSt {$$=initNode("Stmt","");addChild($$,$1);}
	| RETURN Exp SEMI {$$=initNode("Stmt","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE /*提高移入ELSE的优先级*/ {$$=initNode("Stmt","");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| IF LP Exp RP Stmt ELSE Stmt {$$=initNode("Stmt","");addChild($$,$7);addChild($$,$6);addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| IF LP Exp RP error ELSE Stmt {myerror("Missing \";\"");errorCount++;}
	| WHILE LP Exp RP Stmt {$$=initNode("Stmt","");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp LB error SEMI {myerror("Missing \"]\"");errorCount++;}
	| IF LP error SEMI {myerror("Missing \")\"");errorCount++;}
	| LP Exp error SEMI {myerror("Missing \")\"");errorCount++;}
	| ID LP error SEMI {myerror("Missing \")\"");errorCount++;}
	| error SEMI {errorCount++;}
	;

/*--------------------Local Definitions--------------------*/
DefList : Def DefList {$$=initNode("DefList","");addChild($$,$2);addChild($$,$1);}
	|  /*empty*/ {$$=initNode("DefList","");}
	;
Def : Specifier DecList SEMI {$$=initNode("Def","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Specifier error SEMI {errorCount++;}
	;
DecList : Dec {$$=initNode("DecList","");addChild($$,$1);}
	| Dec COMMA DecList {$$=initNode("DecList","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
Dec : VarDec {$$=initNode("Dec","");addChild($$,$1);}
	| VarDec ASSIGNOP Exp {$$=initNode("Dec","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;

/*--------------------Expressions--------------------*/
Exp : Exp ASSIGNOP Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp AND Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp OR Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp RELOP Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp PLUS Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp MINUS Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp STAR Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp DIV Exp {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| LP Exp RP {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| MINUS Exp {$$=initNode("Exp","");addChild($$,$2);addChild($$,$1);}
	| NOT Exp {$$=initNode("Exp","");addChild($$,$2);addChild($$,$1);}
	| ID LP Args RP {$$=initNode("Exp","");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| ID LP RP {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp LB Exp RB {$$=initNode("Exp","");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp DOT ID {$$=initNode("Exp","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| ID {$$=initNode("Exp","");addChild($$,$1);}
	| INT {$$=initNode("Exp","");addChild($$,$1);}
	| FLOAT {$$=initNode("Exp","");addChild($$,$1);}
	;
Args : Exp COMMA Args {$$=initNode("Args","");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp {$$=initNode("Args","");addChild($$,$1);}
	;

%%
int main(int argc, char* argv[])
{
 	if (argc <= 1) return 1;
	FILE* fp = fopen(argv[1],"r");
	if (!fp)
	{
		perror(argv[1]);
		return 1;
	}	
	root=NULL;
	yylineno=1;
	yyrestart(fp);
	yyparse();
	printf("%s",errorStr);
	if(errorCount == 0){
		//printTree(root,0);
		initTable();
		Program(root);
	}
	return 0;
}

int yyerror(char* msg){
	if(newLine){
		printf("%s",errorStr);
		sprintf(errorStr,"Error type B at line %d: %s.  (unexpected near '%s')\n", yylineno, msg, yylval.node->value);
	}
}

int myerror(char* msg){
	if(newLine){
		printf("Error type B at line %d: %s.  (unexpected near '%s')\n", yylineno, msg, yylval.node->value);
		strcpy(errorStr,"");
		newLine = 0;
	}
}
