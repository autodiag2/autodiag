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
    typedef int sock_t;
#   include <unistd.h>
#   define close_sock close
#elif defined OS_WINDOWS
#   pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET sock_t;
#   define close_sock closesocket
#endif

#define NETWORK_BACKLOG 10
char * network_location(struct sockaddr_in caddr);
int network_tcp_start(int *bound_port, int start_port);

#endif