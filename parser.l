%{ 

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "util.h"
#include "vm.h"
#include "interpreter.h"

extern VM* vm;
extern Object* Nil; // '()
extern Object* True;
extern Object* False;
extern Object* Lparen;

static Object* obj;
static Object* parent; // the list readed in
static Object* out_obj;
static int in_stack_count = 0;

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
    obj = newStringObject(vm,heap_string(yytext));
    push(vm,obj),in_stack_count++;
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
    push(vm,obj),in_stack_count++;
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
    obj = make_symbol(vm,heap_string(yytext));
    push(vm,obj),in_stack_count++;
    //printf("quote: %s\n",yytext);
}

{vector} {
    obj = make_symbol(vm,heap_string(yytext));
    push(vm,obj),in_stack_count++;
    printf("vector: %s\n",yytext);
}

{identifier} {
    obj = make_symbol(vm,heap_string(yytext));
    push(vm,obj),in_stack_count++;
    //printf("identifier: %s\n",yytext);
}

{b_integer} {
    obj = newIntegerObject(vm,string_to_int(yytext,2));
    push(vm,obj),in_stack_count++;
    //printf("b_integer: %s\n",yytext);
}

{o_integer} {
    obj = newIntegerObject(vm,string_to_int(yytext,8));
    push(vm,obj),in_stack_count++;
    //printf("o_integer: %s\n",yytext);
}

{d_integer} {
    obj = newIntegerObject(vm,string_to_int(yytext,10));
    push(vm,obj),in_stack_count++;
    //printf("d_integer: %s\n",yytext);
}

{x_integer} {
    obj = newIntegerObject(vm,string_to_int(yytext,16));
    push(vm,obj),in_stack_count++;
    //printf("x_integer: %s\n",yytext);
}

{lp} {
    push(vm,Lparen),in_stack_count++;
    printf("lp: %s\n",yytext);
}

{rp} {
    while(in_stack_count > 0){
        out_obj = pop(vm),in_stack_count--;
        if(out_obj == Lparen){
            if(vm->stackSize > 0 && (vm->stack[vm->stackSize-1] == make_symbol(vm,heap_string("'")))){
                Object* quote_symbol_obj = pop(vm);
                in_stack_count--;
                parent = list2(vm,make_symbol(vm,heap_string("quote")),parent);
            }
            push(vm,parent);
            parent = Nil;
            in_stack_count++;
            break;
        }else if(out_obj == make_symbol(vm,heap_string("#("))){
            Object* vector_symbol_obj = make_symbol(vm,heap_string("vector"));
            parent = cons(vm,vector_symbol_obj,parent);
            push(vm,parent);
            parent = Nil;
            in_stack_count++;
            break;
        }else{
            parent = cons(vm,out_obj,parent);
        }
    }
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
} 

%%

void obj_read(FILE *f){
    parent = Nil;
    yyin = f;
    yylex();
}

int yywrap(){
    return 1;
}

/*
int main(int argc,char** argv){
    FILE *f;
    if(argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    Token *tokens_head = (Token *) get_tokens(f);
    print_tokens();
    return 0;
}
*/
