#include "libautodiag/sim/doip/doip.h"
#include "libautodiag/sim/doip/doip_discover.h"

SimDoIp * sim_doip_new() {
    SimDoIp * sim = (SimDoIp*)malloc(sizeof(SimDoIp));
    sim_init_with_defaults((Sim*)sim);
    sim->type = strdup("doip");
    DoIpImplementation * impl = (DoIpImplementation*)malloc(sizeof(DoIpImplementation));
    impl->loop_thread = null;
    impl->loop_ready = false;
    impl->timeout_ms = SIM_DOIP_TIMEOUT_MS_RW;
    impl->broadcast_time_ms = SIM_DOIP_TIMEOUT_MS_BROADCAST;
    sim->implementation = (SimImplementation*)impl;
    return sim;
}

bool sim_doip_network_is_connected(void * implPtr) {
    assert(implPtr != null);
    DoIpImplementation * impl = (DoIpImplementation*)implPtr;
    sock_t handle = impl->handle;
    if ( handle == SOCK_T_INVALID ) {
        return false;
    }
    char buf;
    ssize_t ret = recv(handle, &buf, 1, MSG_PEEK);
    if (ret == 0) return false; // connection closed by peer
    #if defined OS_POSIX
    #   include <fcntl.h>
    #   include <errno.h>
        if (ret == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true; // still connected, no data
            return false; // error, consider disconnected
        }
    #elif defined OS_WINDOWS
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return true;
            return false;
        }
    #else
    #   warning Unsupported OS
    #endif
    return true; // data available, connection alive
}
void sim_doip_destroy(SimDoIp *sim) {
    THREAD_CANCEL(((DoIpImplementation*)sim->implementation)->loop_thread);
    free(((DoIpImplementation*)sim->implementation)->loop_thread);
    ((DoIpImplementation*)sim->implementation)->loop_thread = null;
    ((DoIpImplementation*)sim->implementation)->loop_ready = false;
}
void sim_doip_loop_as_daemon(SimDoIp * sim) {
    THREAD_CANCEL(((DoIpImplementation*)sim->implementation)->loop_thread);
    ((DoIpImplementation*)sim->implementation)->loop_thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( pthread_create(((DoIpImplementation*)sim->implementation)->loop_thread, NULL,
                          (void *(*) (void *)) sim_doip_loop, (void *)sim) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        free(((DoIpImplementation*)sim->implementation)->loop_thread);
        ((DoIpImplementation*)sim->implementation)->loop_thread = null;
        exit(EXIT_FAILURE);
    }
}
bool sim_doip_loop_daemon_wait_ready(SimDoIp *sim) {
    return sim_loop_daemon_wait_ready(&((DoIpImplementation*)sim->implementation)->loop_ready);
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
    assert(data);
    object_DoIPMessage *m = (object_DoIPMessage*)malloc(sizeof(object_DoIPMessage));
    if (!m) return NULL;
    memset(m, 0, sizeof(*m));
    m->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    m->inv_protocol_version = (byte)~m->protocol_version;
    m->payload_type = t;
    m->payload_raw = data ? buffer_slice((Buffer*)data, 0, data->size) : NULL;
    return m;
}

static object_DoIPMessage *mk_doip_diag(uint16_t src, uint16_t dst, const Buffer *data_protocol) {
    return doip_diag_message(buf_u16be(dst),buf_u16be(src),data_protocol ? buffer_slice((Buffer*)data_protocol, 0, data_protocol->size) : NULL);
}
static bool doip_send_msg(SimDoIp *sim, object_DoIPMessage *m) {
    Buffer *out = doip_message_serialize(m);
    if (!out) return false;
    if ( sim_write((Sim*)sim, ((DoIpImplementation*)sim->implementation)->timeout_ms, out->buffer, out->size) == -1 ) {
        log_msg(LOG_ERROR, "Error while sending");
        return false;
    }
    return true;
}

static int handle_alive_check(SimDoIp *sim, object_DoIPMessage *req) {
    Buffer * response = buffer_new();
    buffer_ensure_capacity(response, 2);
    buffer_fill(response, 0x00);
    if (req && req->payload_raw && 2 <= req->payload_raw->size) {
        response->buffer[0] = req->payload_raw->buffer[0];
        response->buffer[1] = req->payload_raw->buffer[1];
    }

    object_DoIPMessage *resp = mk_doip_simple(DOIP_ALIVE_CHECK_RESPONSE, response);
    buffer_free(response);
    if (!resp) return 0;
    return doip_send_msg(sim, resp);
}

static int handle_entity_status(SimDoIp *sim) {
    Buffer * response = buffer_new();
    buffer_ensure_capacity(response, 7);
    buffer_fill(response, 0x00);

    object_DoIPMessage *resp = mk_doip_simple(DOIP_ENTITY_STATUS_RESPONSE, response);
    buffer_free(response);
    if (!resp) return 0;
    return doip_send_msg(sim, resp);
}

static int handle_power_mode(SimDoIp *sim) {
    Buffer * response = buffer_new();
    buffer_ensure_capacity(response, 1);
    buffer_fill(response, 0x00);

    object_DoIPMessage *resp = mk_doip_simple(DOIP_DIAG_POWER_MODE_RESPONSE, response);
    buffer_free(response);
    if (!resp) return 0;
    return doip_send_msg(sim, resp);
}

