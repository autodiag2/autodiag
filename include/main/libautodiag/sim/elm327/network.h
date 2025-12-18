#ifndef __SIM_ELM327_NETWORK_H
#define __SIM_ELM327_NETWORK_H

#include "libautodiag/lib.h"

#ifdef OS_WINDOWS
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   pragma comment(lib, "Ws2_32.lib")
    typedef SOCKET sock_t;
#else
#   define OS_POSIX
#   include <unistd.h>
#   include <errno.h>
#   include <string.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
    typedef int sock_t;
#endif

int start_inet_server_any(int preferred_port, int backlog, int *bound_port);
sock_t accept_client(sock_t server_fd, char *client_ip, int ip_len, int *client_port);

#endif