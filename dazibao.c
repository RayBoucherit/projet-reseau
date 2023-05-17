#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "hashtable.h"
#include "package.h"
#include "tlv.h"
#include "hash.h"

unsigned char id[8] = "godzilla";
uint16_t seq = 0;
unsigned char data[] = "made in Hiroshima";
unsigned char hash[16];
unsigned char net_hash[16];

int main(int argc, char *argv[]) {
    hash_f(hash, id, seq, data, strlen((char*)data));
    if (argc != 3){
        printf("usage ./dazibao IPV6 PORT\n");
        exit(1);
    }
    int port = atoi(argv[2]);;
    int rc, s;
    struct timeval tv;
    fd_set recv;
    fd_set expc;
    s = socket(AF_INET6, SOCK_DGRAM, 0);
    if(s < 0){
        perror("socket()");
        exit(1);
    }
    printf("creating permanent neighbour\n");
    struct sockaddr_in6 permanent;
    memset(&permanent, 0, sizeof(permanent));
    permanent.sin6_family = AF_INET6;
    permanent.sin6_port = htons(port);
    rc = inet_pton(AF_INET6, argv[1], &permanent.sin6_addr);
    if(rc < 0){
        perror("inet_pton()");
        exit(1);
    }
    //add permanent neighbour
    printf("adding permanent neighbour\n");
    hashtable *neighbour_t = new_hashtable(NEIGH_SIZE);
    if (neighbour_t == NULL){
        perror("failed to create neighbour table");
        exit(1);
    }
    hashtable *data_t = new_hashtable(DATA_SIZE);
    if (data_t == NULL){
        perror("failed to create data table");
        exit(1);
    }
    node_sock *sockboot = node_sock_factory((unsigned char*)&permanent.sin6_addr, (uint16_t)permanent.sin6_port);
    node_neigh *neighboot = node_neigh_factory(1, 0);
    rc = update(neighbour_t, sockboot, sizeof(node_sock), neighboot);
    if(!rc){
        perror("failed to add permanent neighbour");
        exit(1);
    }
    printf("data table updated\n");
    quicksort(neighbour_t, 0, hashtable_size(neighbour_t));
    printf("data table sorted\n");
    while(1){
        tv.tv_sec = 20;
        tv.tv_usec = 0;
        FD_ZERO(&recv);
        FD_ZERO(&expc);
        FD_SET(s, &recv);
        FD_SET(s, &expc);
        select(s, &recv, NULL, &expc, &tv);
        if(FD_ISSET(s, &recv)){
            //new neighbour (maybe) sending package
            struct sockaddr_in6 new_neigh;
            //new node (maybe) contained in new_neigh tlv
            struct  sockaddr_in6 new_node;
            //time received
            time_t recvtm;
            socklen_t new_size = sizeof(new_neigh);
            socklen_t node_len = sizeof(new_node);
            memset(&new_neigh, 0, new_size);
            new_neigh.sin6_family = AF_INET6;
            char buf[PKG_MAX_SIZE];
            rc = recvfrom(s, &buf, PKG_MAX_SIZE, 0, &new_neigh, &new_size);
            recvtm = time(NULL);
            if(rc < 0){
                perror("failed to receive package");
            }
            else{
                printf("package received\n");
                package *p = package_factory(rc - 4);
                memcpy(p, &buf, p->len);
                //parse package
                if(package_check(p, rc)){
                    printf("package parsed\n");
                    //updating neighbour table
                    if(hashtable_size(neighbour_t) < 15){
                        node_sock *ns = node_sock_factory(new_neigh.sin6_addr.s6_addr, (uint16_t)(new_neigh.sin6_port));
                        node_neigh *nn;
                        //if new_neigh is permanent
                        if(memcmp(ns, sockboot, sizeof(node_sock))){
                            nn = node_neigh_factory(1, recvtm);
                        }
                        else{
                            nn = node_neigh_factory(0, recvtm);
                        }
                        update(neighbour_t, ns, sizeof(ns), nn);
                        quicksort(neighbour_t, 0, hashtable_size(neighbour_t));
                    }
                    //TODO : if tlv 0 t->len = NULL (segfault)
                    tlv *t = (tlv *)p->body;
                    node_id *id_n;
                    unsigned char tlv_hash[16];
                    for(int i = 0; i < p->len; i+=t->len + 2){
                        switch (t->type){
                            case 0:
                                printf("tlv pad1 received\n");
                            break;
                            case 1:
                                printf("tlv padn received\n");
                            break;
                            case 2:
                                //neighbour request
                                //send tlv neighbour
                                printf("tlv neighbour request received\n");
                                srand(time(NULL));
                                int r = rand() % hashtable_size(neighbour_t);
                                node_sock *n = neighbour_t->keys[r];
                                //building response
                                tlv *nb = neighbour((struct in6_addr *)n->ip, n->port);
                                package *res2 = package_factory(20);
                                memcpy(res2->body, nb, 20);
                                rc = sendto(s, res2, res2->len + 4, 0, &new_neigh, new_size);
                                if(rc < 0){
                                    perror("tlv neighbour not sent");
                                }
                                else{
                                    printf("tlv neighbour request sent\n");
                                }
                                free(res2);
                                free(nb);
                            case 3:
                                //neighbour
                                //send network hash to adress contained in tlv
                                printf("tlv neighbour received\n");
                                memset(&new_node, 0, node_len);
                                //TOCHECK t->body = IP + port
                                rc = inet_pton(AF_INET6, (char*)t->body, &new_node.sin6_addr);
                                if(rc < 0){
                                    perror("inet_pton()");
                                    exit(1);
                                }
                                uint16_t p;
                                memcpy(&p, &t->body[16], 2);
                                new_node.sin6_port = htons(p);
                                tlv *neth = network_hash(net_hash);
                                package *res3 = package_factory(16);
                                memcpy(res3->body, neth, 16);
                                rc = sendto(s, res3, res3->len + 4, 0, &new_node, new_size);
                                if(rc < 0){
                                    perror("tlv network hash not sent");
                                }
                                else{
                                    printf("tlv network hash sent\n");
                                }
                                free(neth);
                                free(res3);
                                break;
                            case 4:
                                //network hash
                                //send network state request
                                printf("tlv netwotk hash received\n");
                                if(!memcmp(t->body, net_hash, 16)){
                                    tlv *net_stat_res = network_state_request();
                                    package *res4 = package_factory(0);
                                    rc = sendto(s, res4, res4->len + 4, 0, &new_neigh, new_size);
                                    if(rc < 0){
                                        perror("tlv network state request not sent");
                                    }
                                    else{
                                        printf("tlv netwotk state request sent\n" );
                                    }
                                    free(net_stat_res);
                                    free(res4);
                                }
                            break;
                            case 5:
                                //network state request
                                //send serie of node hash, one for each known
                                //calculate serie of node hash
                                if (hashtable_size(data_t) * 28 < PKG_MAX_SIZE - 4) {
                                    package *pkg5 = package_factory(hashtable_size(data_t) * 28);
                                    for (size_t i = 0; i < hashtable_size(data_t); i++) {
                                        unsigned char *nid5 = ((node_id*)data_t->keys[i])->nid;
                                        uint16_t seq5 = ((node_data*)data_t->values[i])->seqno;
                                        unsigned char *nbody5 = ((node_data*)data_t->values[i])->body;
                                        unsigned char nbody5_l = ((node_data*)data_t->values[i])->len;
                                        unsigned char h5[16];
                                        hash_f(h5, nid5, seq5, nbody5, nbody5_l);
                                        tlv *tlv5 = node_hash(nid5, seq5, h5);
                                        memcpy(&pkg5->body[i * 28], tlv5, 28);
                                        free(tlv5);
                                    }
                                    rc = sendto(s, pkg5, pkg5->len, 0, &new_neigh, new_size);
                                    if (rc < 0) {
                                        perror("response to tlv network_state_request\n");
                                    }
                                    free(pkg5);
                                }
                                else {
                                    int n = hashtable_size(data_t);
                                    do {
                                        package *pkg5 = package_factory((PKG_MAX_SIZE - 4) - (PKG_MAX_SIZE - 4) % 28);
                                        for (size_t i = 0; i < (PKG_MAX_SIZE - 4) / 28; i++) {
                                            unsigned char *nid5 = ((node_id*)data_t->keys[i])->nid;
                                            uint16_t seq5 = ((node_data*)data_t->values[i])->seqno;
                                            unsigned char *nbody5 = ((node_data*)data_t->values[i])->body;
                                            unsigned char nbody5_l = ((node_data*)data_t->values[i])->len;
                                            unsigned char h5[16];
                                            hash_f(h5, nid5, seq5, nbody5, nbody5_l);
                                            tlv *tlv5 = node_hash(nid5, seq5, h5);
                                            memcpy(&pkg5->body[i * 28], tlv5, 28);
                                            free(tlv5);
                                        }
                                        rc = sendto(s, pkg5, pkg5->len, 0, &new_neigh, new_size);
                                        if (rc < 0) {
                                            perror("response to tlv network_state_request\n");
                                        }
                                        free(pkg5);
                                        n -= (PKG_MAX_SIZE - 4) / 28;
                                    } while (n * 28 > PKG_MAX_SIZE - 4);
                                    package *pkg5 = package_factory(n * 28);
                                    for (size_t i = 0; i < n; i++) {
                                        unsigned char *nid5 = ((node_id*)data_t->keys[i])->nid;
                                        uint16_t seq5 = ((node_data*)data_t->values[i])->seqno;
                                        unsigned char *nbody5 = ((node_data*)data_t->values[i])->body;
                                        unsigned char nbody5_l = ((node_data*)data_t->values[i])->len;
                                        unsigned char h5[16];
                                        hash_f(h5, nid5, seq5, nbody5, nbody5_l);
                                        tlv *tlv5 = node_hash(nid5, seq5, h5);
                                        memcpy(&pkg5->body[i * 28], tlv5, 28);
                                        free(tlv5);
                                    }
                                    rc = sendto(s, pkg5, pkg5->len, 0, &new_neigh, new_size);
                                    if (rc < 0) {
                                        perror("response to tlv network_state_request\n");
                                    }
                                    free(pkg5);
                                }
                            break;
                            case 6:
                                //node hash
                                //send network state request
                                printf("tlv node hash received\n");
                                memcpy(tlv_hash, t->body + 10, 16);
                                id_n = node_id_factory(t->body);
                                if(n != NULL){
                                    memcpy(id_n, t->body, 8);
                                    int f = find_key(data_t, id_n, 8);
                                    if(f > 0){
                                        if(memcmp(hash, tlv_hash, 16)){
                                            tlv *nod_stat_req = node_state_request(id_n);
                                            package *res6 = package_factory(8);
                                            memcpy(res6->body, nod_stat_req, res6->len);
                                            rc = sendto(s, res6, res6->len + 4, 0, &new_neigh, new_size);
                                            if(rc < 0){
                                                printf("tlv node state request not sent\n");
                                            }
                                            else{
                                                printf("tlv node state request sent\n");
                                            }
                                        }
                                    }
                                }
                            break;
                            case 7:
                                //node state request
                                //send node state
                                printf("tlv node state request received\n");
                                id_n = node_id_factory(t->body);
                                int k = find_key(data_t, id, 16);
                                if(k > 0){
                                    node_data *d = (node_data *)data_t->keys[k];
                                    tlv *nod_stat = node_state(id_n->nid, d->seqno, &hash, d->body, d->len);
                                    package *res7 = package_factory(28 + d->len);
                                    memcpy(res7->body, nod_stat, 28 + d->len);
                                    rc = sendto(s, res7, res7->len + 4, 0, &new_neigh, new_size);
                                    if(rc < 0){
                                        perror("tlv node state not sent");
                                    }
                                    else{
                                        printf("tlv node state sent\n");
                                    }
                                    free(nod_stat);
                                    free(res7);
                                }
                                else{
                                    printf("node absent from data table\n");
                                    //TODO send tlv warning
                                }
                            break;
                            case 8:
                                //node state
                                //compare hashes
                                if (1) {
                                    unsigned char *nid8 = t->body;
                                    uint16_t seq8 = ntohs((uint16_t)t->body[8]);
                                    unsigned char *data8 = &t->body[26];
                                    if (memcmp(nid8, id, 8) == 0) {
                                        if (seq8 >= seq) {
                                            seq += 1;
                                        }
                                    }
                                    else {
                                        int i = find_key(data_t, nid8, 8);
                                        if (i > -1) {
                                            if (seq > ntohs(((node_data*)data_t->values[i])->seqno)) {
                                                node_data *nd8 = node_data_factory(seq8, t->len - 26, data8);
                                                update(data_t, nid8, 8, nd8);
                                                hash_data_t(net_hash, data_t);
                                            }
                                        }
                                        else {
                                            node_data *nd8 = node_data_factory(seq8, t->len - 26, data8);
                                            update(data_t, nd8, 8, nd8);
                                            hash_data_t(net_hash, data_t);
                                        }
                                    }
                                }
                            break;
                            case 9:
                                printf("warning tlv received : %s\n", t->body);
                                break;
                            break;
                            default:
                                //ignore
                            break;
                        }
                        t = t + t->len;
                    }
                }
                else{
                    printf("uncorrect package format : package ignored\n");
                }
            }
        }
        else if(FD_ISSET(s, &expc)){
            //perror errno
            perror(strerror(errno));
        }
        else{
            //timeval over
            printf("timeout\n");
            for(int i = 0; i < hashtable_size(neighbour_t); i++){
                node_sock *sock  = (node_sock *)neighbour_t->keys[i];
                node_neigh *neigh =  (node_neigh *)neighbour_t->values[i];
                if(!neigh->keep && difftime(time(NULL), neigh->date) > 70){
                    delete(neighbour_t, sock, sizeof(sock));
                    printf("deleting node that didn'nt sent package in 70s\n");
                }
            }
            if(hashtable_size(neighbour_t) < 5){
                //send neighbour_request to one random neighbour
                printf("preparing neighbour request\n");
                struct sockaddr_in6 random;
                memset(&random, 0, sizeof(random));
                random.sin6_family = AF_INET6;
                srand(time(NULL));
                int r = rand() % hashtable_size(neighbour_t);
                node_sock *tmp = (node_sock *)neighbour_t->keys[r];
                rc = inet_pton(AF_INET6, (char *)tmp->ip, &random.sin6_addr);
                if(rc < 0){
                    perror("inet_pton()");
                    exit(1);
                }
                random.sin6_port = htons(tmp->port);
                tlv *nr = neighbour_request();
                package *p = package_factory(0);
                memcpy(p->body, nr, 2);
                rc = sendto(s, (void *)p, 2, 0, &random, sizeof(random));
                if(rc < 0){
                    perror("failed to send tlv neighbour request");
                }
                else{
                    printf("tlv neighbour request to random neighbour sent\n");
                }
            }
            for(int i = 0; i < hashtable_size(neighbour_t); i++){
                struct sockaddr_in6 trst;
                memset(&trst, 0, sizeof(trst));
                node_sock *sock_id = neighbour_t->keys[i];
                rc = inet_pton(AF_INET6, (char *)sock_id->ip, &trst.sin6_addr);
                if(rc < 0){
                    perror("inet_pton()");
                    exit(1);
                }
                trst.sin6_port = htons(port);
                tlv *t = network_hash(&net_hash);
                package *pkg = package_factory(16);
                memcpy(pkg->body, t, pkg->len);
                rc = sendto(s, pkg, pkg->len + 4, 0, &trst, sizeof(trst));
                if(rc < 0){
                    printf("tlv netwotk hash not sent\n");
                }
                else{
                    printf("network hash sent\n");
                }
            }
        }
    }
    free(neighbour_t);
    free(data_t);
    close(s);
    return 0;
}
