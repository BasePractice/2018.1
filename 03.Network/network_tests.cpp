#include <catch2/catch.hpp>
#include <cstring>
#include "protocol.h"
#include "network.h"

#define MACHINE_ADDRESS "192.168.250.1"
#define MACHINE_PORT    "8000"

struct Command {
    const char *command;
} commands[] = {
        {MACHINE_ADDRESS " " MACHINE_PORT " PXA100013026026"},
        {MACHINE_ADDRESS " " MACHINE_PORT " PYA100013026026"},
        {MACHINE_ADDRESS " " MACHINE_PORT " PXA050013026026"},
        {MACHINE_ADDRESS " " MACHINE_PORT " PYA050013026026"},
        {MACHINE_ADDRESS " " MACHINE_PORT " PXA051013026026PXA051013026026"},
};

TEST_CASE("[03.Network]", "Клиент") {
    network_init();
    Client *c = client_create("", "");
    REQUIRE(client_connect(c));
    for (auto &command : commands) {
        size_t size = strlen(command.command);
        client_request(c, (uint8_t *) command.command, size, (RESPONSE_HANDLE)client_wait_symbols, (void *)8);
    }
    client_destroy(c);
    network_destroy();
}