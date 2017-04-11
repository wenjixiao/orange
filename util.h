#ifndef UTIL_H
#define UTIL_H

#include "object.h"

typedef struct _stack {
    Object* objects[STACK_MAX];
    int size;
} Stack;

Stack* make_stack();
void push(Stack* stack,Object* obj);
Object* pop(Stack* stack);

int string_to_int(char* s,int radix);
char *heap_string(const char *s);

#endif
