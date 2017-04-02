#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char* info){
    printf("error: %s\n",info);
    exit(1);
}

int string_to_number(char* s){
    return atoi(s);
}

char *heap_string(const char *s){
    char *str = (char *)malloc(strlen(s)+1);
    strcpy(str,s);
    return str;
}
