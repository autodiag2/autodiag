#include "libautodiag/sim/doip/doip.h"

SimDoIp * sim_doip_new() {
    SimDoIp * sim = (SimDoIp*)malloc(sizeof(SimDoIp));
    sim_init_with_defaults((Sim*)sim);
    sim->type = strdup("doip");
    sim->implementation = (DoIpImplementation*)malloc(sizeof(DoIpImplementation));
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