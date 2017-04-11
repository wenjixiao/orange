gc_include = /home/wenjixiao/gc/include
gc_lib = /home/wenjixiao/gc/lib

ops = -g -std=gnu99

#--------------------------------------
interpreter : interpreter.o procedures.o parser.o object.o util.o
	gcc -o $@ $^ -L$(gc_lib) -lm -lgc -static

interpreter.o : interpreter.c util.h object.h parser.h
	gcc $(ops) -c $<

procedures.o : procedures.c procedures.h object.h
	gcc $(ops) -c $<

object.o : object.c object.h
	gcc $(ops) -c $< -I$(gc_include)

util.o : util.c util.h object.h
	gcc $(ops) -c $< -I$(gc_include)

parser.o : parser.c object.h util.h interpreter.h
	gcc $(ops) -c $<

parser.c : parser.l
	flex -o parser.c parser.l 

run: interpreter
	./interpreter example.scm

clean:
	rm *.o
	rm interpreter
#--------------------------------------
hashtable : hashtable.o 
	gcc -o $@ $^ -L$(gc_lib) -lgc

hashtable.o : hashtable.c hashtable.h
	gcc $(ops) -c $< -I$(gc_include)
#--------------------------------------
test : test_scheme.o
	gcc -o $@ $^ -lcheck -lm -lpthread -lrt

test_scheme.o : test_scheme.c
	gcc -c $<
#--------------------------------------
