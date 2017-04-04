#define STACK_MAX 256
#define HEAP_SIZE (1024 * 1024)

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

typedef enum {
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
    void *moveTo;
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

// A virtual machine with its own virtual stack and heap. All objects live on
// the heap. The stack just points to them.
typedef struct {
  Object* stack[STACK_MAX];
  int stackSize;

  // The beginning of the contiguous heap of memory that objects are allocated
  // from.
  void* heap;

  // The beginning of the next chunk of memory to be allocated from the heap.
  void* next;
} VM;

VM* newVM(); 
void push(VM* vm, Object* value);
Object* pop(VM* vm);
void gc(VM* vm);
void freeVM(VM *vm);

Object* newObject(VM* vm, ObjectType type);
void pushInt(VM* vm, int intValue);
Object* pushPair(VM* vm);
void printObject(Object* obj);

Object* cons(VM* vm,Object* car,Object* cdr);
Object* make_empty_list(VM* vm);
int list_length(Object* list);
void list_append_obj(VM* vm,Object* list,Object* obj);
int is_list_empty(Object* list);
Object* list_last_obj(Object* list);
Object* newIntegerObject(VM* vm,int i);
Object* newSymbolObject(VM* vm,char* symname);
Object* newStringObject(VM* vm,char* s);
Object* newBooleanObject(VM* vm,int i);
Object* newPrimitiveProcedure(VM* vm,Object* (*primitive)());
