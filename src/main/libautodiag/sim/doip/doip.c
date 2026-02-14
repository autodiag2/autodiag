#include "libautodiag/sim/doip/doip.h"

SimDoIp * sim_doip_new() {
    SimDoIp * sim = (SimDoIp*)malloc(sizeof(SimDoIp));
    sim->type = strdup("doip");
    sim->ecus = list_SimECU_new();
    sim->implementation = (DoIpImplementation*)malloc(sizeof(DoIpImplementation));
    final SimECU *ecu = sim_ecu_new(0xE8);
    list_SimECU_append(LIST_SIM_ECU(sim->ecus),ecu);
    sim->device_location = null;
    return sim;
}

void sim_doip_loop(SimDoIp * doip) {
    #ifdef OS_WINDOWS
        #ifdef OS_POSIX
            doip->implementation->network_handle = -1;
            doip->implementation->client_socket = -1;
        #else
            doip->implementation->client_socket = INVALID_SOCKET;
        #endif
        doip->implementation->server_fd = -1;
        int boundPort = -1;
        int serverFD = network_start(&boundPort, DOIP_NETWORK_PORT);
        if ( serverFD == -1 ) {
            log_msg(LOG_ERROR, "Failed to start server");
            perror("doip network_start");
            return;
        }
        assert(boundPort != -1);
        doip->implementation->server_fd = serverFD;
        asprintf(&doip->device_location, "0.0.0.0:%d", boundPort);
    #elif defined OS_POSIX
        doip->implementation->handle = -1;
        doip->implementation->server_fd = -1;
        int boundPort = -1;
        int serverFD = network_start(&boundPort, DOIP_NETWORK_PORT);
        if ( serverFD == -1 ) {
            log_msg(LOG_ERROR, "Failed to start server");
            perror("doip: network_start");
            return;
        }
        assert(boundPort != -1);
        doip->implementation->server_fd = serverFD;
        asprintf(&doip->device_location, "0.0.0.0:%d", boundPort);
    #else
    #   warning OS unsupported
    #endif

    log_msg(LOG_INFO, "sim running on %s", doip->device_location);

}