ops = -g -std=gnu99


interpreter : interpreter.o procedures.o tokens.o vm.o util.o -lm
	gcc -o $@ $^

interpreter.o : interpreter.c token.h util.h vm.h
	gcc $(ops) -c $<

procedures.o : procedures.c procedures.h vm.h
	gcc $(ops) -c $<

vm.o : vm.c vm.h
	gcc $(ops) -c $<

util.o : util.c util.h
	gcc $(ops) -c $<

tokens.o : tokens.c token.h util.h
	gcc $(ops) -c $<

tokens.c : tokens.l
	flex -o tokens.c tokens.l 

test : test.c
	gcc -std=gnu99 test.c -o test -lm

run: interpreter
	./interpreter example.scm

clean:
	rm *.o
	rm interpreter

