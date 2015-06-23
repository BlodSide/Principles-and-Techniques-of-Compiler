all:
	bison -d -v syntax.y
	flex lexical.l
	gcc -g syntax.tab.c node.c -lfl -o parser
test:
	reset
	./parser 1.cmm
	./parser 2.cmm
	./parser 3.cmm
	./parser 4.cmm
test_2:
	reset
	./parser 1_.cmm
	./parser 2_.cmm
	./parser 3_.cmm
	./parser 4_.cmm
	./parser 5_.cmm
	./parser 6_.cmm
