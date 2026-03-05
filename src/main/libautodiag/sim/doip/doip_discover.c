#include "libautodiag/sim/doip/doip_discover.h"

void * sim_doip_discovery_loop(void *arg) {
    SimDoIp * sim = (SimDoIp*)arg;
    DoIpImplementation * implementation = ((DoIpImplementation*)sim->implementation);
    sock_t handle = implementation->disc_server_handle;
    int port = object_handle_t_get_port(implementation->server_handle);
    if ( port == -1 ) {
        log_msg(LOG_ERROR, "Failed to get discovery server port");
        return null;
    }
    long last_broadcast_ms = -1;
    final int timeout_waiting_client_ms = implementation->broadcast_time_ms/10;
    Buffer * annoucement = buffer_from_ascii_hex("000102030405060708090A0B0C0D0E0F");
    while ( sim_doip_should_continue(sim) ) {
        if ( last_broadcast_ms == -1 || (time_ms() - last_broadcast_ms) > implementation->broadcast_time_ms ) {
            last_broadcast_ms = time_ms();
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

            log_msg(LOG_DEBUG, "Annoucement sent");
            sendto(handle, annoucement->buffer, annoucement->size, 0,
                (struct sockaddr*)&addr, sizeof(addr));
        } 
        int r = network_udp_wait_readable(handle, timeout_waiting_client_ms);
        if (0 < r) {
            Buffer * request = buffer_new();
            buffer_ensure_capacity(request, 1024);
            struct sockaddr_in from;
            socklen_t from_len = (socklen_t)sizeof(from);
            int n = (int)recvfrom(handle, request->buffer, request->size_allocated, 0, (struct sockaddr *)&from, &from_len);
            if (n <= 0) continue;
    
            sendto(handle, annoucement->buffer, annoucement->size, 0, (struct sockaddr *)&from, from_len);
            char * addr_str = network_location(from);
            log_msg(LOG_DEBUG, "Annoucement sent to %s", addr_str);
            free(addr_str);
        }
    }
    return NULL;
}
/**
 * Start the broadcast listening and sending server
 */
void sim_doip_discover_start(SimDoIp * sim) {
    int discovery_loop_port = -1;
    
    sock_t discovery_loop_handle = network_udp_start(&discovery_loop_port, DOIP_NETWORK_PORT);
    if ( discovery_loop_handle == SOCK_T_INVALID ) {
        log_msg(LOG_ERROR, "Failed to bind the discovery server");
        perror("doip network_udp_start");
        return;
    }
    DoIpImplementation * impl = (DoIpImplementation*)sim->implementation;
    impl->disc_server_handle = discovery_loop_handle;
    network_udp_set_reuseaddr(discovery_loop_handle);
    network_udp_enable_broadcast(discovery_loop_handle);
    log_msg(LOG_DEBUG, "Listening on UDP");
    pthread_create(&impl->discovery_thread, null, sim_doip_discovery_loop, (void*)sim);
}