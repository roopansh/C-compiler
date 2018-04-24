# Compiler

Compiler for a C-like language. Converts the code to MIPS Assembly Language which can be run using SPIM.

## How to run

	$ cd src

	$ make

	$ ./compiler < sample.c

## Output files

*tree.txt* : Parse Tree of the sample code

*intermediate.txt* : Intermediate code generated for the sample file

*mips.s* : MIPS Assembly code

## Features of the Language

* Data types : void, int, bool, char, string

* Variable Declaration

* Variable Assignment

* Function Declaration

* Reading from console

* Printing to console

* Logical Expressions involving '&&' and '||'

* Relational operators : '>', '<', '>=', '<=', '==', '<>', '!='

* Arithmatic operators : '+', '-', '*', '/', '%'

* Unary Operators : '+', '-'

* For Loop

* Foreach loop

* While Loop

* Conditional statements

* Nested code blocks

* Explicit Scope specifiers

* breaks in loops

* continues in loops
