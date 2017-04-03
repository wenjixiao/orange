#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "util.h"
#include "vm.h"
#include "procedures.h"

Object* Symbols; /* global symbol table */

enum {KWD_QUOTE,KWD_SET,KWD_DEFINE,KWD_IF,KWD_LAMBDA,KWD_BEGIN,KWD_COND,NUM_KEYWORDS};

const char* keyword_names[] = {"quote","set!","define","if","lambda","begin","cond"};

Object* keywords[NUM_KEYWORDS];

Object *true,*false,*primitive,*procedure;

/*
 * find or generate a symbol
 * maybe the symbol table should depends on env,
 * and different scope have different symtable
 * i don't know now
 */
Object* make_symbol(VM* vm,const char* symname){
    Object* obj;
    Object* pair = Symbols;
    if(!is_list_empty(Symbols)){
        while(pair != NULL){
            obj= CAR(pair);
            if(strcmp(obj->value.s,symname) == 0){
                return obj;
            }
            pair=CDR(pair);
        }
    }
    //not found 
    Object* newObj = newSymbolObject(vm,heap_string(symname));
    list_append_obj(vm,Symbols,newObj);
    return newObj;
}

void init_symbols(VM* vm){
    Symbols = make_empty_list(vm);
    primitive = make_symbol(vm,"primitive");
    procedure = make_symbol(vm,"procedure");
    for(int i=0;i<NUM_KEYWORDS;i++){
        keywords[i] = make_symbol(vm,keyword_names[i]);
    }

    push(vm,Symbols); // can't be gc now
}

/* env is a list of frame.
 * Frame is a pair,not list */
Object* make_frame(VM* vm,Object* vars,Object* vals){
    return cons(vm,vars,vals);
}

Object* extend_env(VM* vm,Object* vars,Object* vals,Object* base_env){
    if(list_length(vars) == list_length(vals)){
        return cons(vm,make_frame(vm,vars,vals),base_env);
    }else{
        perror("vars and vals lengh not match!");
        exit(1);
    }
}

Object* scan_in_frame(Object* frame,Object* var){
    Object* frame_var;
    Object* vars = CAR(frame);
    Object* vals = CDR(frame);
    while(vars != NULL && vals != NULL){
        frame_var = CAR(vars);
        if(var == frame_var){
            return CAR(vals);
        }
        vars = CDR(vars),vals = CDR(vals);
    }
    //not found
    return NULL;
}

Object* lookup_variable_value(Object* var,Object* env){
    Object *pair = env;
    Object *val;
    while(pair != NULL){
        val = scan_in_frame(CAR(pair),var);
        if(val != NULL) return val;
        pair = CDR(pair);
    }
    perror("unbound variable!");
    exit(1);
}

/* func -> (list 'primitive func) */
Object* make_primitive_procedure(VM* vm,Object* primitive_procedure){
    Object* list = make_empty_list(vm);
    list_append_obj(vm,list,primitive);
    list_append_obj(vm,list,primitive_procedure);
    return list;
}

void add_primitive_procedure(VM* vm,Object* vars,Object* vals,char* symname,Object* (*func)()){
    Object* sym_obj = make_symbol(vm,symname);
    Object* primitive_procedure = newPrimitiveProcedure(vm,func);
    Object* func_list = make_primitive_procedure(vm,primitive_procedure);
    list_append_obj(vm,vars,sym_obj);
    list_append_obj(vm,vals,func_list);
}

Object* init_env(VM* vm){
    Object* vars = make_empty_list(vm);
    Object* vals = make_empty_list(vm);

    add_primitive_procedure(vm,vars,vals,"+",primitive_add);
    add_primitive_procedure(vm,vars,vals,"-",primitive_sub);

    Object* empty_env = make_empty_list(vm);
    return extend_env(vm,vars,vals,empty_env);
}
/* 
 * read tokens, return a pointer to Object. 
 * just like scheme's read procedure.
 * If not a list,just return the last obj! 
 *
 * normally,the obj we return is a list.
 */
Object* obj_read(VM* vm,Token* tokens_head){
    Token* token = tokens_head;
    Object *parent=make_empty_list(vm),*obj=NULL;

    while(token != NULL){
        switch(token->type){
            case LP:
                obj = make_empty_list(vm);
                list_append_obj(vm,parent,obj);
                push(vm,parent);
                parent = obj;
                break;
            case RP:
                parent = pop(vm);
                break;
            default:
                switch(token->type){
                    case INTEGER:
                        obj = newIntegerObject(vm,string_to_int(token->text));
                        break;
                    case BOOLEAN:
                        if(strcmp(token->text,"#t")==0){
                            obj = newBooleanObject(vm,1);
                        }else if(strcmp(token->text,"#f")==0){
                            obj = newBooleanObject(vm,0);
                        }else{
                            perror("boolean literal error!");
                            exit(1);
                        }
                        break;
                    case STRING:
                        obj = newStringObject(vm,heap_string(token->text));
                        break;
                    case IDENTIFIER:
                        obj = make_symbol(vm,token->text);
                        break;
                    default:
                        perror("the data type not supported now!");
                        exit(1);
                }

                list_append_obj(vm,parent,obj);
        }

        token=token->next;
    }
    return list_last_obj(parent);
}

