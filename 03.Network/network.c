#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "protocol.h"

#if defined(WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>

#endif

#include "network.h"

int
milliseconds_now() {
#if defined(WIN32)
#if 0
    static const LARGE_INTEGER s_frequency;
    static const BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    } else {
        return GetTickCount();
    }
#else
    return (int) GetTickCount();
#endif
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

void
network_destroy() {
#if defined(WIN32)
    WSACleanup();
#endif
}

void
network_init() {
#if defined(WIN32)
    WSADATA wsa_data;
    int ret;

    ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", ret);
    }
#endif
}

SOCKET socket_listen_server(const char *port);

void
socket_close(SOCKET sock) {
#if defined(WIN32)
    closesocket(sock);
#else
    close(sock);
#endif
}

int
network_last_error() {
#if defined(WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}

SOCKET
socket_listen_server(const char *port) {
    struct addrinfo *result = NULL, hints;
    int ret;
    int option = 1;
    SOCKET listen_socket;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    ret = getaddrinfo(NULL, port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "[server] getaddrinfo failed: %d\n", ret);
        return INVALID_SOCKET;
    }
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == -1) {
        fprintf(stderr, "[server] error at socket(): %i\n", network_last_error());
        freeaddrinfo(result);
        return INVALID_SOCKET;
    }
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &option, sizeof(option));
    setsockopt(listen_socket, SOL_SOCKET, SO_DEBUG, (const char *) &option, sizeof(option));
    ret = bind(listen_socket, result->ai_addr, (int) result->ai_addrlen);
    if (ret == -1) {
        fprintf(stderr, "[server] bind failed with error: %d\n", network_last_error());
        freeaddrinfo(result);
        socket_close(listen_socket);
        return INVALID_SOCKET;
    }
    if (listen(listen_socket, SOMAXCONN) == -1) {
        fprintf(stderr, "[server] listen failed with error: %i\n", network_last_error());
        socket_close(listen_socket);
        return INVALID_SOCKET;
    }
    freeaddrinfo(result);
    return listen_socket;
}

