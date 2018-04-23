%{

	#include <bits/stdc++.h>
	#include "compile.h"

	using namespace std;

	extern int yylex();
	extern int yyparse();
	extern int yylineno;
	void yyerror(string s);

	Node *ParseTreeRoot;
%}

%union
{
	Node *node;
}

%token<node> ADD SUB MUL DIV GT LT GE LE EQ NE MAIN INT TRUE FALSE FLOAT BOOL GET PUT RETURN IN OR AND IF FOR FOREACH WHILE ELSE BREAK CONTINUE INTEGERS FLOATING_POINTS IDENTIFIER SEMI LB_CURLY RB_CURLY LB_ROUND RB_ROUND COMMA EQUAL CHAR CHARACTERS MOD STRING STRING_LITERALS PUTS LIBRARY VOID

%type<node> program declr_list declr variable_declr variable_list variable type func_declr parameters param_list parameter main_function statements statement condition loop for_loop for_each_loop while_loop return_statement read write expression logical_expression and_expression relational_expression simple_expression divmul_expression unary_expression term function_call args args_list constants operator3 operator1 operator2 unary_operator write_string

%start program

%define parse.error verbose

%%

program 	:	libraries declr_list main_function
				{$$ = new Node("program", "", $2, $3, NULL); ParseTreeRoot = $$;}
			;

libraries	:	libraries LIBRARY
			|
			;

declr_list	:	declr_list declr
				{$$ = new Node("declaration_list", "", $1, $2, NULL);}
			|
				{$$ = new Node("declaration_list", "", NULL, NULL, NULL);}
			;


declr	:	variable_declr
			{$$ = new Node("declaration", "", $1, NULL, NULL);}
		|	func_declr
			{$$ = new Node("declaration", "", $1, NULL, NULL);}
		;


variable_declr	:	type variable_list SEMI
					{$$ = new Node("variable_declaration", "", $1, $2, NULL);}
				| 	type variable EQUAL expression SEMI
					{$$ = new Node("variable_declaration", "=", $1, $2, $4);}
				;

variable_list 	:	variable_list COMMA variable
					{$$ = new Node("variable_list","",$1, $3, NULL);}
				|  variable
					{$$ = new Node("variable_list","", $1, NULL, NULL);}
				;

variable 	:	IDENTIFIER
				{$$ = new Node("variable",$1->getValue(), $1, NULL, NULL);}
			;

type	:	INT
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_int);}
		| FLOAT
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_float);}
		| VOID
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_none);}
		| BOOL
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_bool);}
		| CHAR
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_char);}
		| STRING
			{$$ = new Node("type",$1->getValue(), $1, NULL, NULL); $$->setDataType(dt_string);}
		;

func_declr 	: 	type IDENTIFIER LB_ROUND parameters RB_ROUND LB_CURLY statements RB_CURLY
				{$$ = new Node("function_declaration", $2->getValue(), $1, $4, $7); $$->setDataType($1->getDataType());}
			;

parameters 	: 	param_list
				{$$ = new Node("parameters","",$1, NULL, NULL);}
			|
				{$$ = new Node("parameters","",NULL, NULL, NULL);}
			;


param_list 	:	param_list COMMA parameter
				{$$ = new Node("parameters_list","", $1, $3, NULL);}
			|	parameter
				{$$ = new Node("parameters_list","", $1, NULL, NULL);}
			;

parameter 	:	type variable
				{$$ = new Node("parameter","", $1, $2, NULL);$$->setDataType($1->getDataType());}
			;


main_function 	:	MAIN LB_ROUND RB_ROUND LB_CURLY statements RB_CURLY
					{$$ = new Node("main_function","", $5, NULL, NULL);}
				;


statements 	:	statements statement
				{$$ = new Node("statements","", $1, $2, NULL);}
			|
				{$$ = new Node("statements","", NULL, NULL, NULL);}
			;

