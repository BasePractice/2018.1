#ifndef C_CW_PROGRAMMING_PRACTICE_NETWORK_COMMON_H
#define C_CW_PROGRAMMING_PRACTICE_NETWORK_COMMON_H
#if defined(WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#define SOCKET int
#define INVALID_SOCKET  (~0)
#define SOCKET_ERROR    (~0)
#define TRUE      1
#define FALSE     0
#define closesocket     close
#endif
#endif //C_CW_PROGRAMMING_PRACTICE_NETWORK_COMMON_H