static int handle_diag(SimDoIp *sim, object_DoIPMessage *msg) {
    assert(msg->payload_type == DOIP_DIAGNOSTIC_MESSAGE);
    object_DoIPMessagePayloadDiag * diag = (object_DoIPMessagePayloadDiag *)msg->payload;
    if (!diag->src_addr || !diag->dst_addr) {
        log_msg(LOG_ERROR, "issue with addressing");
        return 1;
    }
    if (diag->src_addr->size < 2 || diag->dst_addr->size < 2) {
        log_msg(LOG_ERROR, "issue with addressing 2");
        return 1;
    }

    uint16_t tester = be16_u(diag->src_addr->buffer);
    assert(2 == diag->dst_addr->size);

    for (int i = 0; i < sim->ecus->size; i++) {
        SimECU *ecu = sim->ecus->list[i];
        if (ecu->address != diag->dst_addr->buffer[1]) {
            log_msg(LOG_DEBUG, "Not addressed to this ecu, continuing");
            continue;
        }

        log_msg(LOG_DEBUG, "Found target ecu %02hhX", ecu->address);
        Buffer *data_protocol = sim_ecu_response(ecu, diag->data);
        if (!data_protocol) return 1;

        log_msg(LOG_DEBUG, "Sending back %s", buffer_to_hex_string(data_protocol));
        object_DoIPMessage *doip_resp = mk_doip_diag(0x0700 + ((uint16_t)ecu->address), tester, data_protocol);
        if (!doip_resp) return 0;
        if (!doip_send_msg(sim, doip_resp)) return 0;
        return 1;
    }

    return 1;
}

void sim_doip_loop(SimDoIp * sim) {

    sim_doip_discover_start(sim);

    sim->implementation->handle = SOCK_T_INVALID;
    sim->implementation->server_fd = SOCK_T_INVALID;

    int main_loop_port = -1;
    sock_t serverFD = network_tcp_start(&main_loop_port, DOIP_NETWORK_PORT);
    if ( serverFD == SOCK_T_INVALID ) {
        log_msg(LOG_ERROR, "Failed to start server");
        perror("doip network_tcp_start");
        return;
    }
    log_msg(LOG_DEBUG, "Listening on TCP");
    assert(main_loop_port != -1);
    sim->implementation->server_fd = serverFD;
    asprintf(&sim->device_location, "0.0.0.0:%d", main_loop_port);

    log_msg(LOG_INFO, "sim running on %s", sim->device_location);
    final Buffer * recv_buffer = buffer_new();
    buffer_ensure_capacity(recv_buffer, 100);

    while(((DoIpImplementation*)sim->implementation)->loop_thread != null) {
        buffer_recycle(recv_buffer);
        if (!((DoIpImplementation*)sim->implementation)->loop_ready) ((DoIpImplementation*)sim->implementation)->loop_ready = true;

        struct sockaddr_in addr;

        if ( ! sim_doip_network_is_connected(((DoIpImplementation*)sim->implementation)) ) {
            log_msg(LOG_DEBUG, "Waiting for a client to connect");
            socklen_t addr_len = sizeof(addr);
            sim->implementation->handle = accept(sim->implementation->server_fd, (struct sockaddr*)&addr, &addr_len);
            if (sim->implementation->handle == SOCK_T_INVALID) {
                perror("accept");
                return;
            }
            char * location = network_location(addr);
            log_msg(LOG_INFO, "Client %s connected", location);
            free(location);
        }

        if ( sim_read((Sim*)sim, ((DoIpImplementation*)sim->implementation)->timeout_ms, recv_buffer) == -1 ) {
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

        object_DoIPMessage *msg = doip_message_parse(recv_buffer);
        if (!msg) {
            log_msg(LOG_DEBUG, "Unabled to parse incoming data : '%s'", buffer_to_ascii_espace_breaking_chars(recv_buffer));
            continue;
        }

        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                log_msg(LOG_DEBUG, "Sim received '%s'", buffer_to_hex_string(recv_buffer));
                if (!handle_diag(sim, msg)) {
                    log_msg(LOG_DEBUG, "diag message has not responded");
                    continue;
                }
            } break;
            case DOIP_ROUTING_ACTIVATION_REQUEST: {
                object_DoIPMessagePayloadRoutineActivationRequest * reqPayload = (object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload;

                object_DoIPMessage * reply = doip_message_new(DOIP_ROUTING_ACTIVATION_RESPONSE);
                object_DoIPMessagePayloadRoutineActivationResponse * payload = (object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload;
                buffer_memcpy(payload->tester, reqPayload->src_addr, sizeof(reqPayload->src_addr));
                buffer_assign(payload->ecu, buffer_from_ascii_hex("0000"));
                payload->code = DOIP_MESSAGE_RARES_CODE_SUCCESS;
                payload->iso_reserved = buffer_new_random(4);
                payload->oem_reserved = buffer_new_random(4);

                final int res = doip_send_msg(sim, reply);
                object_DoIPMessage_free(reply);
            } break;
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                log_msg(LOG_DEBUG, "This message is not supposed to be sent by the tester, ignoring...");
            } break;
            case DOIP_ALIVE_CHECK_REQUEST: {
                if (!handle_alive_check(sim, msg)) return;
            } break;
            case DOIP_ENTITY_STATUS_REQUEST: {
                if (!handle_entity_status(sim)) return;
            } break;
            case DOIP_DIAG_POWER_MODE_REQUEST: {
                if (!handle_power_mode(sim)) return;
            } break;
            default: {
                log_msg(LOG_ERROR, "Payload type 0x%04X not implemented", (int)msg->payload_type);
            } break;
        }
    }
}