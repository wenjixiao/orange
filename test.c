#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <math.h>
#include <ctype.h>

/* m is the length of hash table => the target space size */
unsigned int BKDRHash(char* str,unsigned int m)
{
    unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
    unsigned int hash = 0;
    unsigned int i   = 0;
    
    while(*str){
        hash = (hash * seed) + (*str++);
    }
    return hash % m;
}

int main(){
    char *s = "hello world!";
    unsigned int i = BKDRHash(s);
    printf("%d\n",i);
    return 0;
}
