#ifndef HASH_H
#define HASH_H

typedef struct hashtable hashtable;

void hash_f(void *dst, unsigned char *id, uint16_t seqno, unsigned char *data, unsigned int data_size);
void hash_data_t(void *dst, hashtable *data);

#endif
