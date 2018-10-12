cmake_minimum_required(VERSION 3.2.1)
project(02.Examples)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)

add_executable(${PROJECT_NAME}_ConsoleGui console_gui_show.c)
add_executable(${PROJECT_NAME}_NetworkCommand send_network_command.c)
add_executable(${PROJECT_NAME}_NetworkServer recv_network_command.c)

target_link_libraries(${PROJECT_NAME}_ConsoleGui 01.ConsoleGui ${CURSES_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_NetworkCommand 03.Network -lpthread)
target_link_libraries(${PROJECT_NAME}_NetworkServer 03.Network -lpthread)

