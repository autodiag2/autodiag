#ifndef __SIM_ELM327_NETWORK_H
#define __SIM_ELM327_NETWORK_H

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

#include "libautodiag/com/network.h"

#define ELM327_NETWORK_PORT 35000
bool sim_elm327_network_is_connected(void * implPtr);

#endif