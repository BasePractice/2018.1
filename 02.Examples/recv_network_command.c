#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <network.h>

#define BUFFER_SIZE 1024

/** 10.0.75.200 8000 PXA100013026026 */

int main(int argc, char **argv) {
    int i;
    struct Server *server;
    network_init();

    /**  */
    server = server_create(argv[1], client_handler);

    server_start(server);
    server_wait(server, 10000);
    server_destroy(server);

    network_destroy();
    return EXIT_SUCCESS;
}
