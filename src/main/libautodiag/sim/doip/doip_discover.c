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
    while ( sim_doip_should_continue(sim) ) {
        /*if ( last_broadcast_ms == -1 || (time_ms() - last_broadcast_ms) > implementation->broadcast_time_ms ) {
            Buffer * annoucement = buffer_from_ascii_hex("000102030405060708090A0B0C0D0E0F");   
            last_broadcast_ms = time_ms();
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

            log_msg(LOG_DEBUG, "Annoucement sent");
            sendto(handle, annoucement->buffer, annoucement->size, 0,
                (struct sockaddr*)&addr, sizeof(addr));
        }*/
        int r = network_udp_wait_readable(handle, timeout_waiting_client_ms);
        if (0 < r) {
            Buffer * request = buffer_new();
            buffer_ensure_capacity(request, 1024);
            struct sockaddr_in from;
            socklen_t from_len = (socklen_t)sizeof(from);
            int n = (int)recvfrom(handle, request->buffer, request->size_allocated, 0, (struct sockaddr *)&from, &from_len);
            if (n <= 0) continue;
            request->size = (unsigned)n;
            object_DoIPMessage * requestMessage = doip_message_parse(request);
            object_DoIPMessage * responseMessage = null;
            switch(requestMessage->payload_type) {
                case DOIP_VEHICLE_IDENT_REQUEST_EID:
                case DOIP_VEHICLE_IDENT_REQUEST_VIN:
                    log_msg(LOG_DEBUG, "TODO: ignoring ecu targetting for now");
                case DOIP_VEHICLE_IDENT_REQUEST: {
                    log_msg(LOG_DEBUG, "Vehicle identification request received from %s", network_location(from));
                    responseMessage = doip_message_new(DOIP_VEHICLE_ANNOUNCEMENT_RESPONSE);
                    object_DoIPMessagePayloadVehicleIdResponse * payload = (object_DoIPMessagePayloadVehicleIdResponse*)responseMessage->payload;
                    payload->vin = buffer_from_ascii("VF1BB05CF26010203");
                    assert(0 < sim->ecus->size);
                    payload->addr = buffer_from_ints(0x07, sim->ecus->list[0]->address);
                    payload->eid = buffer_from_ascii_hex("1234567890AB");
                    payload->gid = buffer_from_ascii_hex("1234567890AB");
                    payload->further_action_required = false;
                    payload->sync_status = false;
                } break;

                default: {
                    log_msg(LOG_DEBUG, "Unknown message received from %s", network_location(from));
                } break;
            }
            object_DoIPMessage_free(requestMessage);
            if (responseMessage) {
                Buffer * responseBuffer = doip_message_serialize(responseMessage);
                if (responseBuffer) {
                    sendto(handle, responseBuffer->buffer, responseBuffer->size, 0, (struct sockaddr *)&from, from_len);
                    char * addr_str = network_location(from);
                    log_msg(LOG_DEBUG, "Response sent to %s (0x%s)", addr_str, buffer_to_hex_string(responseBuffer));
                    free(addr_str);
                    buffer_free(responseBuffer);
                }
                object_DoIPMessage_free(responseMessage);
            }
        }
    }
    return null;
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