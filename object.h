#ifndef OBJECT_H
#define OBJECT_H

#define STACK_MAX 256

#define PAIR(obj)       obj->value.pair

#define CDR(obj)        PAIR(obj).cdr
#define CDDR(obj)       CDR(CDR(obj))
#define CDDDR(obj)      CDR(CDDR(obj))

#define CAR(obj)        PAIR(obj).car /* first */
#define CADR(obj)       CAR(CDR(obj)) /* second */
#define CADDR(obj)      CAR(CDDR(obj)) /* third */
#define CADDDR(obj)     CAR(CDDDR(obj)) /* fourth */

#define CAADR(obj)       CAR(CAR(CDR(obj)))
#define CDADR(obj)       CDR(CAR(CDR(obj)))

#define INTEGER(obj)     obj->value.i
#define STR(obj)         obj->value.s

typedef enum {
	OBJ_NULL,
    OBJ_VOID,
    OBJ_INTEGER,
    OBJ_SYMBOL,
    OBJ_STRING,
    OBJ_BOOLEAN,
    OBJ_CHARACTER,
    OBJ_PRIMITIVE_PROCEDURE,
    OBJ_PAIR
} ObjectType;

typedef struct _object {
    ObjectType type;
    union {
        int i;
        char c;
        char *s;
        struct _object * (*primitive_procedure)();
        struct {
            struct _object *car,*cdr;
        } pair;
    } value;
} Object;

void myprint(Object* obj,char* s);
void obj_print(Object* obj);
Object* cons(Object* car,Object* cdr);
int length(Object* list);
Object* append(Object* list,Object* obj);
Object* last(Object* list);

Object* new_object(ObjectType type);
Object* new_integer(int i);
Object* new_symbol(char* symname);
Object* new_string(char* s);
Object* new_primitive_procedure(Object* (*primitive)());

Object* new_list1(Object* obj);
Object* new_list2(Object* obj1,Object* obj2);
Object* new_list3(Object* obj1,Object* obj2,Object* obj3);
Object* new_list4(Object* obj1,Object* obj2,Object* obj3,Object* obj4);

#endif
