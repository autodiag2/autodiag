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
#include "libautodiag/com/doip/doip.h"
#include "libautodiag/sim/ecu/ecu.h"
#include "libautodiag/sim/sim.h"

typedef struct {
    SimImplementation;
    pthread_t* loop_thread;
    bool loop_ready;
    /**
     * Read & write timeout ms
     */
    int timeout_ms;
    /**
     * Delay in ms between each broadcast
     */
    int broadcast_time_ms;
} DoIpImplementation;
#define SIM_DOIP_TIMEOUT_MS_RW 5000
#define SIM_DOIP_TIMEOUT_MS_BROADCAST 1000

typedef struct _SimDoIp {
    Sim;
} SimDoIp;

OBJECT_H(SimDoIPServer,
    SimDoIp * sim;
    sock_t handle;
    int port;
);

#define DOIP_NETWORK_PORT 13400
SimDoIp * sim_doip_new();
void sim_doip_loop(SimDoIp * doip);
void sim_doip_destroy(SimDoIp *sim);
void sim_doip_loop_as_daemon(SimDoIp * sim);
bool sim_doip_loop_daemon_wait_ready(SimDoIp *sim);

#endif