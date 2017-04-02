#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "vm.h"

/* parameters is a list */
Object* primitive_add(VM* vm,Object* params){
    int result = 0;
    if(is_list_empty(params)) return newNumberObject(vm,result);

    Object* pair=params;
    while(pair != NULL){
        result += NUMBER(CAR(pair));
        pair = CDR(pair);
    }

    return newNumberObject(vm,result);
}

Object* primitive_sub(VM* vm,Object* params){
    if(is_list_empty(params)) error("params can't be empty!");
    if(list_length(params) == 1){
        int v = NUMBER(CAR(params));
        return newNumberObject(vm,0-v);
    }

    int now = NUMBER(CAR(params));
    for(Object* pair = CDR(params);pair != NULL;pair = CDR(pair)){
        now -= NUMBER(CAR(pair));
    }

    return newNumberObject(vm,now);
}
