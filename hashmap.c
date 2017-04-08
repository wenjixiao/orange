#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M 37

typedef struct _bucket {
    char *key;
    void *pvalue;
    struct _bucket *next;
} Bucket;

typedef struct _hashtable {
    unsigned size; /* buckets count include free bucket and used bucket */
    unsigned used; /* used buckets count */
    Bucket *head;
} HashTable;

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
    HashTable *ht = (HashTable *) malloc(sizeof(HashTable));
    ht->size = size;
    ht->used = 0;
    ht->head = (Bucket *) calloc(ht->size,sizeof(Bucket));
    return ht;
}

static Bucket *_get_bucket(HashTable *ht,char *key){
    unsigned h = BKDRHash(key,ht->size);
    //printf("key=%s,h=%u\n",key,h);
    return ht->head+h*sizeof(Bucket);
}

void hashtable_insert(HashTable *ht,char* key,void *pvalue){
    Bucket *p = _get_bucket(ht,key);
    p->key = key;
    p->pvalue = pvalue;
    ht->used++;
}
/* return a pointer to any type,you should change type from void* */
void *hashtable_get(HashTable *ht,char* key){
    Bucket *p = _get_bucket(ht,key);
    if(p->key != NULL){
        if(p->next == NULL){
            return p->pvalue;
        }else{
            do{
                if(strcmp(p->key,key)==0){
                    return p->pvalue;
                }
                p = p->next;
            }while(p != NULL);
        }
    }
    return NULL;
}

void hashtable_delete(HashTable *ht,char* key){
    Bucket *p = _get_bucket(ht,key);
    if(p->key != NULL){
        if(p->next == NULL){
            p->key = NULL,p->pvalue = NULL;
        }else{
            if(strcmp(p->key,key)==0){
                //head bucket is,so copy next to head
                p->key = p->next->key;
                p->pvalue = p->next->pvalue;
                p->next = p->next->next;
                free(p->next);
                ht->used--;
            }else{
                //not at head,in body list
                Bucket *pre = p;
                Bucket *q = p->next;
                do{
                    if(strcmp(q->key,key)==0){
                       pre->next = q->next;
                       free(q);
                       ht->used--;
                       break;
                    }
                    pre = q;
                    q = q->next;
                }while(q != NULL);
            }
        }
    }
}

void hashtable_print(HashTable *ht,void (* func)()){
    printf("------------------\n");
    unsigned i;
    Bucket *p;
    for(i=0;i<ht->size;i++){
        p=ht->head+i*sizeof(Bucket);
        if(p->key != NULL){
            printf("index=%u,key=%s,",i,p->key);
            (*func)(p->pvalue);
            printf("\n");
        }
    }
}
//===============================================
int *get_int(int i){
    int *p = (int *) malloc(sizeof(int));
    *p = i;
}

void int_print(void *p){
    int *v = (int *) p;
    printf("%d",*v);
}
//===============================================
int main(){
    HashTable *ht = hashtable_init(M);
    printf("size=%d\n",ht->size);

    hashtable_insert(ht,"hello",get_int(1));
    hashtable_insert(ht,"you",get_int(2));

    void *v = hashtable_get(ht,"iamwen");

    if(v == NULL){
        printf("not found!\n");
    }else{
        printf("found: ");
        int_print(v);
        printf("\n");
    }

    hashtable_print(ht,int_print);
    hashtable_delete(ht,"you");
    hashtable_print(ht,int_print);
    return 0;
}
