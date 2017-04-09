%{ 

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "util.h"
#include "vm.h"
#include "interpreter.h"

extern Object* Nil; // '()
extern Object* True;
extern Object* False;
extern Object* Lparen;

static Object* obj;
static Object* parent; // the list readed in
static Object* out_obj;

Stack* stack;

FILE *f;
int Work_model;

int check_stack_for_return(){
    if(stack->size == 1){
        Object* obj = stack->objects[0];
        if(obj->type == OBJ_PAIR){
            return 1;
        }else if(obj == Lparen){
            return 0;
        }else{
            return 1;
        }
    }else{
        return 0;
    }
}

/*
 * keyword quote|lambda|if|set!|begin|cond|and|or|case|
 * let|let*|letrec|do|delay|quasiquote|else|=>|define|unquote|unquote-splicing
 */

%}

vector              #\(
quote               '
quasiquote          `
comma               ,
unquote_splicing    ,@
dot                 \.
lp                  \(
rp                  \)
character           #\\.
string              \"[^\"\\]*\"
b_integer            ((#e|#i)?#b)|(#b(#e|#i)?)(\+|-)?[01]+
o_integer            ((#e|#i)?#o)|(#o(#e|#i)?)(\+|-)?[0-7]+
d_integer            ((#e|#i)?(#d)?)|((#d)?(#e|#i)?)(\+|-)?[0-9]+
x_integer            ((#e|#i)?#x)|(#x(#e|#i)?)(\+|-)?[0-9a-fA-F]+
boolean             #t|#f
identifier          [a-zA-Z!$%&*/:<=>?^_~][a-zA-Z!$%&*/:<=>?^_~0-9+-@]*|\+|-|\.\.\.
comment             ;.*
whitespace          [ \n\t\r\v\f]+

%%

{character} {
    printf("character: %s\n",yytext);
}

{string} {
    obj = new_string(heap_string(yytext));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("string: %s\n",yytext);
}

{dot} {
    //printf("dot: %s\n",yytext);
}

{boolean} {
    if(strcmp(yytext,"#t")==0){
        obj = True;
    }else if(strcmp(yytext,"#f")==0){
        obj = False;
    }else{
        perror("boolean literal error!");
        exit(1);
    }
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("boolean: %s\n",yytext);
}

{unquote_splicing} {
    //printf("unquote_splicing: %s\n",yytext);
}

{comma} {
    //printf("comma: %s\n",yytext);
}

{quasiquote} {
    //printf("quasiquote: %s\n",yytext);
}

{quote} {
    obj = make_symbol(heap_string(yytext));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("quote: %s\n",yytext);
}

{vector} {
    obj = make_symbol(heap_string(yytext));
    push(stack,obj);
    printf("vector: %s\n",yytext);
    if(check_stack_for_return()) return 1;
}

{identifier} {
    obj = make_symbol(heap_string(yytext));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("identifier: %s\n",yytext);
}

{b_integer} {
    obj = new_integer(string_to_int(yytext,2));
    push(stack,obj);
    //printf("b_integer: %s\n",yytext);
    if(check_stack_for_return()) return 1;
}

{o_integer} {
    obj = new_integer(string_to_int(yytext,8));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("o_integer: %s\n",yytext);
}

{d_integer} {
    obj = new_integer(string_to_int(yytext,10));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("d_integer: %s\n",yytext);
}

{x_integer} {
    obj = new_integer(string_to_int(yytext,16));
    push(stack,obj);
    if(check_stack_for_return()) return 1;
    //printf("x_integer: %s\n",yytext);
}

{lp} {
    push(stack,Lparen);
    //printf("lp: %s\n",yytext);
}

{rp} {
    while(stack->size > 0){
        out_obj = pop(stack);
        if(out_obj == Lparen){
            if(stack->size > 0 && (stack->objects[stack->size-1] == make_symbol(heap_string("'")))){
                Object* quote_symbol_obj = pop(stack);
                parent = list2(make_symbol(heap_string("quote")),parent);
            }
            push(stack,parent);
            parent = Nil;
            break;
        }else if(out_obj == make_symbol(heap_string("#("))){
            Object* vector_symbol_obj = make_symbol(heap_string("vector"));
            parent = cons(vector_symbol_obj,parent);
            push(stack,parent);
            parent = Nil;
            break;
        }else{
            parent = cons(out_obj,parent);
        }
    }
    if(check_stack_for_return()) return 1;
    //printf("rp: %s\n",yytext);
}

{comment} {
    printf("comment: %s\n",yytext);
}

{whitespace} {
    //printf("whitespace: %s\n",yytext);
}

. {
    printf("other_things: %s\n",yytext);
    exit(1);
} 

%%

void obj_read(FILE *f){
    parent = Nil;
    yyin = f;
    Object *o,*result;
    Object *env = init_env();

    printf(">");
    while(yylex()){
        o = pop(stack);
        result = obj_eval(o,env);
        //printf("\n");
        obj_print(result);
        if(Work_model == 1){
            printf("\n>");
        }
    }
    printf("\n-----obj_read returned-----\n");
}

int yywrap(){
    return 1;
}

int main(int argc,char** argv){
    if(argc > 1){
        Work_model = 0;
        f = fopen(argv[1], "r");
    }else{
        Work_model = 1;
        f = stdin;
    }

    stack = make_stack();
    init_consts();
    obj_read(f);
    /*
    obj_read(f);
    Object* o = pop(stack);
    printf("exp: ");
    obj_print(o);
    Object* env = init_env();
    Object* r = obj_eval(o,env);
    printf("\n>>>");
    obj_print(r);
    printf("\n");
    */

    fclose(f);
    return 0;
}
