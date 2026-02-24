#include "libautodiag/com/doip/device.h"
#include "libautodiag/com/doip/doip.h"

bool doip_configure(final object_DoIPDevice * device) {
    log_msg(LOG_DEBUG, "Configuring doip device");
    {
        object_DoIPMessage * msg = doip_message_new(DOIP_ROUTING_ACTIVATION_REQUEST);
        object_DoIPMessagePayloadRoutineActivationRequest * payload = object_DoIPMessagePayloadRoutineActivationRequest_new();
        payload->src_addr[0] = (device->address >> 8) & 0xFF;
        payload->src_addr[1] = device->address & 0xFF;
        msg->payload = (DoIPMessageDef*)payload;
        doip_send_internal(device, buffer_to_hex_string(doip_message_serialize(msg)));
        object_DoIPMessage_free(msg);
    }
    {
        buffer_recycle(device->recv_buffer);
        doip_recv_internal(device);
        object_DoIPMessage * msg = doip_message_parse(device->recv_buffer);
        switch(msg->payload_type) {
            case DOIP_ROUTING_ACTIVATION_RESPONSE: {
                object_DoIPMessagePayloadRoutineActivationResponse * payload = (object_DoIPMessagePayloadRoutineActivationResponse*)msg->payload;
                switch(payload->code) {
                    case DOIP_MESSAGE_RARES_CODE_SUCCESS: {
                        object_DoIPMessage_free(msg);
                    } return true;
                    default: {
                        log_msg(LOG_ERROR, "Doip node refusing to start diagnostic maybe implement manufacturer specific process");
                        object_DoIPMessage_free(msg);
                    } return false;
                }
            } break;
            default: {
                log_msg(LOG_ERROR, "Received 0x%04X instead of 0x%04X aborting the configure", msg->payload_type, DOIP_ROUTING_ACTIVATION_RESPONSE);
                object_DoIPMessage_free(msg);
            } return false;
        }
    }
    return false;
}

