#include "libautodiag/com/doip/device.h"
#include "libautodiag/com/doip/doip.h"

AD_OBJECT_SRC(DoIPDevice)

int doip_disc_send(object_DoIPDevice *device, object_DoIPMessage *msg) {
    assert(device != null && msg != null);

    switch(msg->payload_type) {
        case DOIP_VEHICLE_IDENT_REQUEST:
        case DOIP_VEHICLE_IDENT_REQUEST_EID:
        case DOIP_VEHICLE_IDENT_REQUEST_VIN:
            break;
        default: {
            log_msg(LOG_WARNING, "Not supposed to send 0x%X payload with udp", msg->payload_type);
        } break;
    }

    Buffer *serialized = doip_message_serialize(msg);
    if (!serialized) return -1;

    struct sockaddr_in addr = network_location_to_object(device->location);
    #ifdef OS_POSIX
        ssize_t sent = sendto(device->implementation->disc_handle,
                            serialized->buffer,
                            serialized->size,
                            0,
                            (struct sockaddr *)&addr,
                            sizeof(addr));
        return (sent == serialized->size) ? 0 : -1;

    #elif defined(OS_WINDOWS)
        int sent = sendto(device->implementation->disc_handle,
                        (const char *)serialized->buffer,
                        (int)serialized->size,
                        0,
                        (struct sockaddr *)&addr,
                        sizeof(addr));
        return (sent == (int)serialized->size) ? 0 : -1;

    #endif
}

