#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "object.h"
#include "parser.h"
#include "procedures.h"
#include "interpreter.h"
#include "hashtable.h"

#define M 977

enum {KWD_QUOTE,KWD_SET,KWD_DEFINE,KWD_IF,KWD_LAMBDA,KWD_BEGIN,KWD_COND,NUM_KEYWORDS};

char* Keyword_names[] = {"quote","set!","define","if","lambda","begin","cond"};

Object* Keywords[NUM_KEYWORDS]; // keyword symbol in vm

/* global symbols */
HashTable *Symbols; /* global symbol table */
Object *Primitive; //symbol object for primitive procedure
Object *Procedure;
Object *Lparen;
Object *Rparen;
/* global consts */
Object *Nil; // '()
Object *True;
Object *False;
Object *Void;

/* have,return;not have,create and put in then return it */
Object* make_symbol(char *symname){
	//printf("want make symbol ==%s==\n",symname);
	Object *obj;
	if((obj = hashtable_get(Symbols,symname)) !=  NULL){
		return obj;
	}
	obj = new_symbol(symname);
	hashtable_put(Symbols,symname,obj);
	return obj;
}

void init_consts(){
    /* basic objects */
    Nil = new_object(OBJ_NULL);
    True = new_object(OBJ_BOOLEAN);
    True->value.i = 1;
    False = new_object(OBJ_BOOLEAN);
    False->value.i = 0;
    Void = new_object(OBJ_VOID);
    /* symbols */
    Symbols = hashtable_init(M);
    for(int i=0;i<NUM_KEYWORDS;i++){
		//printf("\n--%d--\n",Keyword_names[i]);
        Keywords[i] = make_symbol(Keyword_names[i]);
		//obj_print(Keywords[i]);
    }
    Primitive = make_symbol("primitive");
    Procedure = make_symbol("procedure");
    Lparen = make_symbol("(");
    Rparen = make_symbol(")");
}
/* env is a list of frame.
 * Frame is a pair,not list */
Object* make_frame(Object* vars,Object* vals){
    return cons(vars,vals);
}

