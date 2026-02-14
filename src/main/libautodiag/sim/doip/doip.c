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
            sock_t handle = impl->handle;
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
static uint16_t be16_u(const byte *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

static Buffer *buf_u16be(uint16_t v) {
    Buffer *b = (Buffer*)malloc(sizeof(Buffer));
    b->size = 2;
    b->buffer = (byte*)malloc(2);
    b->buffer[0] = (byte)((v >> 8) & 0xFF);
    b->buffer[1] = (byte)(v & 0xFF);
    return b;
}

static object_DoIPMessage *mk_doip_simple(DoIpPayloadType t, const Buffer *data) {
    object_DoIPMessage *m = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    if (!m) return NULL;
    memset(m, 0, sizeof(*m));
    m->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    m->inv_protocol_version = (byte)~m->protocol_version;
    m->payload_type = t;
    m->payload_length = data ? data->size : 0;
    m->payload_raw = data ? buffer_slice((Buffer*)data, 0, data->size) : NULL;
    return m;
}

static object_DoIPDiagMessage *mk_doip_diag(uint16_t src, uint16_t dst, const Buffer *data_protocol) {
    object_DoIPDiagMessage *m = object_DoIPDiagMessage_new();
    m->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    m->inv_protocol_version = (byte)~m->protocol_version;
    m->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
    m->payload.src_addr = buf_u16be(src);
    m->payload.dst_addr = buf_u16be(dst);
    m->payload.data = data_protocol ? buffer_slice((Buffer*)data_protocol, 0, data_protocol->size) : NULL;
    m->payload_length = 4 + (data_protocol ? data_protocol->size : 0);
    return m;
}
static bool doip_send_diag(SimDoIp *sim, object_DoIPDiagMessage *m) {
    Buffer *out = doip_diag_message_serialize(m);
    if (!out) return false;
    if ( sim_write((Sim*)sim, sim->implementation->timeout_ms, out->buffer, out->size) == -1 ) {
        log_msg(LOG_ERROR, "Error while sending");
        return false;
    }
    return true;
}
static bool doip_send_simple(SimDoIp *sim, object_DoIPMessage *m) {
    if (!m) return false;

    int plen = (m->payload_raw ? m->payload_raw->size : 0);
    int out_sz = 8 + plen;

    Buffer out;
    out.size = out_sz;
    out.buffer = (byte*)malloc((size_t)out_sz);
    if (!out.buffer) return false;

    out.buffer[0] = m->protocol_version ? m->protocol_version : DOIP_PROTOCOL_VERSION_CURRENT;
    out.buffer[1] = (byte)~out.buffer[0];
    out.buffer[2] = (byte)(((int)m->payload_type >> 8) & 0xFF);
    out.buffer[3] = (byte)((int)m->payload_type & 0xFF);
    out.buffer[4] = (byte)((plen >> 24) & 0xFF);
    out.buffer[5] = (byte)((plen >> 16) & 0xFF);
    out.buffer[6] = (byte)((plen >> 8) & 0xFF);
    out.buffer[7] = (byte)(plen & 0xFF);

    if (0 < plen) memcpy(out.buffer + 8, m->payload_raw->buffer, (size_t)plen);

    if ( sim_write((Sim*)sim, sim->implementation->timeout_ms, out.buffer, out.size) == -1 ) {
        free(out.buffer);
        log_msg(LOG_ERROR, "Error while sending");
        return false;
    }

    free(out.buffer);
    return true;
}

static int handle_routing_activation(SimDoIp *sim, object_DoIPMessage *req) {
    uint16_t tester = 0x0E80;
    if (req && req->payload_raw && 2 <= req->payload_raw->size) {
        tester = be16_u(req->payload_raw->buffer);
    }

    Buffer pl;
    pl.size = 13;
    pl.buffer = (byte*)malloc((size_t)pl.size);
    if (!pl.buffer) return 0;
    memset(pl.buffer, 0, (size_t)pl.size);

    pl.buffer[0] = (byte)((tester >> 8) & 0xFF);
    pl.buffer[1] = (byte)(tester & 0xFF);
    pl.buffer[2] = 0x10;
    pl.buffer[3] = 0x00;

    object_DoIPMessage *resp = mk_doip_simple(DOIP_ROUTING_ACTIVATION_RESPONSE, &pl);
    free(pl.buffer);
    if (!resp) return 0;
    return doip_send_simple(sim, resp);
}

static int handle_alive_check(SimDoIp *sim, object_DoIPMessage *req) {
    Buffer pl;
    pl.size = 2;
    pl.buffer = (byte*)malloc(2);
    if (!pl.buffer) return 0;
    pl.buffer[0] = 0x00;
    pl.buffer[1] = 0x00;
    if (req && req->payload_raw && 2 <= req->payload_raw->size) {
        pl.buffer[0] = req->payload_raw->buffer[0];
        pl.buffer[1] = req->payload_raw->buffer[1];
    }

    object_DoIPMessage *resp = mk_doip_simple(DOIP_ALIVE_CHECK_RESPONSE, &pl);
    free(pl.buffer);
    if (!resp) return 0;
    return doip_send_simple(sim, resp);
}

static int handle_entity_status(SimDoIp *sim) {
    Buffer pl;
    pl.size = 7;
    pl.buffer = (byte*)malloc((size_t)pl.size);
    if (!pl.buffer) return 0;
    memset(pl.buffer, 0, (size_t)pl.size);

    object_DoIPMessage *resp = mk_doip_simple(DOIP_ENTITY_STATUS_RESPONSE, &pl);
    free(pl.buffer);
    if (!resp) return 0;
    return doip_send_simple(sim, resp);
}

static int handle_power_mode(SimDoIp *sim) {
    Buffer pl;
    pl.size = 1;
    pl.buffer = (byte*)malloc(1);
    if (!pl.buffer) return 0;
    pl.buffer[0] = 0x00;

    object_DoIPMessage *resp = mk_doip_simple(DOIP_DIAG_POWER_MODE_RESPONSE, &pl);
    free(pl.buffer);
    if (!resp) return 0;
    return doip_send_simple(sim, resp);
}

static int handle_diag(SimDoIp *sim, object_DoIPDiagMessage *msg) {
    if (!msg->payload.src_addr || !msg->payload.dst_addr) return 1;
    if (msg->payload.src_addr->size < 2 || msg->payload.dst_addr->size < 2) return 1;

    uint16_t tester = be16_u(msg->payload.src_addr->buffer);
    assert(2 == msg->payload.dst_addr->size);

    for (int i = 0; i < sim->ecus->size; i++) {
        SimECU *ecu = sim->ecus->list[i];
        if (ecu->address != msg->payload.dst_addr->buffer[1]) continue;

        Buffer *uds_resp = sim_ecu_response(ecu, msg->payload.data);
        if (!uds_resp) return 1;

        object_DoIPDiagMessage *doip_resp = mk_doip_diag(0x0700 + ((uint16_t)ecu->address), tester, uds_resp);
        if (!doip_resp) return 0;
        if (!doip_send_diag(sim, doip_resp)) return 0;
        return 1;
    }

    return 1;
}

void sim_doip_loop(SimDoIp * sim) {
    #ifdef OS_WINDOWS
        #ifdef OS_POSIX
            sim->implementation->handle = -1;
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
        if (!sim->implementation->loop_ready) sim->implementation->loop_ready = true;

        struct sockaddr_in addr;

        if ( ! sim_doip_network_is_connected(sim->implementation) ) {
            #if defined OS_POSIX
                socklen_t addr_len = sizeof(addr);
                #ifdef OS_WINDOWS
                    sim->implementation->handle = accept(sim->implementation->server_fd, (struct sockaddr*)&addr, &addr_len);
                    if (sim->implementation->handle == -1) {
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

        if (recv_buffer->size < 8) {
            log_msg(LOG_WARNING, "Received message appears truncated (len: %d)", recv_buffer->size);
            continue;
        }

        char * buffer_str = buffer_to_ascii_espace_breaking_chars(recv_buffer);
        log_msg(LOG_DEBUG, "Received '%s' (len: %d)", buffer_str, recv_buffer->size);
        free(buffer_str);

        object_DoIPMessage *hdr = doip_message_parse(recv_buffer);
        if (!hdr) continue;

        if (hdr->payload_type == DOIP_DIAGNOSTIC_MESSAGE ||
            hdr->payload_type == DOIP_DIAGNOSTIC_MESSAGE_ACK ||
            hdr->payload_type == DOIP_DIAGNOSTIC_MESSAGE_NACK) {

            object_DoIPDiagMessage *dmsg = doip_diag_message_parse(recv_buffer);
            if (!dmsg) continue;
            if (dmsg->payload_type == DOIP_DIAGNOSTIC_MESSAGE) {
                if (!handle_diag(sim, dmsg)) return;
            }
            continue;
        }

        if (hdr->payload_type == DOIP_ROUTING_ACTIVATION_REQUEST) {
            if (!handle_routing_activation(sim, hdr)) return;
        } else if (hdr->payload_type == DOIP_ALIVE_CHECK_REQUEST) {
            if (!handle_alive_check(sim, hdr)) return;
        } else if (hdr->payload_type == DOIP_ENTITY_STATUS_REQUEST) {
            if (!handle_entity_status(sim)) return;
        } else if (hdr->payload_type == DOIP_DIAG_POWER_MODE_REQUEST) {
            if (!handle_power_mode(sim)) return;
        } else {
            log_msg(LOG_ERROR, "Payload type 0x%04X not implemented", (int)hdr->payload_type);
        }
    }
}