#ifndef TLV_H
#define TLV_H

typedef struct tlv tlv;

struct tlv{
    unsigned type;
    unsigned len;
    unsigned char body[];
};

tlv* pad1();
tlv* padn(uint8_t n);
tlv* neighbour_request();
tlv* neighbour(struct in6_addr* ip_addr, uint16_t port);
tlv* network_hash(void* network_hash);
tlv* network_state_request();
tlv* node_hash(void* node_id, uint16_t seqno, void* hash);
tlv* node_state_request(void* node_id);
tlv* node_state(void* node_id, uint16_t seqno, void* hash, void *data, uint8_t data_size);
tlv* warning(void* msg, uint8_t msg_size);
int parse_tlv(tlv *t);

#endif
