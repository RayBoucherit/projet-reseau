#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "hashtable.h"

//quicksort
void swap(void *a, void *b){
    if (a == NULL && b == NULL){
        return;
    }
    else if(b == NULL){
        a = NULL;
        return;
    }
    else{
        b = NULL;
        return;
    }
    void *tmp = a;
    a = b;
    b = tmp;
}

int partition(hashtable *table, int low, int high){
    void *pivot = table->keys[high];
    int i = low;
    for (int j = 0; j < high; j++){
        if((table->keys[j] != NULL && table->values[j] != NULL) && memcmp(table->keys[j], pivot, 16) < 0){
            swap(table->keys[i], table->keys[j]);
            swap(table->values[i], table->values[j]);
            i++;
        }
    }
    swap(table->keys[i], table->keys[high]);
    swap(table->values[i], table->values[high]);
    return i;
}

void quicksort(hashtable *table, int low, int high){
    if(table == NULL) return;
    else if(table->keys[low] == NULL){
        quicksort(table, low + 1, high);
    }
    else if(table->keys[high] == NULL){
        quicksort(table, low, high - 1);
    }
    else if(low < high){
        int p = partition(table, low, high);
        quicksort(table, low, p - 1);
        quicksort(table, p + 1, high);
    }
}
//quicksort

node_data* node_data_factory(uint16_t seqno , uint8_t len, void* data)
{
    node_data* d = malloc(3 + len);
    if (d == NULL) return NULL;
    d->seqno = htons(seqno);
    d->len = len;
    memcpy(d->body, data, len);
    return d;
}

node_id* node_id_factory(void* id)
{
    node_id* n = malloc(8);
    if (n == NULL) return NULL;
    memcpy(n, id, 8);
    return n;
}

node_sock *node_sock_factory(unsigned char *ip, uint16_t port){
    node_sock *n = malloc(32);
    if (n == NULL) return NULL;
    inet_pton(AF_INET6, (char *)ip, n->ip);
    n->port = port;
    return n;
}

node_neigh *node_neigh_factory(uint8_t keep, time_t date){
    node_neigh *n = malloc(1 + sizeof(time_t));
    if( n == NULL) return NULL;
    n->keep = keep;
    n->date = date;
    return n;
}

hashtable* new_hashtable(size_t size)
{
    hashtable* h = malloc(sizeof(hashtable));
    if (h == NULL) return NULL;
    h->size = size;
    h->keys = malloc(h->size * sizeof(void*));
    if (h->keys == NULL) return NULL;
    h->values = malloc(h->size * sizeof(void*));
    if (h->values == NULL) return NULL;
    for (size_t i = 0; i < h->size; i++)
    {
        h->keys[i] = NULL;
        h->values[i] = NULL;
    }
    return h;
}

int delete(hashtable *h, void *key, size_t keysize){
    int i = find_key(h, key, keysize);
    if(i > -1){
        for(int j = i; i + 1< h->size; j++){
            swap(h->keys[i], h->keys[i + 1]);
            swap(h->values[i], h->values[i + 1]);
            if(h->keys[i + 1] == NULL && h->values[i + 1] == NULL){
                h->keys[i] = NULL;
                h->values[i] = NULL;
                return 1;
            }
        }
    }
    return 0;
}

int hashtable_size(hashtable *h){
    int n = find_free(h);
    return n < 0 ? 0 : n;
}

void free_hashtable(hashtable *h)
{
    free(h->keys);
    free(h->values);
    free(h);
}

hashtable* resize_hashtable(hashtable* h)
{
    if (h == NULL) return NULL;
    hashtable* new = malloc(sizeof(hashtable));
    if (new == NULL) return h;
    new->size = 2 * h->size;
    new->keys = realloc(h->keys, new->size);
    if (new->keys == NULL)
    {
        free_hashtable(new);
        return h;
    }
    new->values = realloc(h->values, new->size);
    if (new->values == NULL)
    {
        free_hashtable(new);
        return h;
    }
    free_hashtable(h);
    return new;
}

int find_key(hashtable* h, void* key, size_t keysize)
{
    if (h == NULL) return -1;
    for (size_t i = 0; i < h->size; i++)
    {
        if (h->keys[i] != NULL)
            if (memcmp(h->keys[i], key, keysize) == 0) return i;
    }
    return -1;
}

int find_free(hashtable* h)
{
    if (h == NULL) return -1;
    for (size_t i = 0; i < h->size; i++)
    {
        if (h->keys[i] == NULL && h->values[i] == NULL) return i;
    }
    return -1;
}

int update(hashtable* h, void* key, size_t keysize, void* value)
{
    if (h == NULL || key == NULL) return 0;
    int exists = find_key(h, key, keysize);
    if (exists >= 0)
    {
        free(h->values[exists]);
        if (value == NULL)
        {
            free(h->keys[exists]);
            h->keys[exists] = NULL;
            h->values[exists] = NULL;
            return 1;
        }
        h->values[exists] = value;
        return 1;
    }
    else
    {
        if (value == NULL) return 1;
        int free = find_free(h);
        if (free < 0)
        {
            free = h->size;
            h = resize_hashtable(h);
            if (free >= h->size) return 0;
        }
        h->keys[free] = key;
        h->values[free] = value;
        return 1;
    }
    return 0;
}
