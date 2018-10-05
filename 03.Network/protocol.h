#ifndef C_CW_PROGRAMMING_PRACTICE_SERVICE_PROTOCOL_H
#define C_CW_PROGRAMMING_PRACTICE_SERVICE_PROTOCOL_H

#include <stdlib.h>
#include <stdint.h>
#include "network_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

enum RequestType {
    RequestUnknown
};
enum ResponseType {
    ResponseUnknown,
};

enum ServiceStatus {
    StatusOk                = 0,
    StatusFailed            = 1
};

struct ServerResponse {
    enum ResponseType  response_type;
    enum ServiceStatus status;
};

void protocol_request_handler(SOCKET socket_client, void *user_data);
void protocol_response_handler(SOCKET socket_server, struct ServerResponse *response);

#if defined(__cplusplus)
}
#endif

#endif //C_CW_PROGRAMMING_PRACTICE_SERVICE_PROTOCOL_H
