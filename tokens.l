%{ 

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "util.h"
#include "token.h"

Token *head=NULL,*tail=NULL;

Token * make_token(enum token_type type,char *text){
    Token *t;
    t = (Token *) malloc(sizeof(Token));
    t->next = NULL;
    t->text = heap_string(text);
    t->type = type;
    return t;
}

void append_token(Token *t){
    if(head==NULL){
        head=t;
        tail=t;
    }else{
        tail->next=t;
        tail=t;
    }
}

void print_tokens(){
    Token *p=head;
    while(p!=NULL){
        printf("type=%d,text=%s\n",p->type,p->text);
        p=p->next;
    }
}

/*
 * keyword quote|lambda|if|set!|begin|cond|and|or|case|
 * let|let*|letrec|do|delay|quasiquote|else|=>|define|unquote|unquote-splicing
 * (((#e|#i)?(#b)?)|((#b)?(#e|#i)?))(\+|-)?[01]+
 * (((#e|#i)?(#o)?)|((#o)?(#e|#i)?))(\+|-)?[0-7]+
 * (((#e|#i)?(#d)?)|((#d)?(#e|#i)?))(\+|-)?[0-9]+
 * (((#e|#i)?(#x)?)|((#x)?(#e|#i)?))(\+|-)?[0-9a-fA-F]+
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
    append_token(make_token(STRING,yytext));
    printf("string: %s\n",yytext);
}

{dot} {
    printf("dot: %s\n",yytext);
}

{boolean} {
    append_token(make_token(BOOLEAN,yytext));
    printf("boolean: %s\n",yytext);
}

{unquote_splicing} {
    printf("unquote_splicing: %s\n",yytext);
}

{comma} {
    printf("comma: %s\n",yytext);
}

{quasiquote} {
    printf("quasiquote: %s\n",yytext);
}

{quote} {
    printf("quote: %s\n",yytext);
}

{vector} {
    printf("vector: %s\n",yytext);
}

{identifier} {
    append_token(make_token(IDENTIFIER,yytext));
    printf("identifier: %s\n",yytext);
}

{b_integer} {
    append_token(make_token(INTEGER2,yytext));
    printf("b_integer: %s\n",yytext);
}

{o_integer} {
    append_token(make_token(INTEGER8,yytext));
    printf("o_integer: %s\n",yytext);
}

{d_integer} {
    append_token(make_token(INTEGER10,yytext));
    printf("d_integer: %s\n",yytext);
}

{x_integer} {
    append_token(make_token(INTEGER16,yytext));
    printf("x_integer: %s\n",yytext);
}

{lp} {
    append_token(make_token(LP,yytext));
    printf("lp: %s\n",yytext);
}

{rp} {
    append_token(make_token(RP,yytext));
    printf("rp: %s\n",yytext);
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

Token *get_tokens(FILE *f){
    yyin = f;
    yylex();
    return head;
}

void destroy_tokens(Token* head){
    Token *p=head,*q=NULL;
    while(p != NULL) {
        free(p->text);
        q = p;
        p=p->next;
        free(q);   
    }
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
