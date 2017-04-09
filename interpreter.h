#ifndef INTERPRETER_H
#define INTERPRETER_H

void init_consts();
Object* init_env();
Object* make_symbol(const char* symname);
Object* obj_eval(Object* obj,Object* env);

#endif

