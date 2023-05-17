#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "tlv.h"

int size(tlv* t)
{
    if (t == NULL) return 0;
    if (t->type == 0) return 1;
    return t->len + 2;
}

tlv* pad1()
{
    tlv* t = malloc(1);
    if (t == NULL) return NULL;
    t->type = 0;
    return t;
}

tlv* padn(uint8_t n)
{
    tlv* t = malloc(2 + n);
    t->type = 1;
    t->len = n;
    memset(t->body, 0, t->len);
    return t;
}

tlv* neighbour_request()
{
    tlv* t = malloc(2);
    if (t == NULL) return NULL;
    t->type = 2;
    t->len = 0;
    return t;
}

tlv* neighbour(struct in6_addr* ip_addr, uint16_t port)
{
    tlv* t = malloc(20);
    if (t == NULL) return NULL;
    uint16_t swapped = htons(port);
    t->type = 3;
    t->len = 18;
    memcpy(t->body, ip_addr, 16);
    memcpy(&t->body[16], &swapped, 2);
    return t;
}

tlv* network_hash(void* network_hash)
{
    tlv* t = malloc(18);
    if (t == NULL) return NULL;
    t->type = 4;
    t->len = 16;
    memcpy(t->body, network_hash, 16);
    return t;
}

tlv* network_state_request()
{
    tlv* t = malloc(2);
    if (t == NULL) return NULL;
    t->type = 5;
    t->len = 0;
    return t;
}

tlv* node_hash(void* node_id, uint16_t seqno, void* hash)
{
    tlv* t = malloc(28);
    if (t == NULL) return NULL;
    uint16_t swapped = htons(seqno);
    t->type = 6;
    t->len = 26;
    memcpy(t->body, node_id, 8);
    memcpy(&t->body[8], &swapped, 2);
    memcpy(&t->body[10], hash, 16);
    return t;
}

tlv* node_state_request(void* node_id)
{
    tlv *t = malloc(10);
    if (t == NULL) return NULL;
    t->type = 7;
    t->len = 8;
    memcpy(t->body, node_id, 8);
    return t;
}

tlv* node_state(void* node_id, uint16_t seqno, void* hash, void *data, uint8_t data_size)
{
    if (data_size > 192) return NULL;
    tlv* t = malloc(28 + data_size);
    if (t == NULL) return NULL;
    uint16_t swapped = htons(seqno);
    t->type = 8;
    t->len = 26 + data_size;
    memcpy(t->body, node_id, 8);
    memcpy(&t->body[8], &swapped, 2);
    memcpy(&t->body[10], hash, 16);
    memcpy(&t->body[26], data, t->len - 26);
    return t;
}


tlv* warning(void* msg, uint8_t msg_size)
{
    tlv* t = malloc(2 + msg_size);
    if (t == NULL) return NULL;
    t->type = 9;
    t->len = msg_size;
    memcpy(t->body, msg, t->len);
    return t;
}

int parse_tlv(tlv *t){
    if(t == NULL) return 0;
    switch(t->type){
        case 0:
            return 1;
        break;
        case 1:
            return 1;
        break;
        case 2:
            if(t->len == 0 && t->body == NULL) return 1;
        break;
        case 3:
            if(t->len == 24) return 1;
        break;
        case 4:
            return 1;
        break;
        case 5:
            return 1;
        break;
        case 6:
            if(t->len == 40) return 1;
        break;
        case 7:
            if(t->len == 8) return 1;
        break;
        case 8:
            if(t->len >=40 && t->len <= 40 + 192) return 1;
        break;
        case 9:
            return 1;
        break;
        default:
        return 0;
        break;
    }
    return 0;
}