statement 	:	variable_declr
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			| 	expression SEMI
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|   condition
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|   loop
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|	BREAK SEMI
				{$$ = new Node("statement", "break", $1, NULL, NULL);}
			|	CONTINUE SEMI
				{$$ = new Node("statement", "continue", $1, NULL, NULL);}
			|	return_statement SEMI
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|	read SEMI
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|	write_string SEMI
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|	write SEMI
				{$$ = new Node("statement", "", $1, NULL, NULL);}
			|	LB_CURLY statements RB_CURLY
				{$$ = new Node("statement", "scope", $2, NULL, NULL);}
			| 	error SEMI
				{yyerrok;}
			;

condition 	: 	IF LB_ROUND expression RB_ROUND LB_CURLY statements RB_CURLY ELSE LB_CURLY statements RB_CURLY
				{$$ = new Node("condition", "", $3, $6, $10);}
			| 	IF LB_ROUND expression RB_ROUND LB_CURLY statements RB_CURLY
				{$$ = new Node("condition", "", $3, $6, NULL);}
			;

loop	:	for_loop
			{$$ = new Node("loop","", $1, NULL, NULL);}
		| 	for_each_loop
			{$$ = new Node("loop","", $1, NULL, NULL);}
		| 	while_loop
			{$$ = new Node("loop","", $1, NULL, NULL);}
		;

for_each_loop 	: 	FOREACH LB_ROUND variable IN simple_expression RB_ROUND LB_CURLY statements RB_CURLY
					{$$ = new Node("for_each_loop","", $3, $5, $8);}
			;

for_loop 	: 	FOR LB_ROUND expression SEMI expression SEMI expression RB_ROUND LB_CURLY statements RB_CURLY
				{$$ = new Node("for_loop","", $3, $5, $7); $$->addChild4($10);}
			;

while_loop 	:	WHILE LB_ROUND expression RB_ROUND LB_CURLY statements RB_CURLY
				{$$ = new Node("while_loop","", $3, $6, NULL);}
			;

return_statement 	: 	RETURN
						{$$ = new Node("return_statement","", $1, NULL, NULL);}
					| 	RETURN expression
						{$$ = new Node("return_statement","", $1, $2, NULL);}
					;

read	:	GET LB_ROUND variable RB_ROUND
			{$$ = new Node("read","", $3, NULL, NULL);}
		;

write_string 	: 	PUTS LB_ROUND STRING_LITERALS RB_ROUND
					{$$ = new Node("write_string", "", $3, NULL, NULL);}
				;

write 	: 	PUT LB_ROUND expression RB_ROUND
			{$$ = new Node("write","", $3, NULL, NULL);}
			;


expression 	:	variable EQUAL expression
				{$$ = new Node("expression","=", $1, $3, NULL);}
			| 	logical_expression
				{$$ = new Node("expression","", $1, NULL, NULL);}
			;

logical_expression 	:	logical_expression OR and_expression
						{$$ = new Node("logical_expression","or", $1, $3, NULL);}
					| 	and_expression
						{$$ = new Node("logical_expression","", $1, NULL, NULL);}
					;

and_expression 	:	and_expression AND relational_expression
					{$$ = new Node("and_expression","and", $1, $3, NULL);}
				|	relational_expression
					{$$ = new Node("and_expression","", $1, NULL, NULL);}
				;

relational_expression 	:	relational_expression operator3 simple_expression
							{$$ = new Node("relational_expression","op", $1, $2, $3);}
						|	simple_expression
							{$$ = new Node("relational_expression","", $1, NULL, NULL);}
						;

simple_expression 	:	simple_expression operator1 divmul_expression
						{$$ = new Node("simple_expression","op", $1, $2, $3);}
					|	divmul_expression
						{$$ = new Node("simple_expression","", $1, NULL, NULL);}
					;

divmul_expression 	: 	divmul_expression operator2 unary_expression
						{$$ = new Node("divmul_expression","op", $1, $2, $3);}
					| 	unary_expression
						{$$ = new Node("divmul_expression","", $1, NULL, NULL);}
					;

