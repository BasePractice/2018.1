#include <catch2/catch.hpp>
#include "console_gui.h"


TEST_CASE("[01.ConsoleGui]", "Инициализация") {
    struct Console *c;
    REQUIRE( console_init(&c) );
}