int
socket_send_client(const char *hostname, const char *port, const char *send_buf) {
    struct addrinfo *result = NULL, *it = NULL, hints;
    SOCKET socket_client = INVALID_SOCKET;
    char recv_buf[1024];
    int ret;
    long long start = 0;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    start = milliseconds_now();
    ret = getaddrinfo(hostname, port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "[client] getaddrinfo failed with error: %d\n", ret);
        return 0;
    }

    for (it = result; it != NULL; it = it->ai_next) {
        socket_client = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (socket_client == -1) {
            fprintf(stderr, "[client] socket failed with error: %i\n", network_last_error());
            return 0;
        }
        ret = connect(socket_client, it->ai_addr, (int) it->ai_addrlen);
        if (ret == -1) {
            socket_close(socket_client);
            socket_client = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (socket_client == -1) {
        fprintf(stderr, "[client] unable to connect to server!\n");
        return 0;
    }
    ret = (int) send(socket_client, send_buf, (int) strlen(send_buf), 0);
    if (ret == -1) {
        printf("[client] send failed with error: %d\n", network_last_error());
        socket_close(socket_client);
        return 0;
    }
    ret = shutdown(
            socket_client,
#if defined(WIN32)
            SD_SEND
#else
            SHUT_RD
#endif
    );
    if (ret == -1) {
        fprintf(stderr, "[client] shutdown failed with error: %d\n", network_last_error());
        socket_close(socket_client);
        return 0;
    }
    do {
        ret = (int) recv(socket_client, recv_buf, sizeof(recv_buf), 0);
        if (ret > 0) {
            recv_buf[ret] = 0;
            /*fprintf(stdout, "Recv: %s\n", recv_buf);*/
        } else if (ret == 0) {
            /*fprintf(stdout, "Connection closed\n");*/
        } else {
            fprintf(stderr, "[client] recv failed with error: %d\n", network_last_error());
        }

    } while (ret > 0);
    socket_close(socket_client);
#if defined(OUTPUT_DEBUG_INFO)
    fprintf(stdout, "[client] elapsed %llims\n", milliseconds_now() - start);
#endif
    return 1;
}

struct Server {
    int id;
    char *port;
    volatile bool running;
#if defined(WIN32)
    DWORD thread_id;
    HANDLE handle;
#else
    pthread_t handle;
    pthread_mutex_t running_mut;
    pthread_cond_t running_cond;
#endif

    void (*client_handler)(int, SOCKET, void *);
};

static void
client_default_handler(int id, SOCKET socket_client, void *user_data) {
    int ret;
    long long start = 0;

    start = milliseconds_now();
    protocol_request_handler(socket_client, user_data);
    ret = shutdown(
            socket_client,
#if defined(WIN32)
            SD_SEND
#else
            SHUT_RDWR
#endif
    );
    if (ret == SOCKET_ERROR) {
        /*fprintf(stderr, "[server] shutdown failed: %d\n", network_last_error());*/
        socket_close(socket_client);
        return;
    }
    socket_close(socket_client);
#if defined(OUTPUT_DEBUG_INFO)
    fprintf(stdout, "[server] elapsed %llims\n", milliseconds_now() - start);
#endif
}

struct Server *
server_create(const char *port,
              void (*client_handler)(int, SOCKET, void *)) {
    struct Server *server = calloc(1, sizeof(struct Server));
    server->running = TRUE;
    server->port = strdup(port);
    server->client_handler = (client_handler == NULL ? client_default_handler : client_handler);
#ifndef WIN32
    pthread_mutex_init(&server->running_mut, NULL);
    pthread_cond_init(&server->running_cond, NULL);
#endif
    return server;
}

void
server_destroy(struct Server *server) {
    if (server != NULL) {
        server_stop(server);
        free(server->port);
        server->port = NULL;
#if defined(WIN32)
        if (server->handle != NULL && server->handle != INVALID_HANDLE_VALUE) {
            TerminateThread(server->handle, 0);
            CloseHandle(server->handle);
        }
        server->handle = NULL;
#else
#endif
    }
}

#if defined(WIN32)

static DWORD WINAPI
start_server_windows_thread(LPVOID parameter) {
    struct Server *server = (struct Server *) parameter;
    SOCKET listen_socket = socket_listen_server(server->port);
    if (listen_socket != INVALID_SOCKET) {
        FD_SET read_set;
        SOCKET client_socket;
        struct sockaddr_in sock_addr_in;
        int sock_addr_len = sizeof(sock_addr_in);
        int non_block = 1;

#if defined(OUTPUT_DEBUG_INFO)
        fprintf(stdout, "[server] starting\n");
#endif
        ioctlsocket(listen_socket, FIONBIO, (u_long *) &non_block);
        while (server->running) {
            struct timeval time = {1, 0};
            FD_ZERO(&read_set);
            FD_SET(listen_socket, &read_set);

            if (select((int) (listen_socket + 1), &read_set, NULL, NULL, &time) != SOCKET_ERROR) {
                if (FD_ISSET(listen_socket, &read_set)) {
                    client_socket = accept(listen_socket, (struct sockaddr *) &sock_addr_in,
                                           (socklen_t *) &sock_addr_len);
                    if (client_socket == -1) {
                        fprintf(stderr, "[server] accept failed: %d\n", network_last_error());
                        socket_close(listen_socket);
                        break;
                    }
#if defined(OUTPUT_DEBUG_INFO)
                    fprintf(stdout, "[server] accept %d.%d.%d.%d\n",
                            sock_addr_in.sin_addr.S_un.S_un_b.s_b1,
                            sock_addr_in.sin_addr.S_un.S_un_b.s_b2,
                            sock_addr_in.sin_addr.S_un.S_un_b.s_b3,
                            sock_addr_in.sin_addr.S_un.S_un_b.s_b4
                    );
#endif
                    non_block = 0;
                    ioctlsocket(client_socket, FIONBIO, (u_long *) &non_block);
                    (*server->client_handler)(++server->id, client_socket, server->manager);
                }
            } else {
                fprintf(stderr, "[server] select failed: %d\n", network_last_error());
            }
        }
#if defined(OUTPUT_DEBUG_INFO)
        fprintf(stdout, "[server] stopping\n");
#endif
        socket_close(listen_socket);
        return 0;
    }
    return 255;
}

#else

void *
start_server_unix(void *parameter) {
    struct Server *server = (struct Server *) parameter;
    SOCKET listen_socket = socket_listen_server(server->port);
    if (listen_socket != INVALID_SOCKET) {
        fd_set read_set;
        SOCKET client_socket;
        struct sockaddr_in sock_addr_in;
        int sock_addr_len = sizeof(sock_addr_in);

        fprintf(stdout, "[server] starting\n");
        fcntl(listen_socket, F_SETFL, fcntl(listen_socket, F_GETFL, 0) | O_NONBLOCK);
        while (server->running) {
            struct timeval time = {1, 0};
            FD_ZERO(&read_set);
            FD_SET(listen_socket, &read_set);

            if (select(listen_socket + 1, &read_set, NULL, NULL, &time) != SOCKET_ERROR) {
                if (FD_ISSET(listen_socket, &read_set)) {
                    client_socket = accept(listen_socket, (struct sockaddr *) &sock_addr_in,
                                           (socklen_t *) &sock_addr_len);
                    if (client_socket == -1) {
                        fprintf(stderr, "[server] accept failed: %d\n", network_last_error());
                        socket_close(listen_socket);
                        break;
                    }
                    fprintf(stdout, "[server] accept %d.%d.%d.%d\n",
                            ((char *) &sock_addr_in.sin_addr.s_addr)[0],
                            ((char *) &sock_addr_in.sin_addr.s_addr)[1],
                            ((char *) &sock_addr_in.sin_addr.s_addr)[2],
                            ((char *) &sock_addr_in.sin_addr.s_addr)[3]
                    );
                    fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0) & (~O_NONBLOCK));
                    (*server->client_handler)(++server->id, client_socket, 0);
                }
            } else {
                fprintf(stderr, "[server] select failed: %d\n", network_last_error());
            }
        }
        fprintf(stdout, "[server] stopping\n");
        socket_close(listen_socket);
    }
    pthread_mutex_lock(&server->running_mut);
    pthread_cond_signal(&server->running_cond);
    pthread_mutex_unlock(&server->running_mut);
    pthread_exit(NULL);
    return NULL;
}

