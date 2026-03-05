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
#   define socklen_t int
#   define SOCK_T_INVALID INVALID_SOCKET
#   define close_sock closesocket
#else
#   warning Unsupported OS
#endif

#define NETWORK_BACKLOG 10
char * network_location(struct sockaddr_in caddr);
sock_t network_tcp_start(int *bound_port, int start_port, int backlog);
sock_t network_udp_start(int *bound_port, int start_port);
int network_udp_enable_broadcast(sock_t s);
int network_udp_set_reuseaddr(sock_t s);
int network_udp_wait_readable(sock_t s, int timeout_ms);
void network_stop(sock_t s);
/**
 * Get the port to which a socket is bound
 * @return -1 on error else port bound to
 */
int sock_t_get_port(sock_t s);

#endif