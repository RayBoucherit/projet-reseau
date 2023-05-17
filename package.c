#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "package.h"
#include "tlv.h"
#define PKG_MAX_SIZE 1024

package* package_factory(uint16_t n)
{
    if (n > PKG_MAX_SIZE - 4) return NULL;
    package* p = malloc(4 + n);
    if (p == NULL) return NULL;
    p->magic = MAGIC;
    p->version = VERSION;
    p->len = htons(n);
    return p;
}

uint16_t package_size(package* pkg)
{
    if (pkg == NULL) return 0;
    return pkg->len + 4;
}

tlv *package_body(package *pkg){
    tlv *ret = (tlv *)pkg->body;
    return ret;
}

int package_check(package* pkg, int rc)
{
    if(pkg == NULL){
        return 0;
    }
    int i = 4;
    //package head
    int len;
    if(pkg->magic == MAGIC && pkg->version == VERSION && rc <= pkg->len + 4 && rc <= PKG_MAX_SIZE){
        tlv *tmp = (tlv *)pkg->body;
        while(tmp != NULL){
            if(!parse_tlv(tmp)) return 0;
            len = (int)tmp->len;
            tmp = tmp + (2 + len);
            i+= tmp->len;
        }
        //package len is correct
        //and each tlv is correct
        if(i == rc) {
            printf("package parsed\n");
            return 1;
        }
    }
    printf("failed to parse package\n");
    return 0;
}
