#ifndef __COM_NETWORK_H
#define __COM_NETWORK_H

#include "libautodiag/lib.h"

#if defined OS_POSIX
#   include <unistd.h>
#   include <errno.h>
#   include <string.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
#   define SOCK_T_INVALID -1
    typedef int sock_t;
#   include <unistd.h>
#   define close_sock close
#elif defined OS_WINDOWS
#   pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET sock_t;
#   define int socklen_t
#   define SOCK_T_INVALID INVALID_SOCKET
#   define close_sock closesocket
#endif

#define NETWORK_BACKLOG 10
char * network_location(struct sockaddr_in caddr);
sock_t network_tcp_start(int *bound_port, int start_port);
sock_t network_udp_start(int *bound_port, int start_port);

#endif