object_DoIPMessage *doip_disc_recv(object_DoIPDevice *device) {
    assert(device != null);

    unsigned char buf[4096];
    #ifdef OS_POSIX
        struct timeval tv = { device->timeout_ms / 1000, (device->timeout_ms % 1000) * 1000 };
        setsockopt(device->implementation->disc_handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        struct sockaddr_in src;
        socklen_t slen = sizeof(src);
        ssize_t recv_len = recvfrom(device->implementation->disc_handle, buf, sizeof(buf), 0,
                                    (struct sockaddr *)&src, &slen);
        if (recv_len <= 0) return null;

    #elif defined(OS_WINDOWS)
        DWORD tv = device->timeout_ms;
        setsockopt(device->implementation->disc_handle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        struct sockaddr_in src;
        int slen = sizeof(src);
        int recv_len = recvfrom(device->implementation->disc_handle, (char*)buf, sizeof(buf), 0,
                                (struct sockaddr *)&src, &slen);
        if (recv_len <= 0) return NULL;
    #endif

    Buffer *recv_buf = buffer_from_bytes(buf, (size_t)recv_len);
    if (!recv_buf) return null;

    object_DoIPMessage *msg = doip_message_parse(recv_buf);
    buffer_free(recv_buf);
    return msg;
}

bool doip_configure(final object_DoIPDevice * device) {
    assert(device != null);
    log_msg(LOG_DEBUG, "Configuring doip device");
    {
        log_msg(LOG_DEBUG, "Discovering the vehicle");
        object_DoIPMessage * request = doip_message_new(DOIP_VEHICLE_IDENT_REQUEST);
        if ( doip_disc_send(device, request) < 0 ) {
            log_msg(LOG_ERROR, "Error while retrieving ECUs");
            return false;
        }
        object_DoIPMessage_free(request);
        object_DoIPMessage * response = doip_disc_recv(device);
        if ( response == null ) {
            log_msg(LOG_ERROR, "Error while retrieving ECUs");
            return false;
        }
        if ( response->payload_type != DOIP_VEHICLE_ANNOUNCEMENT_RESPONSE ) {
            log_msg(LOG_ERROR, "Payload 0x%X received instead of 0x%X", response->payload_type, DOIP_VEHICLE_ANNOUNCEMENT_RESPONSE);
            return false;
        }
        object_DoIPMessagePayloadVehicleIdResponse * responsePayload = (object_DoIPMessagePayloadVehicleIdResponse*)response->payload;
        device->node.address = buffer_copy(responsePayload->addr);
        log_msg(LOG_DEBUG, "Found node 0x%s VIN=%s", buffer_to_hex_string(responsePayload->addr), buffer_to_ascii_espace_breaking_chars(responsePayload->vin));
        object_DoIPMessage_free(response);
    }
    {
        log_msg(LOG_DEBUG, "Routine activation");
        object_DoIPMessage * request = doip_message_new(DOIP_ROUTING_ACTIVATION_REQUEST);
        object_DoIPMessagePayloadRoutineActivationRequest * requestPayload = object_DoIPMessagePayloadRoutineActivationRequest_new();
        requestPayload->src_addr[0] = (device->address >> 8) & 0xFF;
        requestPayload->src_addr[1] = device->address & 0xFF;
        request->payload = (DoIPMessageDef*)requestPayload;
        doip_send_internal(device, buffer_to_hex_string(doip_message_serialize(request)));
        object_DoIPMessage_free(request);
        buffer_recycle(device->recv_buffer);
        doip_recv_internal(device);
        object_DoIPMessage * response = doip_message_parse(device->recv_buffer);
        if ( response == null ) {
            return false;
        }
        switch(response->payload_type) {
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                object_DoIPMessagePayloadRoutineActivationResponse * responsePayload = (object_DoIPMessagePayloadRoutineActivationResponse*)response->payload;
                switch(responsePayload->code) {
                    case DOIP_MESSAGE_RARES_CODE_SUCCESS: {
                        object_DoIPMessage_free(response);
                    } return true;
                    default: {
                        log_msg(LOG_ERROR, "Doip node refusing to start diagnostic maybe implement manufacturer specific process");
                        object_DoIPMessage_free(response);
                    } return false;
                }
            } break;
            default: {
                log_msg(LOG_ERROR, "Received 0x%04X instead of 0x%04X aborting the configure", response->payload_type, DOIP_ROUTING_ACTIVATION_RESPONSE);
                object_DoIPMessage_free(response);
            } return false;
        }
    }
    return false;
}

bool doip_node_queue_is_full(final object_DoIPDevice * device) {
    object_DoIPMessage * request = doip_message_new(DOIP_ENTITY_STATUS_REQUEST);
    doip_send_internal(device, buffer_to_hex_string(doip_message_serialize(request)));
    object_DoIPMessage_free(request);
    buffer_recycle(device->recv_buffer);
    doip_recv_internal(device);
    object_DoIPMessage * response = doip_message_parse(device->recv_buffer);
    switch(response->payload_type) {
        case DOIP_ENTITY_STATUS_RESPONSE: {
            object_DoIPMessagePayloadEntityStatusResponse * payload = (object_DoIPMessagePayloadEntityStatusResponse*)response->payload;
            bool result = payload->openned_connections >= payload->max_concurrent_connections;
            log_msg(LOG_DEBUG, "Node queue is %s (openned connections: %d, max concurrent connections: %d)", result ? "full" : "not full", payload->openned_connections, payload->max_concurrent_connections);
            object_DoIPMessage_free(response);
            return result;
        } break;
        default: {
            log_msg(LOG_ERROR, "Received 0x%04X instead of 0x%04X aborting the configure", response->payload_type, DOIP_ENTITY_STATUS_RESPONSE);
            object_DoIPMessage_free(response);
        } return bool_unset;
    }
}
void doip_close(final object_DoIPDevice * device) {
    if ( device == null || device->state != AD_DEVICE_STATE_READY ) {
        log_msg(LOG_INFO, "Close: device not open");
        return;
    } 
    assert(device_location_is_network((Device*)device));
    
    network_stop(device->implementation->handle);
    network_stop(device->implementation->disc_handle);
    device->state = AD_DEVICE_STATE_NOT_READY;
}
int doip_open(final object_DoIPDevice * device) {
    if ( device == null ) {
        log_msg(LOG_INFO, "Open: Cannot open since no device info given");
        return GENERIC_FUNCTION_ERROR;
    } 

    if (device->location == null) {
        log_msg(LOG_DEBUG, "Open: Cannot open since no location on the device");
        return GENERIC_FUNCTION_ERROR;
    }

    doip_close(device);
    assert(device_location_is_network((Device*)device));

    const char *addr = device->location;
    char host[500];
    char port_str[8] = "35000";
    const char *colon = strchr(addr, ':');
    if (colon) {
        size_t len = colon - addr;
        if (len >= sizeof(host)) return GENERIC_FUNCTION_ERROR;
        memcpy(host, addr, len);
        host[len] = 0;
        strncpy(port_str, colon + 1, sizeof(port_str) - 1);
    } else {
        strncpy(host, addr, sizeof(host) - 1);
    }

    device->implementation->disc_handle = SOCK_T_INVALID;
    log_msg(LOG_DEBUG, "Opening UDP discovery socket");

    #ifdef OS_POSIX
        int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_fd < 0) {
            perror("UDP socket");
            return GENERIC_FUNCTION_ERROR;
        }

        int opt = 1;
        setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
        setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in udp_sa;
        bzero(&udp_sa, sizeof(udp_sa));
        udp_sa.sin_family = AF_INET;
        udp_sa.sin_port = htons(0);
        udp_sa.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(udp_fd, (struct sockaddr *)&udp_sa, sizeof(udp_sa)) < 0) {
            perror("UDP bind");
            close(udp_fd);
            return GENERIC_FUNCTION_ERROR;
        }

        device->implementation->disc_handle = udp_fd;

    #elif defined(OS_WINDOWS)
        SOCKET udp_s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udp_s == INVALID_SOCKET) {
            log_msg(LOG_ERROR, "UDP socket failed: %d", WSAGetLastError());
            return GENERIC_FUNCTION_ERROR;
        }

        BOOL opt = TRUE;
        setsockopt(udp_s, SOL_SOCKET, SO_BROADCAST, (const char*)&opt, sizeof(opt));
        setsockopt(udp_s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        struct sockaddr_in udp_sa;
        ZeroMemory(&udp_sa, sizeof(udp_sa));
        udp_sa.sin_family = AF_INET;
        udp_sa.sin_port = htons(0);
        udp_sa.sin_addr.s_addr = INADDR_ANY;

        if (bind(udp_s, (struct sockaddr *)&udp_sa, sizeof(udp_sa)) == SOCKET_ERROR) {
            log_msg(LOG_ERROR, "UDP bind failed: %d", WSAGetLastError());
            closesocket(udp_s);
            return GENERIC_FUNCTION_ERROR;
        }

        device->implementation->disc_handle = udp_s;

    #else
    #   warning Unsupported OS
    #endif

    log_msg(LOG_DEBUG, "UDP discovery socket opened");

    device->implementation->handle = SOCK_T_INVALID;
    #ifdef OS_POSIX
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            perror("socket");
            return GENERIC_FUNCTION_ERROR;
        }

        struct sockaddr_in sa;
        bzero(&sa, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(atoi(port_str));

        if (inet_pton(AF_INET, host, &sa.sin_addr) != 1) {
            perror("inet_pton");
            close(fd);
            return GENERIC_FUNCTION_ERROR;
        }

        if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            perror("connect");
            close(fd);
            return GENERIC_FUNCTION_ERROR;
        }

        device->implementation->handle = fd;
    #elif defined OS_WINDOWS
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
            log_msg(LOG_ERROR, "WSAStartup failed");
            return GENERIC_FUNCTION_ERROR;
        }

        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) {
            log_msg(LOG_ERROR, "socket failed: %d", WSAGetLastError());
            WSACleanup();
            return GENERIC_FUNCTION_ERROR;
        }

        struct sockaddr_in sa;
        ZeroMemory(&sa, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons((u_short)atoi(port_str));

        if (inet_pton(AF_INET, host, &sa.sin_addr) != 1) {
            log_msg(LOG_ERROR, "invalid address: %s", host);
            closesocket(s);
            WSACleanup();
            return GENERIC_FUNCTION_ERROR;
        }

        if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR) {
            log_msg(LOG_ERROR, "connect failed: %d", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return GENERIC_FUNCTION_ERROR;
        }

        device->implementation->handle = s;
        log_msg(LOG_DEBUG, "Opening TCP connection: %s", device->location);
        if (
            device->implementation->handle == SOCK_T_INVALID
        ) {
            log_msg(LOG_WARNING, "Cannot open the device %s", device->location);
            device->status = DEVICE_DOIP_STATUS_ERROR;
            return GENERIC_FUNCTION_ERROR;
        }
        log_msg(LOG_DEBUG, "Openning device: %s", device->location);
    #else
    #   warning Unsupported OS            
    #endif

    device->state = AD_DEVICE_STATE_READY;

    log_msg(LOG_DEBUG, "DoIP device openned");

    return GENERIC_FUNCTION_SUCCESS;
}
static char* doip_describe_communication_layer(final object_DoIPDevice* device) {
    return strdup("DoIP");
}

