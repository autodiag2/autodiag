#include "libautodiag/sim/doip/doip.h"
#include "libautodiag/sim/doip/doip_discover.h"

SimDoIp * sim_doip_new() {
    SimDoIp * sim = (SimDoIp*)malloc(sizeof(SimDoIp));
    sim_init_with_defaults((Sim*)sim);
    sim->type = strdup("doip");
    sim->max_concurrent_connections = 1;
    sim->openned_connections = 0;
    sim->max_data_size = DOIP_MESSAGE_ENTITY_STATUS_DEFAULT_MAX_DATA_SIZE;
    DoIpImplementation * impl = (DoIpImplementation*)malloc(sizeof(DoIpImplementation));
    impl->loop_thread = null;
    impl->loop_ready = false;
    impl->timeout_ms = SIM_DOIP_TIMEOUT_MS_RW;
    impl->broadcast_time_ms = SIM_DOIP_TIMEOUT_MS_BROADCAST;
    impl->handle = ad_object_handle_t_new();
    impl->server_handle = ad_object_handle_t_new();
    impl->disc_server_handle = SOCK_T_INVALID;
    sim->implementation = (SimImplementation*)impl;
    return sim;
}

void sim_doip_destroy(SimDoIp *sim) {
    DoIpImplementation * impl = (DoIpImplementation*)sim->implementation;
    THREAD_CANCEL(impl->loop_thread);
    free(impl->loop_thread);
    impl->loop_thread = null;
    impl->loop_ready = false;
}
void sim_doip_loop_as_daemon(SimDoIp * sim) {
    DoIpImplementation * impl = (DoIpImplementation*)sim->implementation;
    THREAD_CANCEL(impl->loop_thread);
    impl->loop_thread = (pthread_t*)malloc(sizeof(pthread_t));
    if ( pthread_create(impl->loop_thread, NULL,
                          (void *(*) (void *)) sim_doip_loop, (void *)sim) != 0 ) {
        log_msg(LOG_ERROR, "thread creation error");
        free(impl->loop_thread);
        impl->loop_thread = null;
        exit(EXIT_FAILURE);
    }
}
bool sim_doip_loop_daemon_wait_ready(SimDoIp *sim) {
    return sim_loop_daemon_wait_ready(&((DoIpImplementation*)sim->implementation)->loop_ready);
}
static ad_object_DoIPMessage *mk_doip_simple(DoIpPayloadType t, const Buffer *data) {
    assert(data);
    ad_object_DoIPMessage *m = (ad_object_DoIPMessage*)malloc(sizeof(ad_object_DoIPMessage));
    if (!m) return NULL;
    memset(m, 0, sizeof(*m));
    m->protocol_version = DOIP_PROTOCOL_VERSION_CURRENT;
    m->inv_protocol_version = (byte)~m->protocol_version;
    m->payload_type = t;
    m->payload_raw = data ? ad_buffer_slice((Buffer*)data, 0, data->size) : NULL;
    return m;
}

static ad_object_DoIPMessage *mk_doip_diag(uint16_t src, uint16_t dst, const Buffer *data_protocol) {
    return doip_message_diag(ad_buffer_be_from_uint16(dst),ad_buffer_be_from_uint16(src),data_protocol ? ad_buffer_slice((Buffer*)data_protocol, 0, data_protocol->size) : NULL);
}
static bool doip_send_msg(SimDoIp *sim, ad_object_DoIPMessage *m) {
    Buffer *out = doip_message_serialize(m);
    if (!out) return false;
    if ( sim_write((Sim*)sim, ((DoIpImplementation*)sim->implementation)->timeout_ms, out->buffer, out->size) == -1 ) {
        log_msg(LOG_ERROR, "Error while sending");
        return false;
    }
    return true;
}

static int handle_power_mode(SimDoIp *sim) {
    Buffer * response = ad_buffer_new();
    ad_buffer_ensure_capacity(response, 1);
    ad_buffer_fill(response, 0x00);

    ad_object_DoIPMessage *resp = mk_doip_simple(DOIP_DIAG_POWER_MODE_RESPONSE, response);
    ad_buffer_free(response);
    if (!resp) return 0;
    return doip_send_msg(sim, resp);
}

