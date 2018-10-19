#include <catch2/catch.hpp>
#include <cstring>
#include "protocol.h"
#include "network.h"

#define MACHINE_ADDRESS "192.168.250.1"
#define MACHINE_PORT    "8000"

struct Command {
    const char *command;
} commands[] = {
        {"PXA010013026026"},
        {"PYA020013026026"},
        {"PXA050013026026"},
        {"PYA050013026026"},
        {"PXA051013026026PXA051013026026"},
};

TEST_CASE("[03.Network]", "Клиент") {
    network_init();
    Client *c = client_create(MACHINE_ADDRESS, MACHINE_PORT);
    REQUIRE(client_connect(c) == 1);
    for (auto &command : commands) {
        size_t size = strlen(command.command);
        client_request(c, (uint8_t *) command.command, size, (RESPONSE_HANDLE)client_wait_symbols, (void *)8);
    }
    client_destroy(c);
    network_destroy();
}