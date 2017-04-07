ops = -g -std=gnu99


interpreter : interpreter.o procedures.o parser.o vm.o util.o -lm
	gcc -o $@ $^

interpreter.o : interpreter.c util.h vm.h parser.h
	gcc $(ops) -c $<

procedures.o : procedures.c procedures.h vm.h
	gcc $(ops) -c $<

vm.o : vm.c vm.h
	gcc $(ops) -c $<

util.o : util.c util.h
	gcc $(ops) -c $<

parser.o : parser.c vm.h util.h interpreter.h
	gcc $(ops) -c $<

parser.c : parser.l
	flex -o parser.c parser.l 
#--------------------------------------
#--------------------------------------
test : test.c
	gcc -std=gnu99 test.c -o test -lm

run: interpreter
	./interpreter example.scm

clean:
	rm *.o
	rm interpreter

loop : loop.o /home/wenjixiao/gc/lib/libgc.a
	gcc -o $@ $^

loop.o : loop.c
	gcc $(ops) -I/home/wenjixiao/gc/include -c $<
