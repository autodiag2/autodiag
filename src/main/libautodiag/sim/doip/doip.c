#include "libautodiag/sim/doip/doip.h"

SimDoIp * sim_doip_new() {
    SimDoIp * sim = (SimDoIp*)malloc(sizeof(SimDoIp));
    sim_init_with_defaults((Sim*)sim);
    sim->type = strdup("doip");
    sim->implementation = (DoIpImplementation*)malloc(sizeof(DoIpImplementation));
    sim->implementation->loop_thread = null;
    sim->implementation->loop_ready = false;
    sim->implementation->timeout_ms = 5000;
    return sim;
}

bool sim_doip_network_is_connected(void * implPtr) {
    assert(implPtr != null);
    DoIpImplementation * impl = (DoIpImplementation*)implPtr;
    #ifdef OS_POSIX
        #ifdef OS_WINDOWS
            sock_t handle = impl->network_handle;
        #else        
            sock_t handle = impl->handle;
        #endif
        if ( handle == -1 ) {
            return false;
        }
    #elif defined OS_WINDOWS
        sock_t handle = impl->client_socket;
        if ( handle == INVALID_SOCKET ) {
            return false;
        }
    #else
    #   warning Unsupported OS
    #endif
    char buf;
    ssize_t ret = recv(handle, &buf, 1, MSG_PEEK);
    if (ret == 0) return false; // connection closed by peer
    #if defined OS_POSIX
    #   include <fcntl.h>
    #   include <errno.h>
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return 1; // still connected, no data
            return false; // error, consider disconnected
        }
    #elif defined OS_WINDOWS
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return 1;
            return false;
        }
    #else
    #   warning Unsupported OS
    #endif
    return true; // data available, connection alive
}
void sim_doip_destroy(SimDoIp *sim) {
    THREAD_CANCEL(sim->implementation->loop_thread);
    free(sim->implementation->loop_thread);
    sim->implementation->loop_thread = null;
    sim->implementation->loop_ready = false;
}
void sim_doip_loop_as_daemon(SimDoIp * sim) {
    THREAD_CANCEL(sim->implementation->loop_thread);
    sim->implementation->loop_thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( pthread_create(sim->implementation->loop_thread, NULL,
                          (void *(*) (void *)) sim_doip_loop, (void *)sim) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        free(sim->implementation->loop_thread);
        sim->implementation->loop_thread = null;
        exit(EXIT_FAILURE);
    }
}
bool sim_doip_loop_daemon_wait_ready(SimDoIp *sim) {
    return sim_loop_daemon_wait_ready(&sim->implementation->loop_ready);
}
void sim_doip_loop(SimDoIp * sim) {
    #ifdef OS_WINDOWS
        #ifdef OS_POSIX
            sim->implementation->network_handle = -1;
            sim->implementation->client_socket = -1;
        #else
            sim->implementation->client_socket = INVALID_SOCKET;
        #endif
        sim->implementation->server_fd = -1;
        int boundPort = -1;
        int serverFD = network_start(&boundPort, DOIP_NETWORK_PORT);
        if ( serverFD == -1 ) {
            log_msg(LOG_ERROR, "Failed to start server");
            perror("doip network_start");
            return;
        }
        assert(boundPort != -1);
        sim->implementation->server_fd = serverFD;
        asprintf(&sim->device_location, "0.0.0.0:%d", boundPort);
    #elif defined OS_POSIX
        sim->implementation->handle = -1;
        sim->implementation->server_fd = -1;
        int boundPort = -1;
        int serverFD = network_start(&boundPort, DOIP_NETWORK_PORT);
        if ( serverFD == -1 ) {
            log_msg(LOG_ERROR, "Failed to start server");
            perror("doip: network_start");
            return;
        }
        assert(boundPort != -1);
        sim->implementation->server_fd = serverFD;
        asprintf(&sim->device_location, "0.0.0.0:%d", boundPort);
    #else
    #   warning OS unsupported
    #endif

    log_msg(LOG_INFO, "sim running on %s", sim->device_location);
    final Buffer * recv_buffer = buffer_new();
    buffer_ensure_capacity(recv_buffer, 100);

    while(sim->implementation->loop_thread != null) {
        buffer_recycle(recv_buffer);
        if ( sim->implementation->loop_ready == false ) {
            sim->implementation->loop_ready = true;
        }
        struct sockaddr_in addr;
        if ( ! sim_doip_network_is_connected(sim->implementation) ) {
            #if defined OS_POSIX
                socklen_t addr_len = sizeof(addr);
                #ifdef OS_WINDOWS
                    sim->implementation->network_handle = accept(sim->implementation->server_fd, (struct sockaddr*)&addr, &addr_len);
                    if (sim->implementation->network_handle == -1) {
                        perror("accept");
                        return;
                    }
                #else
                    sim->implementation->handle = accept(sim->implementation->server_fd, (struct sockaddr*)&addr, &addr_len);
                    if (sim->implementation->handle == -1) {
                        perror("accept");
                        return;
                    }
                #endif
            #elif defined OS_WINDOWS
                int addr_len = sizeof(addr);
                sim->implementation->client_socket = accept(sim->implementation->server_fd, (struct sockaddr*)&addr, &addr_len);
                if (sim->implementation->client_socket == -1) {
                    perror("accept");
                    return;
                }
            #else
            #   warning Unsupported OS
            #endif
            char * location = network_location(addr);
            log_msg(LOG_INFO, "Client %s connected", location);
            free(location);
        }

        if ( sim_read((Sim*)sim, sim->implementation->timeout_ms, recv_buffer) == -1 ) {
            log_msg(LOG_ERROR, "Error during reception, exiting the loop");
            return;
        }
        if ( recv_buffer->size <= 1 ) {
            continue;
        }
        char * buffer_str = ascii_escape_breaking_chars((char*)recv_buffer->buffer);
        log_msg(LOG_DEBUG, "Received '%s' (len: %d)", buffer_str, recv_buffer->size);
        free(buffer_str);
        
        if ( sim_write((Sim*)sim, sim->implementation->timeout_ms, recv_buffer->buffer, recv_buffer->size) == -1 ) {
            log_msg(LOG_ERROR, "Error while writing, exiting the loop");
            return;
        }
    }
}