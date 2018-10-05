#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "network.h"
#include "protocol.h"

#ifndef WIN32

#include <netdb.h>

#endif

enum RequestState {
    RequestReadType,
    RequestReadNext,
    RequestWriteStatus,
    RequestWriteResult,
    EndRequest,
    RequestUnknownProtocol,
    RequestNetworkError
};

#if defined(OUTPUT_DEBUG_INFO)
static inline void
write_status_message(const char *message) {
    fprintf(stdout, message);
    fprintf(stdout, "\n");
    fflush(stdout);
}
#else
static inline void
write_status_message(const char *message) {
    (void)message;
}
#endif

static inline const char *
response_status(uint8_t kode) {
    switch (kode) {
        case 0:
            return "ok";
        default:
            return "unknown";
    }
}

void
protocol_request_handler(SOCKET socket_client, void *user_data) {
    write_status_message("[server] request end");
}

enum ResponseState {
    ResponseReadStatus,
    EndResponse,
    ResponseUnknownProtocol,
    ResponseNetworkError
};

void
protocol_response_handler(SOCKET socket_server, struct ServerResponse *response) {
     write_status_message("[client] response end");
}
