cmake_minimum_required(VERSION 3.2.1)
project(03.Network)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)

set(SOURCE_FILES network.c network.h network_common.h protocol.c protocol.h)
set(SOURCE_TEST_FILES network_tests.cpp)

if (WIN32)
    set(SYSTEM_LIBRARIES ws2_32)
endif ()

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
add_executable(${PROJECT_NAME}_Tests ${SOURCE_FILES} ${SOURCE_TEST_FILES})
target_link_libraries(${PROJECT_NAME} ${SYSTEM_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_Tests  tests_main ${SYSTEM_LIBRARIES})

