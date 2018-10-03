cmake_minimum_required(VERSION 3.2.1)
project(01.ConsoleGui)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(.)

set(HEADER_FILES console_gui.h)
set(SOURCE_FILES console_gui.c)

if (APPLE)
    find_package(Curses REQUIRED)
    list(APPEND SOURCE_FILES console_gui_curses.c)
else(WIN32)
    list(APPEND SOURCE_FILES console_gui_windows.c)
endif ()


include_directories(${CURSES_INCLUDE_DIR})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
add_executable(${PROJECT_NAME}_Tests console_gui_tests.cpp)

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_Tests tests_main ${PROJECT_NAME} ${CURSES_LIBRARIES})

