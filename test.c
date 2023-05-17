#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    int s, rc;
    s = socket(AF_INET6, SOCK_DGRAM, 0);
    if(s < 0){
        perror()
    }
    return 0;
}
