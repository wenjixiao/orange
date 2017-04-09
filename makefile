gc_include_path = /home/wenjixiao/gc/include
gc_lib = /home/wenjixiao/gc/lib/libgc.a

ops = -g -std=gnu99


interpreter : interpreter.o procedures.o parser.o vm.o util.o $(gc_lib) -lm 
	gcc -o $@ $^

interpreter.o : interpreter.c util.h vm.h parser.h
	gcc $(ops) -c $<

procedures.o : procedures.c procedures.h vm.h
	gcc $(ops) -c $<

vm.o : vm.c vm.h
	gcc $(ops) -I$(gc_include_path) -c $<

util.o : util.c util.h
	gcc $(ops) -c $<

parser.o : parser.c vm.h util.h interpreter.h
	gcc $(ops) -c $<

parser.c : parser.l
	flex -o parser.c parser.l 
#--------------------------------------
hashtable : hashtable.o $(gc_lib)
	gcc -o $@ $^

hashtable.o : hashtable.c hashtable.h
	gcc $(ops) -I$(gc_include_path) -c $<
#--------------------------------------
test : test.c
	gcc -std=gnu99 test.c -o test -lm

run: interpreter
	./interpreter example.scm

clean:
	rm *.o
	rm interpreter
#/home/wenjixiao/gc/lib/libgc.a
loop : loop.o $(gc_lib)
	gcc -o $@ $^

loop.o : loop.c
	gcc $(ops) -I$(gc_include_path) -c $<
