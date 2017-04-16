%{ 

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "util.h"
#include "object.h"
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
unquoted            ,
unquote_splicing    ,@
dot                 \.
lp                  \(
rp                  \)
character           #\\.|#\\space|#\\newline
string              \"[^\"\\]*\"
digit2				[01]
digit8				[0-7]
digit10				[0-9]
digit16				[0-9a-fA-F]
radix2				#b
radix8				#o
radix10				(#d)?
radix16				#x
sign				(\+|-)?
exactness			(#e|#i)?
b_integer           ({exactness}#b)|(#b{exactness}){sign}{digit2}+
o_integer           ({exactness}#o)|(#o{exactness}){sign}[digit8]+
d_integer           ({exactness}(#d)?)|((#d)?{exactness}){sign}{digit10}+
x_integer           ({exactness}#x)|(#x{exactness}){sign}{digit16}+
prefix2				({radix2}{exactness})|({exactness}{radix2})
prefix8				({radix8}{exactness})|({exactness}{radix8})
prefix10			({radix10}{exactness})|({exactness}{radix10})
prefix16			({radix16}{exactness})|({exactness}{radix16})
complex2			{real2}|({real2}@{real2})|({real2}\+{ureal2}i)|({real2}-{ureal2}i)|({real2}\+i)|({real2}-i)|(\+{ureal2}i)|(-{ureal2}i)|(\+i)|(-i)
complex8			{real8}|({real8}@{real8})|({real8}\+{ureal8}i)|({real8}-{ureal8}i)|({real8}\+i)|({real8}-i)|(\+{ureal8}i)|(-{ureal8}i)|(\+i)|(-i)
complex10			{real10}|({real10}@{real10})|({real10}\+{ureal10}i)|({real10}-{ureal10}i)|({real10}\+i)|({real10}-i)|(\+{ureal10}i)|(-{ureal10}i)|(\+i)|(-i)
complex16			{real16}|({real16}@{real16})|({real16}\+{ureal16}i)|({real16}-{ureal16}i)|({real16}\+i)|({real16}-i)|(\+{ureal16}i)|(-{ureal16}i)|(\+i)|(-i)
uinteger2			{digit2}+#*			
uinteger8			{digit8}+#*			
uinteger10			{digit10}+#*			
uinteger16			{digit16}+#*			
exponent_marker		e|s|f|d|l
suffix				({exponent_marker}{sign}{digit10}+)?
decimal10			({uinteger10}{suffix})|(\.{digit10}+#*{suffix})|({digit10}+\.{digit10}*#*{suffix})|({digit10}+#+\.#*{suffix})
ureal2				{uinteger2}|({uinteger2}\/{uinteger2})
ureal8				{uinteger8}|({uinteger8}\/{uinteger8})
ureal10				{uinteger10}|({uinteger10}\/{uinteger10})|{decimal10}
ureal16				{uinteger16}|({uinteger16}\/{uinteger16})
real2				{sign}{ureal2}
real8				{sign}{ureal8}
real10				{sign}{ureal10}
real16				{sign}{ureal16}
number2				{prefix2}{complex2}
number8				{prefix8}{complex8}
number10			{prefix10}{complex10}
number16			{prefix16}{complex16}
number				{number2}|{number8}|{number10}|{number16}
boolean             #t|#f
letter				[a-zA-Z]
digit				[0-9]
special_initial		!|\$|%|&|\*|\/|:|<|=|>|\?|\^|_|~
initial				{letter}|{special_initial}
special_subsequent	\+|-|\.|@
subsequent			{initial}|{digit}|{special_subsequent}
peculiar_identifier \+|-|\.\.\.
identifier          ({initial}{subsequent}*)|{peculiar_identifier}
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

{unquoted} {
    //printf("unquoted: %s\n",yytext);
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
                parent = new_list2(make_symbol(heap_string("quote")),parent);
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
    return -1; /* maybe go back to obj_read,do some process */
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
        if(Work_model == 0){
            printf("\n");
        }
        obj_print(result);
        if(Work_model == 1){
            printf("\n>");
        }
    }
    printf("\n");
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
    fclose(f);
    return 0;
}
