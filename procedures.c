#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "vm.h"

extern Object *Nil,*True,*False;

/* parameters is a list */
/* + */
Object* primitive_add(VM* vm,Object* params){
    int result = 0;
    if(params == Nil) return newIntegerObject(vm,result);

    Object* pair=params;
    while(pair != Nil){
        result += INTEGER(CAR(pair));
        pair = CDR(pair);
    }

    return newIntegerObject(vm,result);
}
/* - */
Object* primitive_sub(VM* vm,Object* params){
    if(params == Nil) {
        perror("params can't be empty!");
        exit(1);
    }
    if(length(params) == 1){
        int v = INTEGER(CAR(params));
        return newIntegerObject(vm,0-v);
    }

    int now = INTEGER(CAR(params));
    for(Object* pair = CDR(params);pair != Nil;pair = CDR(pair)){
        now -= INTEGER(CAR(pair));
    }

    return newIntegerObject(vm,now);
}
/* > */
Object* primitive_gt(VM* vm,Object* params){
    Object* left_obj = CAR(params);
    Object* right_obj;
    Object* pair = CDR(params);
    while(pair != Nil){
        right_obj = CAR(pair);
        if(left_obj->value.i > right_obj->value.i){
            pair = CDR(pair);
        }else{
            return False;
        }
    }
    return True;
}
/* < */
Object* primitive_lt(VM* vm,Object* params){
}
/* = */
Object* primitive_eq(VM* vm,Object* params){
}
/* != */
Object* primitive_not_eq(VM* vm,Object* params){
}
/* >= */
Object* primitive_gt_eq(VM* vm,Object* params){
}
/* <= */
Object* primitive_lt_eq(VM* vm,Object* params){
}
