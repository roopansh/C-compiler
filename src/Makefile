all : compiler
	rm lex.yy.c parser.tab.c parser.tab.h parser.output

compiler : lex.yy.c parser.tab.c parser.tab.h
	g++ lex.yy.c parser.tab.c -o compiler

lex.yy.c : lexer.l parser.tab.h
	flex lexer.l

parser.tab.c parser.tab.h :	parser.y
	bison -d -v parser.y

clean:
	rm lex.yy.c parser.tab.c parser.tab.h parser.output compiler
