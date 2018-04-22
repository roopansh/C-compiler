all : compiler

compiler : lex.yy.c parser.tab.c parser.tab.h
	g++ -g -std=c++11 lex.yy.c parser.tab.c -o compiler

lex.yy.c : lexer.l parser.tab.h compile.h
	flex lexer.l

parser.tab.c parser.tab.h :	parser.y compile.h
	bison -d -v parser.y

clean:
	rm lex.yy.c parser.tab.c parser.tab.h parser.output compiler
