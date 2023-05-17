#ifndef PKG_H
#define PKG_H
#define PKG_MAX_SIZE 1024
#define MAGIC 95
#define VERSION 1

typedef struct tlv tlv;
typedef struct package package;

struct package{
    unsigned char magic;
    unsigned char version;
    uint16_t len;
    char body[];
};

package* package_factory(uint16_t n);
uint16_t package_size(package* p);
int package_check(package* pkg, int rc);
package* package_gen(tlv *body[], int len);
void package_parser(package* pkg, char *body[], int rc);
tlv *package_body(package *pkg);

#endif
