%{

	#include <bits/stdc++.h>
	#include "compile.h"
	#include "parser.tab.h"

	using namespace std;

%}

%option yylineno
%option noyywrap

SEMI	";"
EQUAL	"="
ADD		"+"
SUB		"-"
MUL		"*"
DIV		"/"
GT		">"
LT		"<"
GE		">="
LE		"<="
EQ		"=="
NE 		("!="|"<>")
OR 		"||"
AND 	"&&"
LB_CURLY					"{"
RB_CURLY					"}"
LB_ROUND					"("
RB_ROUND					")"
COMMA 	","
MAIN	"main"
INT		"int"
TRUE 	"true"
FALSE	"false"
FLOAT 	"float"
BOOL	"bool"
GET 	"get"
PUT 	"put"
RETURN 	"return"
IN 		"in"
IF 		"if"
FOR		"for"
WHILE	"while"
ELSE	"else"
BREAK   "break"
CONTINUE 	"continue"

INTEGERS 						([0-9]+)
FLOATING_POINTS					([0-9]+\.[0-9]+)
IDENTIFIER		([A-Za-z_]([A-Za-z0-9_])*)
WHITE_SPACES 	([ \t]+)
NEW_LINE 		([\n])

%%

{SEMI}			{(yylval.node) = new Node("SEMI", yytext, NULL, NULL, NULL); return SEMI;}
{EQUAL}			{(yylval.node) = new Node("EQUAL", yytext, NULL, NULL, NULL); return EQUAL;}
{ADD} 			{(yylval.node) = new Node("ADD", yytext, NULL, NULL, NULL); return ADD;}
{SUB} 			{(yylval.node) = new Node("SUB", yytext, NULL, NULL, NULL); return SUB;}
{MUL} 			{(yylval.node) = new Node("MUL", yytext, NULL, NULL, NULL); return MUL;}
{DIV} 			{(yylval.node) = new Node("DIV", yytext, NULL, NULL, NULL); return DIV;}
{GT} 			{(yylval.node) = new Node("GT", yytext, NULL, NULL, NULL); return GT;}
{LT} 			{(yylval.node) = new Node("LT", yytext, NULL, NULL, NULL); return LT;}
{GE} 			{(yylval.node) = new Node("GE", yytext, NULL, NULL, NULL); return GE;}
{LE} 			{(yylval.node) = new Node("LE", yytext, NULL, NULL, NULL); return LE;}
{EQ} 			{(yylval.node) = new Node("EQ", yytext, NULL, NULL, NULL); return EQ;}
{NE} 			{(yylval.node) = new Node("NE", yytext, NULL, NULL, NULL); return NE;}
{MAIN} 			{(yylval.node) = new Node("MAIN", yytext, NULL, NULL, NULL); return MAIN;}
{INT} 			{(yylval.node) = new Node("INT", yytext, NULL, NULL, NULL); return INT;}
{TRUE} 			{(yylval.node) = new Node("TRUE", yytext, NULL, NULL, NULL); yylval.node->setDataType(dt_bool); return TRUE;}
{FALSE} 		{(yylval.node) = new Node("FALSE", yytext, NULL, NULL, NULL); yylval.node->setDataType(dt_bool); return FALSE;}
{FLOAT} 		{(yylval.node) = new Node("FLOAT", yytext, NULL, NULL, NULL); return FLOAT;}
{BOOL} 			{(yylval.node) = new Node("BOOL", yytext, NULL, NULL, NULL); return BOOL;}
{GET} 			{(yylval.node) = new Node("GET", yytext, NULL, NULL, NULL); return GET;}
{PUT} 			{(yylval.node) = new Node("PUT", yytext, NULL, NULL, NULL); return PUT;}
{RETURN} 		{(yylval.node) = new Node("RETURN", yytext, NULL, NULL, NULL); return RETURN;}
{IN} 			{(yylval.node) = new Node("IN", yytext, NULL, NULL, NULL); return IN;}
{OR} 			{(yylval.node) = new Node("OR", yytext, NULL, NULL, NULL); return OR;}
{AND} 			{(yylval.node) = new Node("AND", yytext, NULL, NULL, NULL); return AND;}
{IF} 			{(yylval.node) = new Node("IF", yytext, NULL, NULL, NULL); return IF;}
{FOR} 			{(yylval.node) = new Node("FOR", yytext, NULL, NULL, NULL); return FOR;}
{WHILE} 		{(yylval.node) = new Node("WHILE", yytext, NULL, NULL, NULL); return WHILE;}
{ELSE} 			{(yylval.node) = new Node("ELSE", yytext, NULL, NULL, NULL); return ELSE;}
{BREAK} 		{(yylval.node) = new Node("BREAK", yytext, NULL, NULL, NULL); return BREAK;}
{CONTINUE} 		{(yylval.node) = new Node("CONTINUE", yytext, NULL, NULL, NULL); return CONTINUE;}
{LB_CURLY}		{(yylval.node) = new Node("LB_CURLY", yytext, NULL, NULL, NULL); return LB_CURLY;}
{RB_CURLY}		{(yylval.node) = new Node("RB_CURLY", yytext, NULL, NULL, NULL); return RB_CURLY;}
{LB_ROUND}		{(yylval.node) = new Node("LB_ROUND", yytext, NULL, NULL, NULL); return LB_ROUND;}
{RB_ROUND}		{(yylval.node) = new Node("RB_ROUND", yytext, NULL, NULL, NULL); return RB_ROUND;}
{COMMA}			{(yylval.node) = new Node("COMMA", yytext, NULL, NULL, NULL); return COMMA;}

{INTEGERS}		{(yylval.node) = new Node("INTEGERS", yytext, NULL, NULL, NULL); yylval.node->setDataType(dt_int); return INTEGERS;}
{FLOATING_POINTS}		{(yylval.node) = new Node("FLOATING_POINTS", yytext, NULL, NULL, NULL); yylval.node->setDataType(dt_float); return FLOATING_POINTS;}
{IDENTIFIER}			{(yylval.node) = new Node("IDENTIFIER", yytext, NULL, NULL, NULL); return IDENTIFIER;}

{NEW_LINE} 			{}
{WHITE_SPACES}		{}

.					{cerr<< "TOKEN CANNOT BE MATCHED : ' "<< yytext <<" '"<<endl;}

%%
