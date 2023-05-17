#include <openssl/sha.h>
#include <arpa/inet.h>
#include <string.h>
#include "hashtable.h"
#include "hash.h"

void hash_f(void *dst, unsigned char *id, uint16_t seqno, unsigned char *data, unsigned int data_size) {
    uint16_t net_seq = htons(seqno);
    unsigned int s_size = 8 + 2 + data_size;
    unsigned char s[s_size];
    unsigned char digest[SHA256_DIGEST_LENGTH];
    memcpy(&s[0], id, 8);
    memcpy(&s[8], &net_seq, 2);
    memcpy(&s[10], data, data_size);
    SHA256(s, s_size, digest);
    memcpy(dst, digest, 16);
}

void hash_data_t(void *dst, hashtable *data) {
    int data_size = find_free(data);
    int s_size = data_size * 16;
    unsigned char s[s_size];
    memset(s, 0, s_size);
    unsigned char digest[SHA256_DIGEST_LENGTH];
    for (size_t i = 0; i < data_size; i++) {
        node_id *id = data->keys[i];
        node_data *nd = data->values[i];
        if (id != NULL && nd != NULL)
            hash_f(&s[i * 16], id->nid, nd->seqno, nd->body, nd->len);
    }
    SHA256(s, s_size, digest);
    memcpy(dst, digest, 16);
}
