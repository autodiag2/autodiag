#ifndef __SIM_ELM327_NETWORK_H
#define __SIM_ELM327_NETWORK_H

#include "libautodiag/lib.h"

#ifdef OS_WINDOWS
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET sock_t;
#   define close_sock closesocket
#else
#   include <unistd.h>
#   include <errno.h>
#   include <string.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
    typedef int sock_t;
#   include <unistd.h>
#   define close_sock close
#endif

#define ELM327_NETWORK_PORT 35000
#define ELM327_CONNECTION_BACKLOG 10
int start_inet_server_any(int *bound_port);
char * network_client_location(struct sockaddr_in caddr);

#endif