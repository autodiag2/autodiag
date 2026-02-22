#ifndef __SIM_DOIP_DISCOVER_H
#define __SIM_DOIP_DISCOVER_H

#include "libautodiag/sim/doip/doip.h"

OBJECT_H(SimDoIPServer,
    SimDoIp * sim;
    sock_t handle;
    int port;
);

void sim_doip_discover_start(SimDoIp * sim);

#endif