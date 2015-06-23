all:
	reset
	bison -d -v syntax.y
	flex lexical.l
	gcc -g syntax.tab.c node.c tables.c semantic.c -lfl -o parser
test:
	reset
	./parser ./sample/1.cmm
	./parser ./sample/2.cmm
	./parser ./sample/3.cmm
	./parser ./sample/4.cmm
	./parser ./sample/5.cmm
	./parser ./sample/6.cmm
	./parser ./sample/7.cmm
	./parser ./sample/8.cmm
	./parser ./sample/9.cmm
	./parser ./sample/10.cmm
	./parser ./sample/11.cmm
	./parser ./sample/12.cmm
	./parser ./sample/13.cmm
	./parser ./sample/14.cmm
	./parser ./sample/15.cmm
	./parser ./sample/16.cmm
	./parser ./sample/17.cmm
	./parser ./sample/1_.cmm
	./parser ./sample/2_.cmm
	./parser ./sample/3_.cmm
	./parser ./sample/4_.cmm
	./parser ./sample/5_.cmm
	./parser ./sample/6_.cmm
