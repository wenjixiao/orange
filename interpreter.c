#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "vm.h"
#include "parser.h"
#include "procedures.h"


enum {KWD_QUOTE,KWD_SET,KWD_DEFINE,KWD_IF,KWD_LAMBDA,KWD_BEGIN,KWD_COND,NUM_KEYWORDS};

const char* Keyword_names[] = {"quote","set!","define","if","lambda","begin","cond"};

Object* Keywords[NUM_KEYWORDS]; // keyword symbol in vm

/* global symbols */
Object* Symbols; /* global symbol table */
Object* Primitive; //symbol object for primitive procedure
Object* Procedure;
Object* Lparen;
Object* Rparen;
/* global consts */
Object* Nil; // '()
Object* True;
Object* False;
Object* Void;
/* global list for gc */
Object* Consts; // as a root for gc mark

VM* vm;

/*
 * find or generate a symbol
 * maybe the symbol table should depends on env,
 * and different scope have different symtable
 * i don't know now
 */
Object* make_symbol(VM* vm,const char* symname){
    Object* obj;
    Object* pair = Symbols;
    if(Symbols != Nil){
        while(pair != Nil){
            obj= CAR(pair);
            if(strcmp(obj->value.s,symname) == 0){
                return obj;
            }
            pair=CDR(pair);
        }
    }
    //not found 
    Object* newObj = newSymbolObject(vm,heap_string(symname));
    Symbols = append(vm,Symbols,newObj);
    return newObj;
}

void init_consts(VM* vm){
    /* basic objects */
    Nil = newObject(vm,OBJ_PAIR);
    CAR(Nil) = NULL;
    CDR(Nil) = NULL;
    True = newObject(vm,OBJ_BOOLEAN);
    True->value.i = 1;
    False = newObject(vm,OBJ_BOOLEAN);
    False->value.i = 0;
    Void = newObject(vm,OBJ_VOID);
    /* symbols */
    Symbols = Nil;
    for(int i=0;i<NUM_KEYWORDS;i++){
        Keywords[i] = make_symbol(vm,Keyword_names[i]);
    }
    Primitive = make_symbol(vm,"primitive");
    Procedure = make_symbol(vm,"procedure");
    Lparen = make_symbol(vm,"(");
    Rparen = make_symbol(vm,")");
    /* add them to global_const_list */
    Consts = Nil;
    Consts = list4(vm,Nil,True,False,Void);
    Consts = cons(vm,Symbols,Consts);
}
/* env is a list of frame.
 * Frame is a pair,not list */
Object* make_frame(VM* vm,Object* vars,Object* vals){
    return cons(vm,vars,vals);
}

Object* extend_env(VM* vm,Object* vars,Object* vals,Object* base_env){
    if(length(vars) == length(vals)){
        return cons(vm,make_frame(vm,vars,vals),base_env);
    }else{
        perror("vars and vals lengh not match!");
        exit(1);
    }
}
/* no return: so,can't be embed in other exps */
void define_variable(VM* vm,Object* var,Object* val,Object* env){
    Object* first_frame = CAR(env);
    Object* var_pair = CAR(first_frame); //vars list
    Object* val_pair = CDR(first_frame); //vals list

    while(var_pair != Nil){
        if(var == CAR(var_pair)){
            //found
            CAR(val_pair) = val;
            return;
        }
        var_pair = CDR(var_pair);
        val_pair = CDR(val_pair);
    }
    //not found,add new binding
    CAR(first_frame) = append(vm,CAR(first_frame),var);
    CDR(first_frame) = append(vm,CDR(first_frame),val);
}
/* return *void* object */
Object* set_variable_value(VM* vm,Object* var,Object* val,Object* env){
    Object* frame_list = env; //frame list
    Object* myframe_pair;
    Object* var_list; //variables list
    Object* val_list; //values list
    Object* myvar;
    Object* myval;
    while(frame_list != Nil){
        myframe_pair = CAR(frame_list); //the frame we get!
        var_list = CAR(myframe_pair); //list of vars
        val_list = CDR(myframe_pair); //list of vals
        while(var_list != Nil){
            myvar = CAR(var_list); //the var we get!
            if(myvar == var){
                CAR(val_list) = val;
                return Void;
            }
            var_list = CDR(var_list); //next var
            val_list = CDR(val_list); //next val
        }
        frame_list = CDR(frame_list); //next frame
    }
    //not found
    perror("unbound variable---SET!");
    exit(1);
}
/*
 * env is a list of frame
 * frame is a pair of vals and vals
 * vars is a list of symbol
 * vals is a list of Object
 * */
