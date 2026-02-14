#ifndef __SIM_DOIP_H
#define __SIM_DOIP_H

#include "libautodiag/compile_target.h"

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#ifdef OS_POSIX
#   include <arpa/inet.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#endif
#include "libautodiag/thread.h"
#include "libautodiag/lib.h"
#include "libautodiag/com/serial/elm/elm327/elm327.h"
#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/sim/sim.h"

typedef struct {
    #ifdef OS_WINDOWS
        #ifdef OS_POSIX
            int network_handle;
        #endif
        int server_fd;
        sock_t client_socket;
    #elif defined OS_POSIX
        int handle;
        int server_fd;
    #else
    #   warning OS unsupported
    #endif
} DoIpImplementation;

typedef struct _SimDoIp {
    Sim;
    DoIpImplementation * implementation;
} SimDoIp;

#define DOIP_NETWORK_PORT 13400
SimDoIp * sim_doip_new();
void sim_doip_loop(SimDoIp * doip);

#endif