static int handle_diag(SimDoIp *sim, ad_object_DoIPMessage *msg) {
    assert(msg->payload_type == DOIP_DIAGNOSTIC_MESSAGE);
    log_msg(LOG_DEBUG, "Diag Message received");
    ad_object_DoIPMessagePayloadDiag * diag = (ad_object_DoIPMessagePayloadDiag *)msg->payload;
    if (!diag->src_addr || !diag->dst_addr) {
        log_msg(LOG_ERROR, "issue with addressing");
        return 1;
    }
    if (diag->src_addr->size < 2 || diag->dst_addr->size < 2) {
        log_msg(LOG_ERROR, "issue with addressing 2");
        return 1;
    }

    uint16_t tester = ad_buffer_to_be16(diag->src_addr->buffer);
    assert(2 == diag->dst_addr->size);

    SimECU *ecu = sim_search_ecu_by_address((Sim*)sim, diag->dst_addr->buffer[diag->dst_addr->size-1]);
    if ( ecu == null ) {
        if ( sim->ecus->size == 1 ) {
            log_msg(LOG_DEBUG, "Diag message was addressed to a different ECU, never mind continuing ...");
            ecu = sim->ecus->list[0];
        }
    }
    
    if ( ecu == null ) {
        log_msg(LOG_WARNING, "No ECU found for address %02hhX, ignoring diag message (should send NACK)", diag->dst_addr->buffer[1]);
        ad_object_DoIPMessage *nack = doip_message_diag_feedback_nack(diag->src_addr, diag->dst_addr, msg->payload_raw, DOIP_DIAGNOSTIC_MESSAGE_NACK_CODE_UNKNOWN_TARGET_ADDR);
        doip_send_msg(sim, nack);
        ad_object_DoIPMessage_free(nack);
        return 1;
    }

    ad_object_DoIPMessage * ack = doip_message_diag_feedback_ack(diag->src_addr, diag->dst_addr, msg->payload_raw, DOIP_DIAGNOSTIC_MESSAGE_ACK_CODE_POSITIVE);
    doip_send_msg(sim, ack);
    ad_object_DoIPMessage_free(ack);

    log_msg(LOG_DEBUG, "Found target ecu %02hhX", ecu->address);
    Buffer *data_protocol = sim_ecu_response(ecu, diag->data);
    if (!data_protocol) return 1;

    log_msg(LOG_DEBUG, "Sending back %s", ad_buffer_to_hex_string(data_protocol));
    ad_object_DoIPMessage *doip_resp = mk_doip_diag(0x0700 + ((uint16_t)ecu->address), tester, data_protocol);
    if (!doip_resp) return 0;
    if (!doip_send_msg(sim, doip_resp)) return 0;

    return 1;
}
bool sim_doip_should_continue(SimDoIp * sim) {
    DoIpImplementation * impl = (DoIpImplementation*)sim->implementation;
    return impl->loop_thread != null;
}
void sim_doip_loop(SimDoIp * sim) {

    DoIpImplementation * impl = (DoIpImplementation*)sim->implementation;
    ad_object_handle_t_init(impl->server_handle);
    ad_object_handle_t_init(impl->handle);

    int main_loop_port = -1;
    sock_t serverFD = network_tcp_start(&main_loop_port, DOIP_NETWORK_PORT, sim->max_concurrent_connections);
    if ( serverFD == SOCK_T_INVALID ) {
        log_msg(LOG_ERROR, "Failed to start server");
        perror("doip network_tcp_start");
        return;
    }
    log_msg(LOG_DEBUG, "Listening on TCP");
    assert(main_loop_port != -1);
    #ifdef OS_POSIX
        impl->server_handle->posix_handle = serverFD;
    #elif defined OS_WINDOWS
        impl->server_handle->win_socket = serverFD;
    #else
    #   warning unsupported OS
    #endif
    asprintf(&sim->device_location, "0.0.0.0:%d", main_loop_port);

    log_msg(LOG_INFO, "running on %s", sim->device_location);
    sim_doip_discover_start(sim);

    final Buffer * recv_buffer = ad_buffer_new();
    ad_buffer_ensure_capacity(recv_buffer, 100);

    // Need to be turned on for diag messages to be sent
    bool routing_activated = false;

    while(sim_doip_should_continue(sim)) {
        ad_buffer_recycle(recv_buffer);
        if (!impl->loop_ready) impl->loop_ready = true;

        struct sockaddr_in addr;

        if ( ! sim_network_is_connected(impl->handle) ) {
            routing_activated = false;
            sim->openned_connections = 0;
            log_msg(LOG_DEBUG, "Waiting for a client to connect");
            #ifdef OS_POSIX
                sock_t server_handle = impl->server_handle->posix_handle;
            #elif defined OS_WINDOWS
                sock_t server_handle = impl->server_handle->win_socket;
            #else
            #   warning unsupported OS
            #endif
            socklen_t addr_len = sizeof(addr);
            sock_t client_socket = accept(server_handle, (struct sockaddr*)&addr, &addr_len);
            #ifdef OS_POSIX
                impl->handle->posix_handle = client_socket;
            #elif defined OS_WINDOWS
                impl->handle->win_socket = client_socket;
            #else
            #   warning unsupported OS
            #endif
            if (client_socket == SOCK_T_INVALID) {
                perror("accept");
                return;
            }
            sim->openned_connections = 1;
            char * location = network_location(addr);
            log_msg(LOG_INFO, "Client %s connected", location);
            free(location);
        }

        if ( sim_read((Sim*)sim, impl->timeout_ms, recv_buffer) == -1 ) {
            log_msg(LOG_ERROR, "Error during reception, exiting the loop");
            return;
        }

        if (recv_buffer->size < 8) {
            log_msg(LOG_WARNING, "Received message appears truncated (len: %d)", recv_buffer->size);
            continue;
        }

        char * buffer_str = ad_buffer_to_hex_string(recv_buffer);
        log_msg(LOG_DEBUG, "Received '%s' (len: %d)", buffer_str, recv_buffer->size);
        free(buffer_str);

        ad_object_DoIPMessage *msg = doip_message_parse(recv_buffer);
        if (!msg) {
            log_msg(LOG_DEBUG, "Unabled to parse incoming data : '%s'", ad_buffer_to_ascii_espace_breaking_chars(recv_buffer));
            continue;
        }

        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                if (!handle_diag(sim, msg)) {
                    log_msg(LOG_DEBUG, "diag message has not responded");
                    continue;
                }
            } break;
            case DOIP_ALIVE_CHECK_RESPONSE: {
                ad_object_DoIPMessagePayloadAliveCheck * payload = (ad_object_DoIPMessagePayloadAliveCheck*)msg->payload;
                log_msg(LOG_DEBUG, "Alive Check Response received from tester 0x%04X", ad_buffer_to_hex_string(payload->src_addr));
            } break;
            case DOIP_ROUTING_ACTIVATION_REQUEST: {
                log_msg(LOG_DEBUG, "Routing Activation Request received");
                ad_object_DoIPMessagePayloadRoutineActivationRequest * reqPayload = (ad_object_DoIPMessagePayloadRoutineActivationRequest*)msg->payload;

                ad_object_DoIPMessage * reply = doip_message_new(DOIP_ROUTING_ACTIVATION_RESPONSE);
                ad_object_DoIPMessagePayloadRoutineActivationResponse * payload = (ad_object_DoIPMessagePayloadRoutineActivationResponse*)reply->payload;
                ad_buffer_memcpy(payload->tester, reqPayload->src_addr, sizeof(reqPayload->src_addr));
                ad_buffer_assign(payload->ecu, ad_buffer_from_ascii_hex("0000"));
                payload->code = DOIP_MESSAGE_RARES_CODE_SUCCESS;
                payload->iso_reserved = ad_buffer_new_random(4);
                payload->oem_reserved = ad_buffer_new_random(4);
                log_msg(LOG_DEBUG, "No OEM specific handling for routing activation or tester address checking, sending success response");

                final bool res = doip_send_msg(sim, reply);
                ad_object_DoIPMessage_free(reply);
                routing_activated = res;
                log_msg(LOG_DEBUG, "Routing activated: %s", res ? "success" : "failure");
            } break;
            case DOIP_ALIVE_CHECK_REQUEST:
            case DOIP_DIAGNOSTIC_MESSAGE_ACK:
            case DOIP_DIAGNOSTIC_MESSAGE_NACK:
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                log_msg(LOG_WARNING, "This message is not supposed to be sent by the tester, ignoring...");
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