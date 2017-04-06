#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <regex.h>

#include "vm.h"

void myprint(Object* obj,char* s){
    printf("\n-------%s-------\n",s);
    obj_print(obj);
    printf("\n-------%s-------\n",s);
}

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

    int v=0;
    unsigned int myexp=0;
    for(int i=len-1;i>=0;i--){
        v += c2int(p[i],base) * pow(base,myexp++);
    }
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
    char c;
    while(num != 0){
        n = num % base;
        c = int2c(n,base);
        buf[--index] = c;
        num = (num - n) / base;
    }
    strcpy(dest,buf+index);
}

int string_to_int(char* s,int radix){
    regex_t preg;
    const size_t nmatch = 3;
    regmatch_t pmatch[nmatch];

    char* pattern=NULL;
    char* p = s;
    int v=0;
    char buf[20];

    switch(radix){
        case 2:
            pattern = "(\\+|-)?([01]+)";
            break;
        case 8:
            pattern = "(\\+|-)?([0-7]+)";
            break;
        case 10:
            pattern = "(\\+|-)?([0-9]+)";
            break;
        case 16:
            pattern = "(\\+|-)?([0-9a-fA-F]+)";
            break;
    }

    if(regcomp(&preg,pattern,REG_EXTENDED) != 0){
        perror("regex compile error!");
        exit(1);
    }

    if(regexec(&preg,p,nmatch,pmatch,0) != REG_NOMATCH){
        // nums
        int i=0;
        for(int j=pmatch[2].rm_so;j<pmatch[2].rm_eo;j++){
            buf[i++] = p[j];
        }
        buf[i] = '\0';
        v = str2int(buf,radix);
        // +|-
        if(pmatch[1].rm_so != -1){
            if(p[pmatch[1].rm_so] == '-')
                v *= -1;
        }
    }else{
        perror("can't happen!");
        exit(1);
    }
    return v;
}

char *heap_string(const char *s){
    char *str = (char *)malloc(strlen(s)+1);
    strcpy(str,s);
    return str;
}
