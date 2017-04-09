#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "hashtable.h"
#define M 3

/*
 * 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47
 * 53 59 61 67 71 73 79 83 89 97
 * m is the size of hash table,you should select a prime number!
 */
unsigned int BKDRHash(char* str,unsigned size)
{
    unsigned seed = 131; /* 31 131 1313 13131 131313 etc.. */
    unsigned hash = 0;
    while(*str){
        hash = (hash * seed) + (*str++);
    }
    return hash % size;
}

/* len = 2^r so, r = log(2)len = log(10)len/log(10)2 */
/* hash table's length is 2^bits */
unsigned int MULHash(unsigned k,int bits){
    unsigned A = 2654435769u;
    //(K*A mod 2^w)rsh(w-r)
    //h(k) = (k*2654435769) >> (w-r)
    unsigned w = 32; //k is 32bit
    //unsigned int r = log(len) / log(2);
    unsigned r = bits;
    return A*k>>w-r;
}

HashTable *hashtable_init(unsigned size){
    HashTable *ht = (HashTable *) GC_MALLOC(sizeof(HashTable));
    ht->size = size;
    ht->head = (Bucket *) GC_MALLOC(ht->size*sizeof(Bucket));
    return ht;
}

static Bucket *_get_bucket(HashTable *ht,char *key){
    unsigned h = BKDRHash(key,ht->size);
    //printf("key=%s,h=%u\n",key,h);
    return ht->head+h*sizeof(Bucket);
}

void hashtable_put(HashTable *ht,char* key,int value){
    Bucket *p = _get_bucket(ht,key);
    if(p->key != NULL){
        //collision
        printf("#insert collision\n");
        Bucket *q = p;
        while(q->next != NULL){
            q = q->next;
        }
        if(q->next == NULL){
            Bucket *new_bucket = (Bucket *) GC_MALLOC(sizeof(Bucket));
            new_bucket->key = key;
            new_bucket->value = value;
            new_bucket->next = NULL;
            q->next = new_bucket;
            ht->used++;
        }
    }else{
        //the bucket is empty now
        printf("#insert\n");
        p->key = key;
        p->value = value;
        p->next = NULL;
        ht->used++;
    }
}
/* return a pointer to any type,you should change type from void* */
int hashtable_get(HashTable *ht,char* key){
    Bucket *p = _get_bucket(ht,key);
    if(p->key != NULL){
        if(p->next == NULL){
            return p->value;
        }else{
            //collision
            do{
                if(strcmp(p->key,key)==0){
                    return p->value;
                }
                p = p->next;
            }while(p != NULL);
        }
    }
    return 0;
}

void hashtable_delete(HashTable *ht,char* key){
    Bucket *p = _get_bucket(ht,key);
    if(p->key != NULL){
        if(p->next == NULL){
            //no collision,just one
            p->key = NULL,p->value = 0;
        }else{
            //it's a list
            if(strcmp(p->key,key)==0){
                //head bucket is,so copy next to head
                //here,p->next is not NULL!
                Bucket *q = p->next;
                p->key = q->key;
                p->value = q->value;
                p->next = q->next;
                ht->used--;
                //free(q);
            }else{
                //not at head,in body list
                Bucket *pre = p;
                Bucket *q = p->next; //here,q is not NULL too!
                do{
                    if(strcmp(q->key,key) == 0){
                       pre->next = q->next;
                       ht->used--;
                       //free(q);
                       break;
                    }
                    pre = q;
                    q = q->next;
                }while(q != NULL);
            }
        }
    }
}

void hashtable_print(HashTable *ht){
    printf("---------begin print---------\n");
    printf("size=%d,used=%d\n",ht->size,ht->used);
    unsigned i;
    Bucket *p,*q;
    for(i=0;i<ht->size;i++){
        p = ht->head + i*sizeof(Bucket);
        if(p->key != NULL){
            printf("index=%u: %s=>%d",i,p->key,p->value);
            q = p->next;
            while(q != NULL){
                printf("|");
                printf("%s=>%d",q->key,q->value);
                q = q->next;
            }
            printf("\n");
        }else{
            printf("key is NULL!\n");
        }
    }
}

void hashtable_test(){
    HashTable *ht = hashtable_init(M);

    hashtable_put(ht,"hello",1);
    hashtable_put(ht,"world",2);
    hashtable_put(ht,"i",3);
    hashtable_put(ht,"am",4);
    hashtable_put(ht,"father",886);
    hashtable_put(ht,"your",5);
    hashtable_put(ht,"I",9);
    hashtable_put(ht,"love",77);
    hashtable_put(ht,"you",6);

    hashtable_print(ht);
    hashtable_delete(ht,"world");
    hashtable_delete(ht,"i");
    hashtable_print(ht);
}
//===============================================
int main(){
    hashtable_test();
    return 0;
}