/*
void hehe(VM* vm,Object* var,Object* val,Object* env){
    Object* frame_list = env; //frame list
    Object* myframe_pair;
    Object* var_list; //variables list
    Object* val_list; //values list
    Object* myvar;
    Object* myval;
    while(frame_list != Nil){
        myframe_pair = CAR(frame_list); //the frame we get!
        var_list = CAR(myframe_pair); //list of vars
        val_list = CDR(myframe_pair); //list of vals
        while(var_list != Nil){
            myvar = CAR(var_list); //the var we get!
            myval = CAR(val_list); //the val we get!
            var_list = CDR(var_list); //next var
            val_list = CDR(val_list); //next val
        }
        frame_list = CDR(frame_list); //next frame
    }
}
*/
Object* lookup_variable_value(Object* var,Object* env){
    Object* frame_list = env; //frame list
    Object* myframe_pair;
    Object* var_list; //variables list
    Object* val_list; //values list
    Object* myvar;
    Object* myval;
    while(frame_list != Nil){
        myframe_pair = CAR(frame_list); //the frame we get!
        var_list = CAR(myframe_pair); //list of vars
        val_list = CDR(myframe_pair); //list of vals
        while(var_list != Nil){
            myvar = CAR(var_list); //the var we get!
            if(myvar == var){
                myval = CAR(val_list); //the val we get!
                return myval;
            }
            var_list = CDR(var_list); //next var
            val_list = CDR(val_list); //next val
        }
        frame_list = CDR(frame_list); //next frame
    }
    //not found
    perror("unbound variable---lookup!");
    exit(1);
}
/* func -> (list 'primitive func) */
Object* make_primitive_procedure(VM* vm,Object* primitive_procedure){
    return list2(vm,Primitive,primitive_procedure);
}

void add_primitive_procedure(VM* vm,Object* frame,char* symname,Object* (*func)()){
    Object* sym_obj = make_symbol(vm,symname);
    Object* primitive_procedure = newPrimitiveProcedure(vm,func);
    Object* func_list = make_primitive_procedure(vm,primitive_procedure);
    
    CAR(frame) = cons(vm,sym_obj,CAR(frame));
    CDR(frame) = cons(vm,func_list,CDR(frame));
}
/* the beginning env */
Object* init_env(VM* vm){
    Object* frame = cons(vm,Nil,Nil);

    add_primitive_procedure(vm,frame,"+",primitive_add);
    add_primitive_procedure(vm,frame,"-",primitive_sub);
    add_primitive_procedure(vm,frame,">",primitive_gt);

    return cons(vm,frame,Nil);
}

Object* obj_eval(VM* vm,Object* obj,Object* env);
Object* obj_apply(VM* vm,Object* obj,Object* params);

Object* sequence_eval(VM* vm,Object* obj,Object* env){
    Object* pair = obj;
    while(CDR(pair) != Nil){
        obj_eval(vm,CAR(pair),env); // not need return value
        pair = CDR(pair);
    }
    return obj_eval(vm,CAR(pair),env);
}

Object* get_assignment_variable(Object* obj){ return CADR(obj); }

Object* get_assignment_value(Object* obj){ return CADDR(obj); }

Object* set_eval(VM* vm,Object* obj,Object* env){

}

Object* make_lambda(VM* vm,Object* parameters,Object* body){
    return list3(vm,Keywords[KWD_LAMBDA],parameters,body);
}

Object* get_definition_variable(Object* obj){
    if(CADR(obj)->type == OBJ_SYMBOL){
        return CADR(obj);
    }else{
        return CAADR(obj);
    }
}

Object* get_definition_value(VM* vm,Object* obj){
    if(CADR(obj)->type == OBJ_SYMBOL){
        return CADDR(obj);
    }else{
        return make_lambda(vm,CDADR(obj),CADDR(obj));
    }
}

void define_eval(VM* vm,Object* obj,Object* env){
    push(vm,env);
    push(vm,obj);
    Object* val = obj_eval(vm,get_definition_value(vm,obj),env);
    pop(vm);
    pop(vm);
    Object* var = get_definition_variable(obj);
    define_variable(vm,var,val,env);
}

Object* get_if_predicate(Object* obj){ return CADR(obj); }

Object* get_if_consequent(Object* obj){ return CADDR(obj); }

Object* get_if_alternative(VM* vm,Object* obj){
    if(CDDDR(obj) != Nil){
        return CADDDR(obj);
    }else{
        return False;
    }
}

Object* if_eval(VM* vm,Object* obj,Object* env){
    Object* result;

    push(vm,obj);
    push(vm,env);
    Object* predicate_obj = obj_eval(vm,get_if_predicate(obj),env);
    pop(vm);
    pop(vm);

    if(predicate_obj->type == OBJ_BOOLEAN && predicate_obj->value.i == 0){
        //false
        result = obj_eval(vm,get_if_alternative(vm,obj),env);
    }else{
        //true
        result = obj_eval(vm,get_if_consequent(obj),env);
    }
    return result;
}

Object* get_operator(Object* list){ return CAR(list); }

Object* get_operands(Object* list){ return CDR(list); }

Object* list_of_values(VM* vm,Object* operands,Object* env){
    Object* operand_obj;
    Object* args = Nil;

    Object* pair = operands;

    push(vm,args); // save
    while(pair != Nil){
        operand_obj = obj_eval(vm,CAR(pair),env);
        args = append(vm,args,operand_obj);
        pair = CDR(pair);
    }
    pop(vm); //restore
    return args;
}

