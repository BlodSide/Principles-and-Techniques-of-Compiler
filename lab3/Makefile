all:
	reset
	bison -d -v syntax.y
	flex lexical.l
	gcc -g syntax.tab.c node.c tables.c semantic.c interCode.c -lfl -o parser
test:
	reset
	./parser ./sample/1.cmm
	./parser ./sample/2.cmm
