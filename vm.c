#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
extern Object* Consts;

// Creates a new VM with an empty stack and an empty (but allocated) heap.
VM* newVM() {
    VM* vm = malloc(sizeof(VM));
    vm->stackSize = 0;

    vm->heap = malloc(HEAP_SIZE);
    vm->next = vm->heap;

    return vm;
}

// Pushes a reference to [value] onto the VM's stack.
void push(VM* vm, Object* value) {
    if (vm->stackSize == STACK_MAX) {
        perror("Stack overflow.\n");
        exit(1);
    }

    vm->stack[vm->stackSize++] = value;
}

// Pops the top-most reference to an object from the stack.
Object* pop(VM* vm) {
    return vm->stack[--vm->stackSize];
}

// Marks [object] as being reachable and still (potentially) in use.
void mark(Object* object) {
    // If already marked, we're done. Check this first to avoid recursing
    // on cycles in the object graph.
    if (object->moveTo) return;

    // Any non-zero pointer indicates the object was reached. For no particular
    // reason, we use the object's own address as the marked value.
    object->moveTo = object;

    // Recurse into the object's fields.
    //if (object->type == OBJ_PAIR) {
    if (object->type == OBJ_PAIR) {
        //mark(object->head);
        mark(CAR(object));
        //mark(object->tail);
        mark(CDR(object));
    }
}

// The mark phase of garbage collection. Starting at the roots (in this case,
// just the stack), recursively walks all reachable objects in the VM.
void markAll(VM* vm) {
    /* mark stack */
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
    /* mark consts */
    Object* p = Consts;
    while(p != Nil){
        mark(CAR(p));
        p = CDR(p);
    }
}

// Phase one of the LISP2 algorithm. Walks the entire heap and, for each live
// object, calculates where it will end up after compaction has moved it.
//
// Returns the address of the end of the live section of the heap after
// compaction is done.
void* calculateNewLocations(VM* vm) {
    // Calculate the new locations of the objects in the heap.
    void* from = vm->heap;
    void* to = vm->heap;
    while (from < vm->next) {
        Object* object = (Object*)from;
        if (object->moveTo) {
            object->moveTo = to;

            // We increase the destination address only when we pass a live object.
            // This effectively slides objects up on memory over dead ones.
            to += sizeof(Object);
        }

        from += sizeof(Object);
    }

    return to;
}

// Phase two of the LISP2 algorithm. Now that we know where each object *will*
// be, find every reference to an object and update that pointer to the new
// value. This includes reference in the stack, as well as fields in (live)
// objects that point to other objects.
//
// We do this *before* compaction. Since an object's new location is stored in
// [object.moveTo] in the object itself, this needs to be able to find the
// object. Doing this process before objects have been moved ensures we can
// still find them by traversing the *old* pointers.
void updateAllObjectPointers(VM* vm) {
    // Walk the stack.
    for (int i = 0; i < vm->stackSize; i++) {
        // Update the pointer on the stack to point to the object's new compacted
        // location.
        vm->stack[i] = vm->stack[i]->moveTo;
    }

    // Walk the heap, fixing fields in live pairs.
    void* from = vm->heap;
    while (from < vm->next) {
        Object* object = (Object*)from;

        if (object->moveTo && object->type == OBJ_PAIR) {
            //object->head = object->head->moveTo;
            CAR(object) = CAR(object)->moveTo;
            CDR(object) = CDR(object)->moveTo;
        }

        from += sizeof(Object);
    }
}

/* heap is inside. 
 * The pointer in heap maybe point to data *NOT* in heap.
 * like 'string' and 'symbol' */
void sweepOutside(VM* vm){
    void* p = vm->heap;
    while (p < vm->next) {
        Object* object = (Object*)p;
        if(object->moveTo == NULL){
            if(object->type == OBJ_STRING || object->type == OBJ_SYMBOL){
                if(object->value.s != NULL){
                    free(object->value.s);
                }
            }
        }
        p += sizeof(Object);
    }
}

// Phase three of the LISP2 algorithm. Now that we know where everything will
// end up, and all of the pointers have been fixed, actually slide all of the
// live objects up in memory.
void compact(VM* vm) {
    void* from = vm->heap;

    while (from < vm->next) {
        Object* object = (Object*)from;
        if (object->moveTo) {
            // Move the object from its old location to its new location.
            Object* to = object->moveTo;
            memmove(to, object, sizeof(Object));

            // Clear the mark.
            to->moveTo = NULL;
        }

        from += sizeof(Object);
    }
}

