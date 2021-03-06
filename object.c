#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "gc.h"

extern Object *Nil,*True,*False;
extern Object *Procedure;

Object* new_object(ObjectType type) {
    Object* obj = (Object*) GC_MALLOC(sizeof(Object));
    obj->type = type;
    return obj;
}

void pair_print(Object* pair,int isBegin){
    if(!isBegin){
        printf("(");
        pair_print(pair,1);
    }else{
        Object* left = CAR(pair);
        Object* right = CDR(pair);
        //print left
        if(left->type != OBJ_PAIR){
            obj_print(left);
            printf(" ");
        }else{
            pair_print(left,0);
        }
        //print right
        if(right == Nil || right->type != OBJ_PAIR){
            if(right != Nil){
                printf(" . ");
                obj_print(right);
            }
            printf(") ");
        }else{
            pair_print(right,1);
        }        
    }
}

void obj_print(Object* obj){
    if(obj == NULL) {
        perror("obj is null!");
        exit(1);
    }
    if(obj != Nil){
        switch(obj->type){
            case OBJ_INTEGER:
                printf("%d",obj->value.i);
                break;
            case OBJ_STRING:
                printf("%s",obj->value.s);
                break;
            case OBJ_SYMBOL:
                printf("%s",obj->value.s);
                break;
            case OBJ_PRIMITIVE_PROCEDURE:
                printf("%s","<#primitive>");
                break;
            case OBJ_PAIR:
				/*
				printf("(");
				obj_print(CAR(obj));
				printf(",");
				obj_print(CDR(obj));
				printf(")");
				*/
				if(CAR(obj) == Procedure){
					printf("%s","<#procedure>");
				}else{
					pair_print(obj,0);
				}
                break;
        }
    }else{
        printf("nil");
    }
}

void myprint(Object* obj,char* s){
    printf("\n-------%s-------\n",s);
    obj_print(obj);
    printf("\n-------%s-------\n",s);
}

Object* new_integer(int i){
    Object* obj = new_object(OBJ_INTEGER);
    obj->value.i = i;
    return obj;
}

Object* new_symbol(char* symname){
    Object* obj = new_object(OBJ_SYMBOL);
    obj->value.s = symname;
    return obj;
}

Object* new_primitive_procedure(Object* (*primitive)()){
    Object* obj = new_object(OBJ_PRIMITIVE_PROCEDURE);
    obj->value.primitive_procedure = primitive;
    return obj;
}

Object* new_string(char* s){
    Object* obj = new_object(OBJ_STRING);
    obj->value.s = s;
    return obj;
}

Object* cons(Object* car,Object* cdr){
    Object* pair = new_object(OBJ_PAIR);
    CAR(pair) = car;
    CDR(pair) = cdr;
    return pair;
}

Object* is_empty(Object* list){ return list == Nil ? True : False; }

Object* new_list1(Object* obj){ return cons(obj,Nil); }

Object* new_list2(Object* obj1,Object* obj2){
	return cons(obj1,new_list1(obj2));
}

Object* new_list3(Object* obj1,Object* obj2,Object* obj3){
	return cons(obj1,new_list2(obj2,obj3));
}

Object* new_list4(Object* obj1,Object* obj2,Object* obj3,Object* obj4){
	return cons(obj1,new_list3(obj2,obj3,obj4));
}

int length(Object* list){
    int count=0;
    Object* pair = list;
    while(pair != Nil){
        count++;
        pair = CDR(pair);
    }
    return count;
}

Object* last_pair(Object* list){
    Object* p = list;
    while(CDR(p) != Nil){
        p = CDR(p);
    }
    return p;
}

Object* last(Object* list){
    Object* pair = last_pair(list);
    return CAR(pair);
}

Object* append(Object* list,Object* obj){
    if(list == Nil){
        return cons(obj,Nil);
    }else{
        Object* lastPair = last_pair(list);
        CDR(lastPair) = cons(obj,Nil);
        return list;
    }
}

Object* filter(Object* list,Object* (*func)()){
    Object *p = list;
    Object *from_obj,*to_obj;
    Object *result_list = Nil;
    while(p != Nil){
        from_obj = CAR(p);
        to_obj = (*func)(from_obj);
        if(to_obj == True){
            result_list = append(result_list,to_obj);
        }
        p = CDR(p);
    }
    return result_list;
}

Object* map(Object* list,Object* (*func)()){
    Object *p = list;
    Object *from_obj,*to_obj;
    Object *result_list = Nil;
    while(p != Nil){
        from_obj = CAR(p);
        to_obj = (*func)(from_obj);
        result_list = append(result_list,to_obj);
        p = CDR(p);
    }
    return result_list;
}

Object* eqv(Object* obj1,Object* obj2){
    if(obj1->type == obj2->type){
        switch(obj1->type){
            case OBJ_SYMBOL:
                return strcmp(STR(obj1),STR(obj2)) == 0 ? True : False; 
            case OBJ_INTEGER:
                return INTEGER(obj1) == INTEGER(obj2) ? True : False;
            case OBJ_STRING:
                return strcmp(STR(obj1),STR(obj2)) == 0 ? True : False; 
        }
    }else{
    }
}

