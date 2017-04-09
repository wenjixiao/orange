#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct _bucket {
    char *key;
    int value;
    struct _bucket *next;
} Bucket;

typedef struct _hashtable {
    unsigned size;
    unsigned used;
    Bucket *head;
} HashTable;

HashTable *hashtable_init(unsigned size);
void hashtable_put(HashTable *ht,char* key,int value);
int hashtable_get(HashTable *ht,char* key);
void hashtable_delete(HashTable *ht,char* key);
void hashtable_print(HashTable *ht);

#endif
