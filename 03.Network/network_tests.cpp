#include <catch2/catch.hpp>
#include <cstring>
#include "protocol.h"
#include "network.h"

struct Command {
    char *command;
} commands[] = {
        {""}
};

void response_handler(SOCKET s, struct ServerResponse *response) {

}

TEST_CASE("[03.Network]", "Клиент") {
    network_init();
    Client *c = client_create("", "");
    REQUIRE(client_connect(c));
    for (auto &command : commands) {
        size_t size = strlen(command.command);
        client_request(c, (uint8_t *) command.command, size, response_handler, 0);
    }
    client_destroy(c);
    network_destroy();
}