// Free memory for all unused objects.
void gc(VM* vm) {
    // Find out which objects are still in use.
    markAll(vm);

    // Determine where they will end up.
    void* end = calculateNewLocations(vm);

    // Fix the references to them.
    updateAllObjectPointers(vm);
    /* sweep before compact */
    sweepOutside(vm);
    // Compact the memory.
    compact(vm);

    // Update the end of the heap to the new post-compaction end.
    vm->next = end;

    //printf("%ld live bytes after collection.\n", vm->next - vm->heap);
}

// Deallocates all memory used by [vm].
void freeVM(VM *vm) {
    free(vm->heap);
    free(vm);
}

// Create a new object.
//
// This does *not* root the object, so it's important that a GC does not happen
// between calling this and adding a reference to the object in a field or on
// the stack.
Object* newObject(VM* vm, ObjectType type) {
    if (vm->next + sizeof(Object) > vm->heap + HEAP_SIZE) {
        gc(vm);

        // If there still isn't room after collection, we can't fit it.
        if (vm->next + sizeof(Object) > vm->heap + HEAP_SIZE) {
            perror("Out of memory");
            exit(1);
        }
    }

    Object* object = (Object*)vm->next;
    vm->next += sizeof(Object);

    object->type = type;
    object->moveTo = NULL;

    return object;
}

// Creates a new int object and pushes it onto the stack.
void pushInt(VM* vm, int intValue) {
    Object* object = newObject(vm, OBJ_INTEGER);
    object->value.i = intValue;

    push(vm, object);
}

// Creates a new pair object. The field values for the pair are popped from the
// stack, then the resulting pair is pushed.
Object* pushPair(VM* vm) {
    // Create the pair before popping the fields. This ensures the fields don't
    // get collected if creating the pair triggers a GC.
    Object* object = newObject(vm, OBJ_PAIR);

    CDR(object) = pop(vm);
    CAR(object) = pop(vm);

    push(vm, object);
    return object;
}

void printObject(Object* obj){
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
                printf("%s","<#primitive_procedure>");
                break;
            case OBJ_PAIR:
                printf("(");
                printObject(CAR(obj));
                printf(",");
                printObject(CDR(obj));
                printf(")");
        }
    }else{
        printf("nil");
    }
}

Object* newIntegerObject(VM* vm,int i){
    Object* obj = newObject(vm,OBJ_INTEGER);
    obj->value.i = i;
    return obj;
}

Object* newSymbolObject(VM* vm,char* symname){
    Object* obj = newObject(vm,OBJ_SYMBOL);
    obj->value.s = symname;
    return obj;
}

Object* newPrimitiveProcedure(VM* vm,Object* (*primitive)()){
    Object* obj = newObject(vm,OBJ_PRIMITIVE_PROCEDURE);
    obj->value.primitive_procedure = primitive;
    return obj;
}

Object* newStringObject(VM* vm,char* s){
    Object* obj = newObject(vm,OBJ_STRING);
    obj->value.s = s;
    return obj;
}

Object* cons(VM* vm,Object* car,Object* cdr){
    Object* pair = newObject(vm,OBJ_PAIR);
    CAR(pair) = car;
    CDR(pair) = cdr;
    return pair;
}

Object* list1(VM* vm,Object* obj){
    return cons(vm,obj,Nil);
}

Object* list2(VM* vm,Object* obj1,Object* obj2){
    return cons(vm,obj1,cons(vm,obj2,Nil));
}

Object* list3(VM* vm,Object* obj1,Object* obj2,Object* obj3){
    return cons(vm,obj1,list2(vm,obj2,obj3));
}

Object* list4(VM* vm,Object* obj1,Object* obj2,Object* obj3,Object* obj4){
    return cons(vm,obj1,list3(vm,obj2,obj3,obj4));
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

Object* append(VM* vm,Object* list,Object* obj){
    if(list == Nil){
        return cons(vm,obj,Nil);
    }else{
        Object* lastPair = last_pair(list);
        CDR(lastPair) = cons(vm,obj,Nil);
        return list;
    }
}