static bool doip_parse_data(final object_DoIPDevice* device, final Vehicle* vehicle) {
    final Buffer * address = buffer_slice(device->recv_buffer, 0, DOIP_MESSAGE_DIAG_ADDR_SZ); 
    buffer_left_shift(device->recv_buffer, DOIP_MESSAGE_DIAG_ADDR_SZ);
    final ECU* ecu = vehicle_ecu_add_if_not_in(vehicle, address->buffer, address->size); 
    buffer_free(address); 
    list_Buffer_append(ecu->data_buffer,buffer_copy(device->recv_buffer));
    return true;
}

static bool doip_set_filter_by_address(final object_DoIPDevice* device, list_Buffer * filter_addresses) {
    return true;
}

static void doip_clear_data(final object_DoIPDevice* device) {
    buffer_recycle(device->recv_buffer);
}

static void doip_lock(final object_DoIPDevice* device) {
    pthread_mutex_lock(&device->implementation->lock_mutex);
}

static void doip_unlock(final object_DoIPDevice* device) {
    pthread_mutex_unlock(&device->implementation->lock_mutex);
}
void doip_set_tester_address(final object_DoIPDevice* device, uint16_t address) {
    device->address = address;
}
static int doip_send(final object_DoIPDevice * device, const char * command) {
    Buffer * diag_message = buffer_from_ascii_hex(command);
    if ( diag_message == null ) {
        log_msg(LOG_ERROR, "Message sent is not ascii hex, cannot be sent");
        return DEVICE_ERROR;
    }
    object_DoIPMessage * msg = doip_message_diag(
        device->node.address, 
        buffer_from_uint16(device->address),
        diag_message
    );
    buffer_free(diag_message);
    return doip_send_internal(device, buffer_to_hex_string(doip_message_serialize(msg)));
}
int doip_send_internal(final object_DoIPDevice * device, const char * command) {
    assert(command != null);

    Buffer * request = buffer_from_ascii_hex(command);
    if ( request == null ) {
        request = buffer_from_ascii(command);
    }
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "Sending");
        buffer_dump(request);
    }

    if (device->implementation->handle == SOCK_T_INVALID) {
        device->state = AD_DEVICE_STATE_NOT_READY;
        return DEVICE_ERROR;
    }

    int bytes_sent = 0;
    int write_len_rv = 0;
    int poll_result = -1;
    
    if ( device->implementation->handle != SOCK_T_INVALID ) {
        #ifdef OS_POSIX
            poll_result = file_pool_write_posix(device->implementation->handle, device->timeout_ms);
        #elif defined OS_WINDOWS
            poll_result = file_pool_write(&device->implementation->handle, device->timeout_ms);
        #else
        #   warning Unsupported OS
        #endif
    }

    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "Error while polling");
        return DEVICE_ERROR;
    } else if ( poll_result == 0 ) {
        log_msg(LOG_ERROR, "Timeout while polling for write");
        return 0;
    }
    #ifdef OS_POSIX
        bytes_sent = write(device->implementation->handle,request->buffer,request->size);
        if ( bytes_sent != request->size ) {
            perror("write");
            log_msg(LOG_ERROR, "Error while writting to the doip");
            return DEVICE_ERROR;
        }
    #elif defined OS_WINDOWS
        DWORD bytes_written;

        int sent = send(device->implementation->handle, (const char *)request->buffer, request->size, 0);
        if (sent <= 0 || sent != request->size) {
            log_msg(LOG_ERROR, "send failed: %d", WSAGetLastError());
            return DEVICE_ERROR;
        }
        bytes_sent = sent;
        
        if (bytes_sent != request->size) {
            log_msg(LOG_ERROR, "Error while writting to the doip");
            return DEVICE_ERROR;
        }
    #endif
    return bytes_sent;
}
static int doip_recv(final object_DoIPDevice * device) {
    bool accepted = false;
    int tries = 10;
    for(int conv_state = 0; conv_state < tries; conv_state++) {
        int res = doip_recv_internal(device);
        if ( res == DEVICE_ERROR ) {
            return DEVICE_ERROR;
        }
        if ( res == DEVICE_RECV_NULL ) {
            return DEVICE_RECV_NULL;
        }
        object_DoIPMessage *msg = doip_message_parse(device->recv_buffer);
        if ( msg == null ) {
            return DEVICE_RECV_NULL;
        }
        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                if ( ! accepted ) {
                    log_msg(LOG_WARNING, "Received a diag message without previous ACK (suspisious) never mind ...");
                }
                object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
                buffer_recycle(device->recv_buffer);
                buffer_append(device->recv_buffer, payload->src_addr);
                if ( log_has_level(LOG_DEBUG) ) {
                    log_msg(LOG_DEBUG, "Received the payload:");
                    buffer_dump(payload->data);
                }
                buffer_slice_append(device->recv_buffer, payload->data, 0, payload->data->size);
            } return DEVICE_RECV_DATA;
            case DOIP_DIAGNOSTIC_MESSAGE_ACK: {
                if ( accepted ) {
                    log_msg(LOG_WARNING, "Already accepted message");
                }
                accepted = true;
                Buffer * serialized = doip_message_serialize(msg);
                buffer_left_shift(device->recv_buffer, serialized->size);
                buffer_free(serialized);
            } break;
            case DOIP_DIAGNOSTIC_MESSAGE_NACK: {
                if ( accepted ) {
                    log_msg(LOG_ERROR, "Cannot guess whether or diag message is accepted");
                }
            } return DEVICE_RECV_DATA_UNAVAILABLE;
            default: {
                log_msg(LOG_WARNING, "Received message with unsupported payload type 0x%04X ignoring", msg->payload_type);
            } break;
        }
        object_DoIPMessage_free(msg);
    }
    log_msg(LOG_WARNING, "Everything has been tried (%d tries) but cannot retrived response from the node", tries);
    return DEVICE_RECV_NULL;
}
int doip_recv_internal(final object_DoIPDevice * device) {
    int readLen = DEVICE_ERROR;
    
    if (device->implementation->handle == SOCK_T_INVALID) {
        device->state = AD_DEVICE_STATE_NOT_READY;
        return DEVICE_ERROR;
    }

    #if defined OS_POSIX
        int res = file_pool_read_posix(device->implementation->handle, &readLen, device->timeout_ms);
        if ( 0 < res ) {
            buffer_ensure_capacity(device->recv_buffer, readLen);
            final int bytes_readed = read(device->implementation->handle, device->recv_buffer->buffer + device->recv_buffer->size, readLen);
            if( 0 < bytes_readed ) {
                device->recv_buffer->size += bytes_readed;
            } else if ( bytes_readed == 0 ) {
                log_msg(LOG_ERROR, "error during reception should read %d bytes", readLen);
            } else {
                perror("read");
                return DEVICE_ERROR;
            }
            if ( log_has_level(LOG_DEBUG) ) {
                log_msg(LOG_DEBUG, "ip data received");
                buffer_dump(device->recv_buffer);
            }
        } else if ( res == -1 ) {
            perror("poll");
        } else if ( res == 0 ) {
            log_msg(LOG_DEBUG, "Timeout while reading data");
        } else {
            log_msg(LOG_ERROR, "unexpected happen on doip line");
        }
    #elif defined OS_WINDOWS

        DWORD bytes_readed = 0;

        int res = file_pool_read(&device->implementation->handle, &readLen, device->timeout_ms);

        if ( res == -1 ) {
            log_msg(LOG_ERROR, "Error while polling");
        } else if ( res == 0 ) {
            log_msg(LOG_WARNING, "Timeout while polling");
        }
        
        buffer_ensure_capacity(device->recv_buffer, readLen);
        int r = recv(
            device->implementation->handle,
            (char *)device->recv_buffer->buffer + device->recv_buffer->size,
            readLen,
            0
        );

        if (r > 0) {
            device->recv_buffer->size += r;
        } else {
            log_msg(LOG_ERROR, "recv failed: %d", WSAGetLastError());
            return DEVICE_ERROR;
        }

    #endif
    return readLen;
}
object_DoIPDevice * object_DoIPDevice_new() {
    object_DoIPDevice * device = (object_DoIPDevice*)malloc(sizeof(object_DoIPDevice));
    device->type = AD_DEVICE_TYPE_DOIP;
    device->timeout_ms = DEVICE_DOIP_DEFAULT_TIMEOUT_MS;
    device->address = DEVICE_DOIP_DEFAULT_ADDRESS;
    device->implementation = (DoIPDeviceImplementation*)malloc(sizeof(DoIPDeviceImplementation));
    device->recv_buffer = buffer_new();
    device->node.max_data_size = DOIP_MESSAGE_ENTITY_STATUS_DEFAULT_MAX_DATA_SIZE;
    device->node.node_type = DOIP_MESSAGE_ENTITY_NODE_TYPE_UNSET;
    device->implementation->handle = SOCK_T_INVALID;
    device->implementation->disc_handle = SOCK_T_INVALID;
    pthread_mutex_init(&device->implementation->lock_mutex, NULL);
    device->location = null;
    device->state = AD_DEVICE_STATE_UNDEFINED;
    device->send = AD_DEVICE_SEND(doip_send);
    device->recv = AD_DEVICE_RECV(doip_recv);
    device->clear_data = AD_DEVICE_CLEAR_DATA(doip_clear_data);
    device->parse_data = AD_DEVICE_PARSE_DATA(doip_parse_data);
    device->describe_communication_layer = AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(doip_describe_communication_layer);
    device->describe_state = AD_DEVICE_DESCRIBE_STATE(device_describe_state);
    device->set_filter_by_address = AD_DEVICE_SET_FILTER_BY_ADDRESS(doip_set_filter_by_address);
    device->lock = AD_DEVICE_LOCK(doip_lock);
    device->unlock = AD_DEVICE_UNLOCK(doip_unlock);
    device->open = AD_DEVICE_OPEN(doip_open);
    device->close = AD_DEVICE_CLOSE(doip_close);
    device->free = null;
    device->node.address = buffer_new();
    return device;
}

void object_DoIPDevice_free(object_DoIPDevice *device) {
    if ( device != null ) {
        buffer_free(device->node.address);
        free(device);
    }
}

object_DoIPDevice * object_DoIPDevice_assign(object_DoIPDevice * to, object_DoIPDevice * from) {
    memcpy(to, from, sizeof(object_DoIPDevice));
    buffer_assign(to->node.address, from->node.address);
    return to;
}