Object* obj_eval(VM* vm,Object* obj,Object* env);
Object* obj_apply(VM* vm,Object* obj,Object* params);

Object* get_if_predicate(Object* obj){ return CADR(obj); }

Object* get_if_consequent(Object* obj){ return CADDR(obj); }

Object* get_if_alternative(VM* vm,Object* obj){
    if(CDDDR(obj) != NULL){
        return CADDDR(obj);
    }else{
        return newBooleanObject(vm,0);
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
    Object* args = make_empty_list(vm);

    Object* pair = operands;

    push(vm,args); // save
    while(pair != NULL){
        operand_obj = obj_eval(vm,CAR(pair),env);
        list_append_obj(vm,args,operand_obj);
        pair = CDR(pair);
    }
    pop(vm); //restore
    return args;
}

Object* get_lambda_params(Object* obj){ return CADR(obj); }
Object* get_lambda_body(Object* obj){ return CDDR(obj); }

Object* make_procedure(VM* vm,Object* params,Object* body,Object* env){
    Object* list = make_empty_list(vm);
    list_append_obj(vm,list,procedure);
    list_append_obj(vm,list,params);
    list_append_obj(vm,list,body);
    list_append_obj(vm,list,env);
    return list;
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
    if(obj->type == OBJ_INTEGER || obj->type == OBJ_STRING 
            || obj->type == OBJ_CHARACTER || obj->type == OBJ_BOOLEAN){
        //self evaluating
        return obj;
    }else if(obj->type == OBJ_SYMBOL){
        //variable
        return lookup_variable_value(obj,env);
    }else if(is_list_tagged(obj,keywords[KWD_LAMBDA])){
        //lambda
            push(vm,env);
            push(vm,obj);
            Object* myobj = make_procedure(vm,get_lambda_params(obj),get_lambda_body(obj),env);
            pop(vm);
            pop(vm);
    }else if(is_list_tagged(obj,keywords[KWD_IF])){
        //if
        return if_eval(vm,obj,env);
    }else if(is_list_tagged(obj,keywords[KWD_QUOTE])){
        //quote
    }else if(is_list_tagged(obj,keywords[KWD_SET])){
        //set
    }else if(is_list_tagged(obj,keywords[KWD_DEFINE])){
        //define
    }else if(is_list_tagged(obj,keywords[KWD_BEGIN])){
        //begin
    }else if(is_list_tagged(obj,keywords[KWD_COND])){
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
        return obj_apply(vm,proc,args);
    }else{
        perror("unknow expression type");
        exit(1);
    }
}

int is_primitive_procedure(Object* obj){ return CAR(obj) == primitive; }
int is_compound_procedure(Object* obj){ return CAR(obj) == procedure; }

Object* get_procedure_body(Object* obj){ return CADDR(obj); }
Object* get_procedure_parameters(Object* obj){ return CADR(obj); }
Object* get_procedure_env(Object* obj){ return CADDDR(obj); }

Object* eval_sequence(VM* vm,Object* obj,Object* env){
    Object* result;
    Object* pair = obj;
    while(pair != NULL){
        result = obj_eval(vm,obj,env);
        pair = CDR(pair);
    }
    return result;
}

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
    if(is_primitive_procedure(procedure)){
        return apply_primitive_procedure(vm,procedure,arguments);
    }else if(is_compound_procedure(procedure)){
        Object* myprocedure = get_procedure_body(procedure);
        Object* myparameters = get_procedure_parameters(procedure);
        Object* myenv = get_procedure_env(procedure);
        return eval_sequence(vm,myprocedure,extend_env(vm,myparameters,arguments,myenv));
    }else{
        perror("unknown procedure type!");
        exit(1);
    }
}

int main(int argc,char** argv){
    FILE *f;
    if(argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    /* gen tokens */
    Token *tokens_head = (Token *) get_tokens(f);
    /* show tokens */
    print_tokens();

    VM* vm = newVM();
    init_symbols(vm);
    Object* global_env = init_env(vm);
    Object* o = obj_read(vm,tokens_head);
    Object* r = obj_eval(vm,o,global_env);
    printObject(o);
    printf("\n>>>");
    printObject(r);
    printf("\n");

    destroy_tokens(tokens_head);
    freeVM(vm);

    fclose(f);
    return 0;
}