#endif

int
server_start(struct Server *server) {
    if (server == NULL)
        return 0;
#if defined(WIN32)
    server->handle = CreateThread(NULL, 0, start_server_windows_thread, server, 0, &server->thread_id);
    return server->handle != NULL;
#else
    return pthread_create(&server->handle, NULL, start_server_unix, server) == 0;
#endif
}

int
server_wait(struct Server *server, uint32_t milliseconds) {
    if (server == NULL)
        return -1;
#if defined(WIN32)
    switch (WaitForSingleObject(server->handle, milliseconds)) {
        case WAIT_TIMEOUT:
            return 0;
        case WAIT_OBJECT_0:
            return 1;
        default:
            return -1;
    }
#else
    struct timespec ts;
    int rc;

    pthread_mutex_lock(&server->running_mut);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += milliseconds * 1000;
    rc = pthread_cond_timedwait(&server->running_cond, &server->running_mut, &ts);
    pthread_mutex_unlock(&server->running_mut);
    return rc != ETIMEDOUT;
#endif
}

int
server_stop(struct Server *server) {
    if (server != NULL) {
        server->running = FALSE;
#if defined(OUTPUT_DEBUG_INFO)
        fprintf(stdout, "[server] running %s\n", server->running ? "true" : "false");
#endif
        server_wait(server, 100);
    }
    return 1;
}

