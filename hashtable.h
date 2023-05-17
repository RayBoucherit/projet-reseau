#ifndef HTABLE_H
#define HTABLE_H
#define NEIGH_SIZE 15
#define DATA_SIZE 512

typedef struct node_data node_data;
typedef struct node_id node_id;
typedef struct node_sock node_sock;
typedef struct node_neigh node_neigh;
typedef struct hashtable hashtable;

struct node_data
{
    uint16_t seqno;
    unsigned char len;
    unsigned char body[];
};

struct node_id
{
    unsigned char nid[8];
};

struct node_sock{
    unsigned char ip[16];
    uint16_t port;
};

struct node_neigh{
    uint8_t keep;
    time_t date;
};

struct hashtable
{
    size_t size;
    void** keys;
    void** values;
};

void swap(void *a, void *b);
int partition(hashtable *table, int low, int high);
void quicksort(hashtable *table, int low, int high);

node_data* node_data_factory(uint16_t seqno , uint8_t len, void* data);
node_id* node_id_factory(void* id);
node_sock *node_sock_factory(unsigned char *ip, uint16_t port);
node_neigh *node_neigh_factory(uint8_t keep, time_t date);

hashtable* new_hashtable(size_t size);
int delete(hashtable *h, void *key, size_t keysize);
int hashtable_size(hashtable *h);
void free_hashtable(hashtable* h);
hashtable* resize_hashtable(hashtable* h);
int find_key(hashtable* h, void* key, size_t keysize);
int find_free(hashtable* h);
int update(hashtable* h, void* key, size_t keysize, void* value);

#endif