Object* get_lambda_params(Object* obj){ return CADR(obj); }
Object* get_lambda_body(Object* obj){ return CDDR(obj); }

Object* make_procedure(VM* vm,Object* params,Object* body,Object* env){
    return list4(vm,Procedure,params,body,env);
}

int is_list_tagged(Object* list,Object* symbol){
    if(list->type == OBJ_PAIR){
        Object* first = CAR(list);
        if(first == symbol){
            return 1;
        }           
    }
    return 0;
}
//enum {KWD_QUOTE,KWD_SET,KWD_DEFINE,KWD_IF,KWD_LAMBDA,KWD_BEGIN,KWD_COND,NUM_KEYWORDS};
Object* obj_eval(VM* vm,Object* obj,Object* env){
    //myprint(obj,"eval obj");
    if(obj->type == OBJ_INTEGER || obj->type == OBJ_STRING 
            || obj->type == OBJ_CHARACTER || obj->type == OBJ_BOOLEAN){
        //self evaluating
        return obj;
    }else if(obj->type == OBJ_SYMBOL){
        //variable
        return lookup_variable_value(obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_LAMBDA])){
        //lambda
            push(vm,env);
            push(vm,obj);
            Object* myobj = make_procedure(vm,get_lambda_params(obj),get_lambda_body(obj),env);
            pop(vm);
            pop(vm);
            return myobj;
    }else if(is_list_tagged(obj,Keywords[KWD_IF])){
        //if
        return if_eval(vm,obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_QUOTE])){
        //quote
    }else if(is_list_tagged(obj,Keywords[KWD_SET])){
        //set
        return set_eval(vm,obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_DEFINE])){
        //define
        define_eval(vm,obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_BEGIN])){
        //begin
        return sequence_eval(vm,CDR(obj),env);
    }else if(is_list_tagged(obj,Keywords[KWD_COND])){
        //cond
    }else if(obj->type == OBJ_PAIR){
        //applications
        push(vm,env);
        push(vm,obj);
        Object* proc = obj_eval(vm,get_operator(obj),env);
        push(vm,proc);
        Object* args = list_of_values(vm,get_operands(obj),env);
        pop(vm);
        pop(vm);
        pop(vm);
        Object* result = obj_apply(vm,proc,args);
        return result;
    }else{
        perror("unknow expression type");
        exit(1);
    }
}

int is_primitive_procedure(VM* vm,Object* obj){ return CAR(obj) == Primitive; }
int is_compound_procedure(VM* vm,Object* obj){ return CAR(obj) == Procedure; }

Object* get_procedure_body(Object* obj){ return CADDR(obj); }
Object* get_procedure_parameters(Object* obj){ return CADR(obj); }
Object* get_procedure_env(Object* obj){ return CADDDR(obj); }

Object* get_primitive_procedure(Object* obj){ return CADR(obj); }

/*
 * we have 2 type of procedure,here we only use 'primitive!
 * (list 'primitive <add-func>) 
 * (list 'procedure parameters body env)
 * */
Object* apply_primitive_procedure(VM* vm,Object* obj,Object* args){
    Object* primitiveObj = get_primitive_procedure(obj);
    return (*primitiveObj->value.primitive_procedure)(vm,args);
}

Object* obj_apply(VM* vm,Object* procedure,Object* arguments){
    if(is_primitive_procedure(vm,procedure)){
        return apply_primitive_procedure(vm,procedure,arguments);
    }else if(is_compound_procedure(vm,procedure)){
        Object* procedure_body = get_procedure_body(procedure);
        Object* myparameters = get_procedure_parameters(procedure);
        Object* myenv = get_procedure_env(procedure);
        return sequence_eval(vm,procedure_body,extend_env(vm,myparameters,arguments,myenv));
    }else{
        perror("unknown procedure type!");
        exit(1);
    }
}

Object* num(VM* vm,int i){
    return newIntegerObject(vm,i);
}
/*
void test_print(VM* vm){
    Object* n1 = num(vm,1);
    Object* n2 = num(vm,2);
    Object* n3 = num(vm,3);
    Object* n4 = num(vm,4);
    Object* n5 = num(vm,5);
    Object* n6 = num(vm,6);

    Object* c12 = cons(vm,n1,n2);
    Object* c34 = cons(vm,n3,n4);
    Object* cc = cons(vm,c12,c34);

    Object* l1 = list4(vm,n1,n2,n3,n4);
    printf("---------------\n");
    printObject(l1);
    printf("\n");
    printObject1(l1);
    printf("\n");
    printf("---------------\n");
    printObject(cc);
    printf("\n");
    printObject1(cc);
    printf("\n");
}
*/

int main(int argc,char** argv){
    FILE *f;
    if(argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    vm = newVM();
    init_consts(vm);
    Object* env = init_env(vm);
    obj_read(f);
    Object* o = pop(vm);
    printf("exp: ");
    obj_print(o);
    Object* r = obj_eval(vm,o,env);
    printf("\n>>>");
    obj_print(r);
    printf("\n");

    freeVM(vm);

    fclose(f);
    return 0;
}
