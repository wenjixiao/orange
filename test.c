#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <ctype.h>

#include "util.h"

/* char -> int 
 * '0' 48 '9' 57
 * 'a' 97 'f' 102 'z' 122
 * 'A' 65 'F' 70  'Z' 90
 * */
int c2int(char c,unsigned int base){
    int base_0 = 48;
    int base_a = 97;
    int base_A = 65;

    int result;
    if(isdigit(c)){
        result = c - base_0;
    }else if(isalpha(c)){
        int mybase = isupper(c) ? base_A : base_a;
        result = c - mybase + 10;
    }else{
        perror("just support number and alpha!");
        exit(1);
    }
    if(result < base) {
        return result;
    }else{
        perror("number can't bigger than base!");
        exit(1);
    }
}
/* string -> int 
 * 234 = 2*10^2 + 3*10^1 + 4*10^0 */
int str2int(char* s,unsigned int base){
    size_t len = strlen(s);
    char* p = s;

    int flag; /* flag=0 no prefix,flag=1 prefix=+,flag=-1 prefix=- */
    if(p[0] == '+'){
        flag = 1;
    }else if(p[0] == '-'){
        flag = -1;
    }else{
        flag = 0;
    }

    int v=0;
    unsigned int myexp=0;
    int num_index = flag == 0? 0 : 1;
    for(int i=len-1;i>=num_index;i--){
        v += c2int(p[i],base) * pow(base,myexp++);
    }

    if(flag < 0) v *= -1;
    return v;
}
/* 0->9=48->57
 * a->f=97->102
 * A->F=65->70
 */
char int2c(int i,int base){
    if(i < base){
        char result;
        if(i<10){
            result = 48+i;
        }else{
            result = 97+i-10;
        }
        return result;
    }else{
        perror("i can't >= base!");
        exit(1);
    }
}

/* 
 * 234 = 2*10^2 + 3*10^1 + 4*10^0 
 *
 * num      n
 * 234 % 10=4
 * 23  % 10=3
 * 2   % 10=2
 *
 * num = (num - n) / 10;
 * n = num % 10
 *
 */
void int2str(char* dest,int i,int base){
    /* int is 32bit,the biggest is 2147483647,about 10 length
     * so the buf is big enougth */
    char buf[20]; 
    buf[19] = '\0';
    int index = 19;
    int num=i,n;
    num = i<0? -1*i : i;
    char c;
    while(num != 0){
        n = num % base;
        c = int2c(n,base);
        buf[--index] = c;
        num = (num - n) / base;
    }
    if(i<0){
        buf[--index]='-';
    }
    strcpy(dest,buf+index);
}

int test_regex(){
    char* b_pattern = "(((#e|#i)?#b)|(#b(#e|#i)?))(\\+|-)?[01]+";   
    char* o_pattern = "(((#e|#i)?#o)|(#o(#e|#i)?))(\\+|-)?[0-7]+";   
    char* d_pattern = "(((#e|#i)?(#d)?)|((#d)?(#e|#i)?))(\\+|-)?[0-9]+";   
    char* x_pattern = "(((#e|#i)?#x)|(#x(#e|#i)?))(\\+|-)?[0-9a-fA-F]+";   
    char* buf = "hello#b111000";
    regex_t preg;
    const size_t nmatch = 2;
    regmatch_t pmatch[nmatch];
    if(regcomp(&preg,b_pattern,REG_EXTENDED) != 0){
        perror("regcomp");
        exit(1);
    }
    int status,i;
    status = regexec(&preg,buf,nmatch,pmatch,0);
    if(status == REG_NOMATCH){
        //no match
        printf("no match\n");
    }
    if(status == 0){
        //match
        printf("match!\n");
        printf("so=%d,eo=%d\n",pmatch[0].rm_so,pmatch[0].rm_eo);
        for(int i=pmatch[0].rm_so;i<pmatch[0].rm_eo;i++){
            printf("%c",buf[i]);
        }
        printf("\n");
    }
    regfree(&preg);
}

int main(){
    char my[20];
    int2str(my,-255,16);
    printf("==%s\n",my);
    //test_regex();
    /*
    char c;
    c = int2c(7,8);
    printf("%c\n",c);
    */
}
