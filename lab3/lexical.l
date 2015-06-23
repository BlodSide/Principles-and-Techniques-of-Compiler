%{
	#include "node.h"
	#include "syntax.tab.h"
	int errorCount = 0;
	int newLine = 1;
%}

comments_ "//"[^\n]*
comments "/*"(.|\r|\n)*?"*/"
comments_illegal "/*"?(.|\r|\n)*?"*/"(.|\r|\n)*?"*/"
ws [ \t]+
decimal ([1-9][0-9]*)|0
octal 0[0-7]*
octal_ 0[0-9]*
hex ("0x"|"0X")[0-9a-fA-F]*
hex_ ("0x"|"0X")[0-9a-zA-Z]*
integer {decimal}|{hex}|{octal}
float {decimal}"."[0-9]+
float_e [0-9]*"."[0-9]*[Ee][+-]?[0-9]+
float_ ({decimal}".")|("."{decimal})
float_e_ [0-9]*"."[0-9]*[Ee][+-]?
id [a-zA-Z_][0-9a-zA-Z_]*
type int|float

%option yylineno
%%
{comments_illegal} 	{errorCount++; printf("Error type A at line %d: Illegal comments.\n",yylineno);newLine=0;}
{comments}	{;}
{comments_} 	{;}
{ws} 		{;}
\n		{newLine=1;}
{float_e} 	{yylval.node=initNode("FLOAT",yytext);return FLOAT;}
{float_e_}	{errorCount++; printf("Error type A at line %d: Illegal float number '%s'\n",yylineno,yytext);newLine=0;return FLOAT;}
{float} 	{yylval.node=initNode("FLOAT",yytext);return FLOAT;}
{float_}	{errorCount++; printf("Error type A at line %d: Illegal float number '%s'\n",yylineno,yytext);newLine=0;return FLOAT;}
{integer} 	{yylval.node=initNode("INT",yytext);return INT;}
{hex_}		{errorCount++; printf("Error type A at line %d: Illegal hex number '%s'\n",yylineno,yytext);newLine=0;return INT;}
{octal_}	{errorCount++; printf("Error type A at line %d: Illegal octal number '%s'\n",yylineno,yytext);newLine=0;return INT;}
{type} 		{yylval.node=initNode("TYPE",yytext);return TYPE;}
struct 		{yylval.node=initNode("STRUCT",yytext);return STRUCT;}
return 		{yylval.node=initNode("RETURN",yytext);return RETURN;}
if		{yylval.node=initNode("IF",yytext);return IF;}
else 		{yylval.node=initNode("ELSE",yytext);return ELSE;}
while 		{yylval.node=initNode("WHILE",yytext);return WHILE;}
{id} 		{yylval.node=initNode("ID",yytext);return ID;}
";" 		{yylval.node=initNode("SEMI",yytext);newLine=1;return SEMI;}
"," 		{yylval.node=initNode("COMMA",yytext);return COMMA;}
"=" 		{yylval.node=initNode("ASSIGNOP",yytext);return ASSIGNOP;}
">" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
"<" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
">=" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
"<=" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
"==" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
"!=" 		{yylval.node=initNode("RELOP",yytext);return RELOP;}
"+" 		{yylval.node=initNode("PLUS",yytext);return PLUS;}
"-" 		{yylval.node=initNode("MINUS",yytext);return MINUS;}
"*" 		{yylval.node=initNode("STAR",yytext);return STAR;}
"/" 		{yylval.node=initNode("DIV",yytext);return DIV;}
"&&" 		{yylval.node=initNode("AND",yytext);return AND;}
"||" 		{yylval.node=initNode("OR",yytext);return OR;}
"." 		{yylval.node=initNode("DOT",yytext);return DOT;}
"!" 		{yylval.node=initNode("NOT",yytext);return NOT;}
"(" 		{yylval.node=initNode("LP",yytext);return LP;}
")" 		{yylval.node=initNode("RP",yytext);return RP;}
"[" 		{yylval.node=initNode("LB",yytext);return LB;}
"]" 		{yylval.node=initNode("RB",yytext);return RB;}
"{" 		{yylval.node=initNode("LC",yytext);return LC;}
"}" 		{yylval.node=initNode("RC",yytext);return RC;}
. 		{errorCount++;printf("Error type A at line %d: Mysterious character '%s'\n",yylineno,yytext);}
%%
