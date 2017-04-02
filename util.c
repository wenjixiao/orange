#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char* info){
    printf("error: %s\n",info);
    exit(1);
}
/*
 * (((#e|#i)?#b)|(#b(#e|#i)?))(\+|-)?[01]+
 * (((#e|#i)?#o)|(#o(#e|#i)?))(\+|-)?[0-7]+
 * (((#e|#i)?(#d)?)|((#d)?(#e|#i)?))(\+|-)?[0-9]+
 * (((#e|#i)?#x)|(#x(#e|#i)?))(\+|-)?[0-9a-fA-F]+
 */
int string_to_number(char* s){
    return atoi(s);
}

char *heap_string(const char *s){
    char *str = (char *)malloc(strlen(s)+1);
    strcpy(str,s);
    return str;
}
