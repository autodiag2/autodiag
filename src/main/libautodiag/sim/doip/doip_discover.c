#include "libautodiag/sim/doip/doip_discover.h"

void * sim_doip_discovery_loop(void *arg) {
    SimDoIp * sim = (SimDoIp*)arg;
    DoIpImplementation * implementation = ((DoIpImplementation*)sim->implementation);
    sock_t handle = implementation->disc_server_handle;
    int port = ad_object_handle_t_get_port(implementation->server_handle);
    if ( port == -1 ) {
        log_msg(LOG_ERROR, "Failed to get discovery server port");
        return null;
    }

    ad_object_DoIPMessage * vehicle_id_response_message = doip_message_new(DOIP_VEHICLE_ANNOUNCEMENT_RESPONSE);
    ad_object_DoIPMessagePayloadVehicleIdResponse * vehicle_id_response_message_payload = (ad_object_DoIPMessagePayloadVehicleIdResponse*)vehicle_id_response_message->payload;
    vehicle_id_response_message_payload->vin = ad_buffer_from_ascii("VF1BB05CF26010203");
    assert(0 < sim->ecus->size);
    vehicle_id_response_message_payload->addr = ad_buffer_from_ints(0x07, sim->ecus->list[0]->address);
    vehicle_id_response_message_payload->eid = ad_buffer_from_ascii_hex("1234567890AB");
    vehicle_id_response_message_payload->gid = ad_buffer_from_ascii_hex("1234567890AB");
    vehicle_id_response_message_payload->further_action_required = false;
    vehicle_id_response_message_payload->sync_status = false;
    Buffer * vehicle_id_response = doip_message_serialize(vehicle_id_response_message);
    ad_object_DoIPMessage_free(vehicle_id_response_message);

    long last_broadcast_ms = -1;
    final int timeout_waiting_client_ms = implementation->broadcast_time_ms/10;
    while ( sim_doip_should_continue(sim) ) {
        /*if ( last_broadcast_ms == -1 || (time_ms() - last_broadcast_ms) > implementation->broadcast_time_ms ) {
            last_broadcast_ms = time_ms();
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

            log_msg(LOG_DEBUG, "Annoucement sent");
            sendto(handle, responseBuffer->buffer, responseBuffer->size, 0,
                (struct sockaddr*)&addr, sizeof(addr));
        }*/
        int r = network_udp_wait_readable(handle, timeout_waiting_client_ms);
        if (0 < r) {
            Buffer * request = ad_buffer_new();
            ad_buffer_ensure_capacity(request, 1024);
            struct sockaddr_in from;
            socklen_t from_len = (socklen_t)sizeof(from);
            int n = (int)recvfrom(handle, request->buffer, request->size_allocated, 0, (struct sockaddr *)&from, &from_len);
            if (n <= 0) continue;
            request->size = (unsigned)n;
            ad_object_DoIPMessage * requestMessage = doip_message_parse(request);
            bool vehicle_id_received = false;
            switch(requestMessage->payload_type) {
                case DOIP_VEHICLE_IDENT_REQUEST_EID:
                case DOIP_VEHICLE_IDENT_REQUEST_VIN:
                    log_msg(LOG_DEBUG, "TODO: ignoring ecu targetting for now");
                case DOIP_VEHICLE_IDENT_REQUEST: {
                    log_msg(LOG_DEBUG, "Vehicle identification request received from %s", network_location(from));
                    sendto(handle, vehicle_id_response->buffer, vehicle_id_response->size, 0, (struct sockaddr *)&from, from_len);
                    char * addr_str = network_location(from);
                    log_msg(LOG_DEBUG, "Response sent to %s (0x%s)", addr_str, ad_buffer_to_hex_string(vehicle_id_response));
                    free(addr_str);
                } break;
                case DOIP_DIAG_POWER_MODE_REQUEST: {
                    ad_object_DoIPMessage * power_mode_response_message = doip_message_new(DOIP_DIAG_POWER_MODE_RESPONSE);
                    ad_object_DoIPMessagePayloadDiagPowerModeResponse * payload = (ad_object_DoIPMessagePayloadDiagPowerModeResponse*)power_mode_response_message->payload;
                    payload->type = DOIP_MESSAGE_DIAG_POWER_MODE_READY;
                    Buffer * power_mode_response_buffer = doip_message_serialize(power_mode_response_message);
                    ad_object_DoIPMessage_free(power_mode_response_message);
                    sendto(handle, power_mode_response_buffer->buffer, power_mode_response_buffer->size, 0, (struct sockaddr *)&from, from_len);
                    ad_buffer_free(power_mode_response_buffer);
                    log_msg(LOG_DEBUG, "Diag Power Mode response sent");
                } break;
                default: {
                    log_msg(LOG_DEBUG, "Unknown message received from %s", network_location(from));
                } break;
            }
            ad_object_DoIPMessage_free(requestMessage);
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