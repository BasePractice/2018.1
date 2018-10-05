#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>

typedef void (*RESPONSE_HANDLE)(SOCKET, void *);

/** 192.168.250.1 8000 PXA100013026026 */

int main(int argc, char **argv) {
    int i;
    struct Client *c;
    network_init();
    c = client_create(argv[1], argv[2]);
    client_connect(c);
    client_request(c,
            (uint8_t *) argv[3],
            strlen(argv[3]),
            (RESPONSE_HANDLE)client_wait_symbols,
            (void *)8);
    client_destroy(c);
    network_destroy();
    return EXIT_SUCCESS;
}