unary_expression 	: 	unary_operator term
						{$$ = new Node("unary_expression","", $1, $2, NULL);}
					| 	term
						{$$ = new Node("unary_expression","", $1, NULL, NULL);}
					;

term 	:	LB_ROUND expression RB_ROUND
			{$$ = new Node("term","", $2, NULL, NULL);}
		| 	function_call
			{$$ = new Node("term","", $1, NULL, NULL);}
		|	constants
			{$$ = new Node("term","", $1, NULL, NULL);}
		|	variable
			{$$ = new Node("term","", $1, NULL, NULL);}
		;

function_call 	:	IDENTIFIER LB_ROUND args RB_ROUND
					{$$ = new Node("function_call",$1->getValue(), $3, NULL, NULL);}
				;

args 	:  	args_list
			{$$ = new Node("args", "", $1, NULL, NULL);}
		|
			{$$ = new Node("args", "", NULL, NULL, NULL);}
		;

args_list	:	args_list COMMA expression
				{$$ = new Node("args_list", "", $1, $3, NULL);}
			| 	expression
				{$$ = new Node("args_list", "", $1, NULL, NULL);}
			;

constants 	:	INTEGERS
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			| 	CHARACTERS
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			| 	STRING_LITERALS
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			| 	FLOATING_POINTS
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			| 	TRUE
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			| 	FALSE
				{$$ = new Node("constants", $1->getValue(), $1, NULL, NULL); $$->setDataType($1->getDataType());}
			;

operator1 	: 	ADD
				{$$ = new Node("operator1", "+", $1, NULL, NULL);}
			| 	SUB
				{$$ = new Node("operator1", "-", $1, NULL, NULL);}
			;

operator2 	: 	MUL
				{$$ = new Node("operator2", "*", $1, NULL, NULL);}
			| 	DIV
				{$$ = new Node("operator2", "/", $1, NULL, NULL);}
			| 	MOD
				{$$ = new Node("operator2", "%", $1, NULL, NULL);}
			;

operator3 	: 	GT
				{$$ = new Node("operator3", ">", $1, NULL, NULL);}
			| 	LT
				{$$ = new Node("operator3", "<", $1, NULL, NULL);}
			| 	GE
				{$$ = new Node("operator3", ">=", $1, NULL, NULL);}
			| 	LE
				{$$ = new Node("operator3", "<=", $1, NULL, NULL);}
			| 	EQ
				{$$ = new Node("operator3", "==", $1, NULL, NULL);}
			| 	NE
				{$$ = new Node("operator3", "!=", $1, NULL, NULL);}
			;

unary_operator 	:	SUB
					{$$ = new Node("unary_operator", "-", $1, NULL, NULL);}
				|	ADD
					{$$ = new Node("unary_operator", "+", $1, NULL, NULL);}
				;


%%

bool syntax_success = true;

void yyerror(string s){
	cerr<<"Line Number " << yylineno <<" : "<<s<<endl;
	syntax_success = false;
}

int main(){
	// Parse the input and build the syntax tree
	cout<<"PARSING"<<endl;
	yyparse();
	if(syntax_success){
		cout<<"No Syntax Errors!"<<endl;
	} else {
		cout<<"There were Syntax Errors!"<<endl;
		exit(1);
	}
	cout<<"__________________________________"<<endl<<endl;
	cout<<"SEMANTIC CHECK"<<endl;

	// Annotate the syntax tree --> Semantic Analysis
	SemanticAnalysis semantic_checker(ParseTreeRoot);
	semantic_checker.errors();		// Print success message or the errors in semantic analysis(if any)
	cout<<"__________________________________"<<endl<<endl;
	cout<<"CODE GENERATION"<<endl;

	// Generate MIPS Code
	MIPSCode mips_code;
	mips_code.generateCode(ParseTreeRoot);
	mips_code.generateDataSection();
	mips_code.generateOutput();

	cout<<"Compilation Successfull!"<<endl;
	cout<<"__________________________________"<<endl<<endl;

}