Object* extend_env(Object* vars,Object* vals,Object* base_env){
    if(length(vars) == length(vals)){
        return cons(make_frame(vars,vals),base_env);
    }else{
        perror("vars and vals lengh not match!");
        exit(1);
    }
}
/* no return: so,can't be embed in other exps */
void define_variable(Object* var,Object* val,Object* env){
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
    CAR(first_frame) = append(CAR(first_frame),var);
    CDR(first_frame) = append(CDR(first_frame),val);
}
/* return *void* object */
Object* set_variable_value(Object* var,Object* val,Object* env){
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
Object* make_primitive_procedure(Object* primitive_procedure){
    return new_list2(Primitive,primitive_procedure);
}

void add_primitive_procedure(Object* frame,char* symname,Object* (*func)()){
    Object* sym_obj = make_symbol(symname);
    Object* primitive_procedure = new_primitive_procedure(func);
    Object* func_list = make_primitive_procedure(primitive_procedure);
    
    CAR(frame) = cons(sym_obj,CAR(frame));
    CDR(frame) = cons(func_list,CDR(frame));
}
/* the beginning env */
Object* init_env(){
    Object* frame = cons(Nil,Nil);

    add_primitive_procedure(frame,"+",primitive_add);
    add_primitive_procedure(frame,"-",primitive_sub);
    add_primitive_procedure(frame,">",primitive_gt);

	hashtable_print(Symbols);

    return cons(frame,Nil);
}

Object* obj_eval(Object* obj,Object* env);
Object* obj_apply(Object* obj,Object* params);

Object* sequence_eval(Object* obj,Object* env){
    Object* pair = obj;
    while(CDR(pair) != Nil){
        obj_eval(CAR(pair),env); // not need return value
        pair = CDR(pair);
    }
    return obj_eval(CAR(pair),env);
}

Object* get_assignment_variable(Object* obj){ return CADR(obj); }

Object* get_assignment_value(Object* obj){ return CADDR(obj); }

Object* set_eval(Object* obj,Object* env){
    Object* var = get_assignment_variable(obj);
    Object* val = obj_eval(get_assignment_value(obj),env);
    set_variable_value(var,val,env);
}

Object* make_lambda(Object* parameters,Object* body){
	return cons(Keywords[KWD_LAMBDA],cons(parameters,body));
}

Object* get_definition_variable(Object* obj){
    if(CADR(obj)->type == OBJ_SYMBOL){
        return CADR(obj);
    }else{
        return CAADR(obj);
    }
}

Object* get_definition_value(Object* obj){
    if(CADR(obj)->type == OBJ_SYMBOL){
        return CADDR(obj);
    }else{
        return make_lambda(CDADR(obj),CDDR(obj));
    }
}

Object* define_eval(Object* obj,Object* env){
    Object* val = obj_eval(get_definition_value(obj),env);
    Object* var = get_definition_variable(obj);
    define_variable(var,val,env);
    return Void;
}

Object* get_if_predicate(Object* obj){ return CADR(obj); }

Object* get_if_consequent(Object* obj){ return CADDR(obj); }

Object* get_if_alternative(Object* obj){
    if(CDDDR(obj) != Nil){
        return CADDDR(obj);
    }else{
        return False;
    }
}

Object* if_eval(Object* obj,Object* env){
    Object* result;
    Object* predicate_obj = obj_eval(get_if_predicate(obj),env);

    if(predicate_obj->type == OBJ_BOOLEAN && predicate_obj->value.i == 0){
        //false
        result = obj_eval(get_if_alternative(obj),env);
    }else{
        //true
        result = obj_eval(get_if_consequent(obj),env);
    }
    return result;
}

Object* get_operator(Object* list){ return CAR(list); }

Object* get_operands(Object* list){ return CDR(list); }

Object* list_of_values(Object* operands,Object* env){
    Object* operand_obj;
    Object* args = Nil;

    Object* pair = operands;

    while(pair != Nil){
        operand_obj = obj_eval(CAR(pair),env);
        args = append(args,operand_obj);
        pair = CDR(pair);
    }
    return args;
}

Object* get_lambda_params(Object* obj){ return CADR(obj); }
Object* get_lambda_body(Object* obj){ return CDDR(obj); }

Object* make_procedure(Object* params,Object* body,Object* env){
    return new_list4(Procedure,params,body,env);
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
Object* obj_eval(Object* obj,Object* env){
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
            return make_procedure(get_lambda_params(obj),get_lambda_body(obj),env);
    }else if(is_list_tagged(obj,Keywords[KWD_IF])){
        //if
        return if_eval(obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_QUOTE])){
        //quote
        return CADR(obj);
    }else if(is_list_tagged(obj,Keywords[KWD_SET])){
        //set
        return set_eval(obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_DEFINE])){
        //define
        return define_eval(obj,env);
    }else if(is_list_tagged(obj,Keywords[KWD_BEGIN])){
        //begin
        return sequence_eval(CDR(obj),env);
    }else if(is_list_tagged(obj,Keywords[KWD_COND])){
        //cond
    }else if(is_list_tagged(obj,make_symbol("vector"))){
        //vector
        perror("not supported now!");
        exit(1);
    }else if(obj->type == OBJ_PAIR){
        //applications
        Object* proc = obj_eval(get_operator(obj),env);
        Object* args = list_of_values(get_operands(obj),env);
        return obj_apply(proc,args);
    }else{
        perror("unknow expression type");
        exit(1);
    }
}

int is_primitive_procedure(Object* obj){ return CAR(obj) == Primitive; }
int is_compound_procedure(Object* obj){ return CAR(obj) == Procedure; }

Object* get_procedure_body(Object* obj){ return CADDR(obj); }
Object* get_procedure_parameters(Object* obj){ return CADR(obj); }
Object* get_procedure_env(Object* obj){ return CADDDR(obj); }

Object* get_primitive_procedure(Object* obj){ return CADR(obj); }

/*
 * we have 2 type of procedure,here we only use 'primitive!
 * (list 'primitive <add-func>) 
 * (list 'procedure parameters body env)
 * */
Object* apply_primitive_procedure(Object* obj,Object* args){
    Object* primitiveObj = get_primitive_procedure(obj);
    return (*primitiveObj->value.primitive_procedure)(args);
}

Object* obj_apply(Object* procedure,Object* arguments){
    if(is_primitive_procedure(procedure)){
        return apply_primitive_procedure(procedure,arguments);
    }else if(is_compound_procedure(procedure)){
        Object* procedure_body = get_procedure_body(procedure);
        Object* myparameters = get_procedure_parameters(procedure);
        Object* myenv = get_procedure_env(procedure);
        return sequence_eval(procedure_body,extend_env(myparameters,arguments,myenv));
    }else{
        perror("unknown procedure type!");
        exit(1);
    }
}