void doip_close(final object_DoIPDevice * device) {
    if ( device == null || device->status != DEVICE_DOIP_STATUS_OPEN ) {
        log_msg(LOG_INFO, "Close: device not open");
        return;
    } 
    assert(device_location_is_network((Device*)device));
    #if defined OS_POSIX
        if (device->implementation->handle >= 0) {
            shutdown(device->implementation->handle, SHUT_RDWR);
            close(device->implementation->handle);
        }
        device->implementation->handle = -1;
    #elif defined OS_WINDOWS
        if (isSocketHandle(device->implementation->win_handle)) {
            SOCKET s = (SOCKET)device->implementation->win_handle;
            shutdown(s, SD_BOTH);
            closesocket(s);
        }
        device->implementation->win_handle = INVALID_HANDLE_VALUE;
    #endif
    device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
}
static int doip_open_internal(final object_DoIPDevice * device) {
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

    #ifdef OS_POSIX
        device->implementation->handle = -1;
    #endif
    #ifdef OS_WINDOWS
        device->implementation->win_handle == INVALID_HANDLE_VALUE;
    #endif

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

        device->implementation->win_handle = (HANDLE)s;
        log_msg(LOG_DEBUG, "Opening TCP connection: %s", device->location);
        if (
            device->implementation->win_handle == INVALID_HANDLE_VALUE
        ) {
            log_msg(LOG_WARNING, "Cannot open the device %s", device->location);
            device->status = DEVICE_DOIP_STATUS_ERROR;
            return GENERIC_FUNCTION_ERROR;
        }
        log_msg(LOG_DEBUG, "Openning device: %s", device->location);
    #else
    #   warning Unsupported OS            
    #endif

    device->status = DEVICE_DOIP_STATUS_OPEN;

    log_msg(LOG_DEBUG, "DoIP device openned");

    return GENERIC_FUNCTION_SUCCESS;
}
static void doip_open(final object_DoIPDevice * device) {
    doip_open_internal(device);
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
    object_DoIPMessage * msg = doip_message_diag(
        buffer_from_ascii_hex("07E8"), 
        buffer_from_uint16(device->address),
        buffer_from_ascii_hex(command)
    );
    return doip_send_internal(device, buffer_to_hex_string(doip_message_serialize(msg)));
}
int doip_send_internal(final object_DoIPDevice * device, const char * command) {
    assert(command != null);

    Buffer * request = buffer_from_ascii_hex(command);
    if ( request == null ) {
        request = buffer_from_ascii(command);
    }
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "device:doip:Sending");
        buffer_dump(request);
    }

    #ifdef OS_POSIX
        if (device->implementation->handle < 0) {
            device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
            return DEVICE_ERROR;
        }
    #elif defined OS_WINDOWS
        if (device->implementation->win_handle == INVALID_HANDLE_VALUE) {
            device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
            return DEVICE_ERROR;
        }
    #endif
    int bytes_sent = 0;
    int write_len_rv = 0;
    int poll_result = -1;
    #ifdef OS_POSIX
        if ( device->implementation->handle != -1 ) {
            poll_result = file_pool_write_posix(device->implementation->handle, device->timeout_ms);
        }
    #elif defined OS_WINDOWS
        if ( device->implementation->win_handle != INVALID_HANDLE_VALUE ) {
            poll_result = file_pool_write(&device->implementation->win_handle, device->timeout_ms);
        }
    #else
    #   warning Unsupported OS
    #endif
    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "device:doip:Error while polling");
        return DEVICE_ERROR;
    } else if ( poll_result == 0 ) {
        log_msg(LOG_ERROR, "device:doip:Timeout while polling for write");
        return 0;
    }
    #ifdef OS_POSIX
        bytes_sent = write(device->implementation->handle,request->buffer,request->size);
        if ( bytes_sent != request->size ) {
            perror("device:doip:write");
            log_msg(LOG_ERROR, "device:doip:Error while writting to the doip");
            return DEVICE_ERROR;
        }
    #elif defined OS_WINDOWS
        DWORD bytes_written;

        if (isSocketHandle(device->implementation->win_handle)) {
            SOCKET s = (SOCKET)device->implementation->win_handle;

            int sent = send(s, (const char *)request->buffer, request->size, 0);
            if (sent <= 0 || sent != request->size) {
                log_msg(LOG_ERROR, "device:doip:send failed: %d", WSAGetLastError());
                return DEVICE_ERROR;
            }
            bytes_sent = sent;
        }
        if (bytes_sent != request->size) {
            log_msg(LOG_ERROR, "device:doip:Error while writting to the doip");
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
        switch(msg->payload_type) {
            case DOIP_DIAGNOSTIC_MESSAGE: {
                if ( ! accepted ) {
                    log_msg(LOG_WARNING, "Received a diag message without previous ACK (suspisious) never mind ...");
                }
                object_DoIPMessagePayloadDiag * payload = (object_DoIPMessagePayloadDiag*)msg->payload;
                buffer_recycle(device->recv_buffer);
                buffer_append(device->recv_buffer, payload->src_addr);
                if ( log_has_level(LOG_DEBUG) ) {
                    log_msg(LOG_DEBUG, "device:Received the payload:");
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
    log_msg(LOG_WARNING, "device: Everything has been tried (%d tries) but cannot retrived response from the node", tries);
    return DEVICE_RECV_NULL;
}
int doip_recv_internal(final object_DoIPDevice * device) {
    int readLen = DEVICE_ERROR;
    #if defined OS_POSIX
        if (device->implementation->handle < 0) {
            device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
            return DEVICE_ERROR;
        }
        int res = file_pool_read_posix(device->implementation->handle, &readLen, device->timeout_ms);
        if ( 0 < res ) {
            buffer_ensure_capacity(device->recv_buffer, readLen);
            final int bytes_readed = read(device->implementation->handle, device->recv_buffer->buffer + device->recv_buffer->size, readLen);
            if( 0 < bytes_readed ) {
                device->recv_buffer->size += bytes_readed;
            } else if ( bytes_readed == 0 ) {
                log_msg(LOG_ERROR, "device: error during reception should read %d bytes", readLen);
            } else {
                perror("device: read");
                return DEVICE_ERROR;
            }
            if ( log_has_level(LOG_DEBUG) ) {
                log_msg(LOG_DEBUG, "device: ip data received");
                buffer_dump(device->recv_buffer);
            }
        } else if ( res == -1 ) {
            perror("poll");
        } else if ( res == 0 ) {
            log_msg(LOG_DEBUG, "device: Timeout while reading data");
        } else {
            log_msg(LOG_ERROR, "device: unexpected happen on doip line");
        }
    #elif defined OS_WINDOWS
        if (device->implementation->win_handle == INVALID_HANDLE_VALUE) {
            device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
            return DEVICE_ERROR;
        }

        DWORD bytes_readed = 0;

        int res = file_pool_read(&device->implementation->win_handle, &readLen, device->timeout_ms);

        if ( res == -1 ) {
            log_msg(LOG_ERROR, "Error while polling");
        } else if ( res == 0 ) {
            log_msg(LOG_WARNING, "Timeout while polling");
        }
        
        if (isSocketHandle(device->implementation->win_handle)) {
            buffer_ensure_capacity(device->recv_buffer, readLen);
            SOCKET s = (SOCKET)device->implementation->win_handle;
            int r = recv(
                s,
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
        } else {
            log_msg(LOG_ERROR, "cannot proceed on current handler");
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
    #if defined OS_WINDOWS
        device->implementation->win_handle = INVALID_HANDLE_VALUE;
    #endif
    #if defined OS_POSIX
        device->implementation->handle = -1;
    #endif
    pthread_mutex_init(&device->implementation->lock_mutex, NULL);
    device->location = null;
    device->status = DEVICE_DOIP_STATUS_NOT_OPEN;
    device->send = AD_DEVICE_SEND(doip_send);
    device->recv = AD_DEVICE_RECV(doip_recv);
    device->clear_data = AD_DEVICE_CLEAR_DATA(doip_clear_data);
    device->parse_data = AD_DEVICE_PARSE_DATA(doip_parse_data);
    device->describe_communication_layer = AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(doip_describe_communication_layer);
    device->set_filter_by_address = AD_DEVICE_SET_FILTER_BY_ADDRESS(doip_set_filter_by_address);
    device->lock = AD_DEVICE_LOCK(doip_lock);
    device->unlock = AD_DEVICE_UNLOCK(doip_unlock);
    device->open = AD_DEVICE_OPEN(doip_open);
    device->close = AD_DEVICE_CLOSE(doip_close);
    device->free = null;
    return device;
}

void object_DoIPDevice_free(object_DoIPDevice *device) {
    free(device);
}

object_DoIPDevice * object_DoIPDevice_assign(object_DoIPDevice * to, object_DoIPDevice * from) {
    memcpy(to, from, sizeof(object_DoIPDevice));
    return to;
}