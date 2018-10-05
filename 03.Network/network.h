#ifndef C_CW_PROGRAMMING_PRACTICE_NETWORK_H
#define C_CW_PROGRAMMING_PRACTICE_NETWORK_H
#include <stdlib.h>
#include <stdint.h>
#include "network_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct Server;
struct Client;
struct ServerResponse;

int milliseconds_now();
void network_init();
void network_destroy();
int  network_last_error();

typedef void (*RESPONSE_HANDLE)(SOCKET, void *);

void socket_close(SOCKET sock);

struct Client *client_create(const char *hostname, const char *port);
int  client_connect(struct Client *client);
int  client_disconnect(struct Client *client);
void client_destroy(struct Client *client);
size_t client_request(struct Client *client,
                      const uint8_t *buffer, size_t size,
                      void (*response_handler)(SOCKET, void *), void *user_data);
void client_wait_symbols(SOCKET s, size_t symbols);
void client_dump(struct Client *client);
void dump_content(const char *identity, uint8_t *content, size_t content_len);


struct Server *server_create(const char *port,
                             void (*client_handler)(int, SOCKET, void *));
int server_start(struct Server *server);
int server_wait(struct Server *server, uint32_t milliseconds);
int server_stop(struct Server *server);
void server_destroy(struct Server *);

#if defined(__cplusplus)
}
#endif

#endif //C_CW_PROGRAMMING_PRACTICE_NETWORK_H