struct Client {
    char *port;
    char *hostname;
    SOCKET socket_server;
    int connect_time;
    int request_time;
    int last_error;
};

struct Client *
client_create(const char *hostname, const char *port) {
    struct Client *client = (struct Client *) calloc(1, sizeof(struct Client));
    client->hostname = strdup(hostname);
    client->port = strdup(port);
    return client;
}

void
client_destroy(struct Client *client) {
    if (client != NULL) {
        free(client->hostname);
        free(client->port);

        client_disconnect(client);
    }
}

int
client_connect(struct Client *client) {
    struct addrinfo *result = NULL, *it = NULL, hints;
    int ret;
    int start = 0;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    start = milliseconds_now();
    client->connect_time = 0;
    ret = getaddrinfo(client->hostname, client->port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "[client] getaddrinfo failed with error: %d\n", ret);
        return 0;
    }

    for (it = result; it != NULL; it = it->ai_next) {
        client->socket_server = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (client->socket_server == INVALID_SOCKET) {
            fprintf(stderr, "[client] socket failed with error: %i\n", network_last_error());
            return 0;
        }
        ret = connect(client->socket_server, it->ai_addr, (int) it->ai_addrlen);
        if (ret == -1) {
            socket_close(client->socket_server);
            client->socket_server = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    if (client->socket_server == INVALID_SOCKET) {
        fprintf(stderr, "[client] Unable to connect to server!\n");
        return 0;
    }
    client->connect_time = milliseconds_now() - start;
    return 1;
}

int
client_disconnect(struct Client *client) {
    shutdown(client->socket_server,
#if defined(WIN32)
             SD_SEND
#else
            SHUT_RDWR
#endif
    );
    socket_close(client->socket_server);
    client->socket_server = INVALID_SOCKET;
    return 1;
}

size_t
client_request(struct Client *client,
               const uint8_t *buffer, size_t size,
               void (*response_handler)(SOCKET, struct ServerResponse *), void *user_data) {
    if (client != NULL && buffer != NULL) {
        int start = milliseconds_now();
        int ret = (int) send(client->socket_server, (const char *) buffer, (int) size, 0);
        if (ret == -1) {
            fprintf(stderr, "[client] send failed with error: %d\n", network_last_error());
            client->request_time = milliseconds_now() - start;
            client->last_error = network_last_error();
            return 0;
        }
#if defined(OUTPUT_DEBUG_INFO)
        fprintf(stdout, "[client] sent %u bytes\n", ret);
#endif
        (*response_handler)(client->socket_server, user_data);
        client->request_time = milliseconds_now() - start;
        client->last_error = 0;
        return (size_t) ret;
    }
    return 0;
}


void
client_dump(struct Client *client) {
#if defined(OUTPUT_DEBUG_INFO)
    if (client != NULL) {
        fprintf(stdout, "[client] connect %dms, request: %dms, failed: %s\n",
                client->connect_time, client->request_time, client->last_error != 0 ? "true" : "false");
        fflush(stdout);
    }
#endif
}

void
dump_content(const char *identity, uint8_t *content, uint64_t content_len) {
    uint64_t i;
    fprintf(stdout, "[%s] Content(%llu):", identity, content_len);
    for (i = 0; i < content_len && i < 256; ++i) {
        if (i % 60 == 0) {
            fprintf(stdout, "\n");
            fflush(stdout);
        }
        fprintf(stdout, "%02X ", content[i]);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
    fflush(stdout);
}


void client_wait_symbols(SOCKET s, size_t symbols) {
    int ret;
    int it = 0;
    char *recv_buf = calloc(1, symbols + 1);

    while (it < symbols) {
        ret = (int) recv(s, recv_buf + it, symbols, 0);
        fprintf(stdout, "%.*s", ret, recv_buf + it);
        it += ret;
    }
    fflush(stdout);
    free(recv_buf);
}

