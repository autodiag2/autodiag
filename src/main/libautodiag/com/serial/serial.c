#include "libautodiag/com/serial/serial.h"

int ad_serial_guess_response(final char * buffer) {
    log_debug("guess for %s", buffer);
    for(int i = 0; i < SerialResponseStrNumber; i++) {
        assert(SerialResponseStr[i] != null);
        if ( strncmp(buffer, SerialResponseStr[i], strlen(SerialResponseStr[i])) == 0 ) {
            log_debug("found serial response : %s", SerialResponseStr[i]);
            return SerialResponseOffset + i;
        }
    }
    return DEVICE_RECV_DATA;
}
int ad_serial_send_internal(final Serial * port, char * tx_buf, int bytes_to_send) {
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "Sending");
        bytes_dump((byte*)tx_buf,bytes_to_send);
    }
    
    if ( ad_object_handle_t_invalid(port->implementation->handle) ) {
        port->state = AD_DEVICE_STATE_NOT_READY;
        return DEVICE_ERROR;
    }

    int write_len_rv = 0;
    int poll_result = ad_object_handle_t_poll_write(port->implementation->handle, port->timeout);
    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "Error while polling");
        return DEVICE_ERROR;
    } else if ( poll_result == 0 ) {
        log_msg(LOG_ERROR, "Timeout while polling for write");
        return 0;
    }
    int result = ad_object_handle_t_write(port->implementation->handle, (byte*)tx_buf, bytes_to_send);
    if ( result == -1 ) {
        ad_serial_close(port);
        return DEVICE_ERROR;
    }
    assert(0 <= result);
    return result;
}

int ad_serial_send(final Serial * port, const char *command) {
    if ( port == null || command == null ) {
        return DEVICE_ERROR;
    } else {
        final int useless_termination = 1;
        final int bytes_to_send = strlen(command) + strlen(port->eol);
        char tx_buf[bytes_to_send + useless_termination];
        sprintf(tx_buf, "%s%s", command, port->eol);
        return ad_serial_send_internal(port, tx_buf, bytes_to_send);
    }
}
int ad_serial_recv_internal(final Serial * port) {
    if ( port == null || port->recv_buffer == null ) {
        return DEVICE_ERROR;
    }
    if ( ad_object_handle_t_invalid(port->implementation->handle) ) {
        port->state = AD_DEVICE_STATE_NOT_READY;
        return DEVICE_ERROR;
    }
    final unsigned initial_buffer_sz = port->recv_buffer->size;
    int maxReadLen = 1024;
    log_msg(LOG_DEBUG, "polling");
    int res = ad_object_handle_t_poll_read(port->implementation->handle, null, port->timeout);
    if ( 0 < res ) {
        while ( 0 < res && ! ad_object_handle_t_invalid(port->implementation->handle) ) {
            ad_buffer_ensure_capacity(port->recv_buffer, maxReadLen);
            final int bytes_readed = ad_object_handle_t_read(port->implementation->handle, port->recv_buffer->buffer + port->recv_buffer->size, maxReadLen);
            if( 0 < bytes_readed ) {
                port->recv_buffer->size += bytes_readed;
            } else if ( bytes_readed == 0 ) {
                break;
            } else {
                perror("read");
                break;
            }
            res = ad_object_handle_t_poll_read(port->implementation->handle, null, port->timeout_seq);
        }
        if ( log_has_level(LOG_DEBUG) ) {
            log_msg(LOG_DEBUG, "Serial data received");
            ad_buffer_dump(port->recv_buffer);
        }
    } else if ( res == -1 ) {
        perror("poll");
    } else if ( res == 0 ) {
        log_msg(LOG_DEBUG, "Timeout while reading data");
    } else {
        log_msg(LOG_ERROR, "unexpected happen on serial line");
    }
    
    return port->recv_buffer->size - initial_buffer_sz;
}

#define SERIAL_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = ad_serial_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }

GEN_SERIAL_RECV(ad_serial_recv,Serial,SERIAL_RECV_ITERATOR)

int ad_serial_open(final Serial * port) {
    if ( port == null ) {
        log_msg(LOG_INFO, "Open: Cannot open since not serial port info given");
        return GENERIC_FUNCTION_ERROR;
    }
    
    // Do not open serial if it has not been configured.
    if (port->location == null) {
        log_msg(LOG_DEBUG, "Open: Cannot open since no name on the serial");
        return GENERIC_FUNCTION_ERROR;
    }

    ad_serial_close(port);

    const char *addr = port->location;
    char host[500];
    char port_str[8] = "35000";
    if ( ad_device_is_network((Device*)port) ) {
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
    }

    if (ad_device_is_network((Device*)port)) {
        #if defined OS_POSIX
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
                log_msg(LOG_ERROR, "inet_pton: %s", strerror(errno));
                close(fd);
                return GENERIC_FUNCTION_ERROR;
            }

            if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
                log_msg(LOG_ERROR, "connect to %s: %s", network_location(sa), strerror(errno));
                close(fd);
                return GENERIC_FUNCTION_ERROR;
            }

            port->implementation->handle->posix_handle = fd;
            port->state = AD_DEVICE_STATE_READY;
            log_msg(LOG_DEBUG, "Open: Serial openned as posix socket");
            return GENERIC_FUNCTION_SUCCESS;
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

            port->implementation->handle->win_socket = s;
            port->state = AD_DEVICE_STATE_READY;
            log_msg(LOG_DEBUG, "Open: Serial openned as windows socket");
            return GENERIC_FUNCTION_SUCCESS;
        #else
        #   warning networking mode unsupported for this OS
        #endif
    }

    #if defined OS_WINDOWS
    {
        port->implementation->handle->win_handle = CreateFile(port->location, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        if (port->implementation->handle->win_handle == INVALID_HANDLE_VALUE) {
            log_msg(LOG_WARNING, "Cannot open the port %s", port->location);
            port->state = AD_DEVICE_STATE_NOT_READY;
            port->ad_serial_state = SERIAL_STATE_OPEN_ERROR;
            return GENERIC_FUNCTION_ERROR;
        }
        log_msg(LOG_DEBUG, "Openning port: %s", port->location);
        
        if ( isComPort(port->implementation->handle->win_handle) ) {
            assert(0 <= port->baud_rate);
            #define TX_TIMEOUT_MULTIPLIER    0
            #define TX_TIMEOUT_CONSTANT      1000

            DCB dcb;
            COMMTIMEOUTS timeouts;
            DWORD bytes_written;

            ZeroMemory(&dcb, sizeof(DCB));
            dcb.DCBlength = sizeof(DCB);
            if (!GetCommState(port->implementation->handle->win_handle, &dcb)) {
                log_msg(LOG_ERROR, "GetCommState failed for %s", port->location);
                CloseHandle(port->implementation->handle->win_handle);
                return GENERIC_FUNCTION_ERROR;
            }
            dcb.BaudRate = port->baud_rate;
            dcb.ByteSize = 8;
            dcb.StopBits = ONESTOPBIT;
            dcb.fParity = FALSE;
            dcb.Parity = NOPARITY;
            dcb.fOutxCtsFlow = FALSE;
            dcb.fOutxDsrFlow = FALSE;
            dcb.fOutX = FALSE;
            dcb.fInX = FALSE;
            dcb.fDtrControl = DTR_CONTROL_ENABLE;
            dcb.fRtsControl = RTS_CONTROL_ENABLE;
            dcb.fDsrSensitivity = FALSE;
            dcb.fErrorChar = FALSE;
            dcb.fAbortOnError = FALSE;
            if (!SetCommState(port->implementation->handle->win_handle, &dcb)) {
                log_msg(LOG_ERROR, "SetCommState failed for %s", port->location);
                CloseHandle(port->implementation->handle->win_handle);
                return GENERIC_FUNCTION_ERROR;
            }

            ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
            timeouts.ReadIntervalTimeout = port->timeout;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = port->timeout;
            timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER;
            timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT;
            if (!SetCommTimeouts(port->implementation->handle->win_handle, &timeouts)) {
                log_msg(LOG_ERROR, "SetCommTimeouts failed for %s", port->location);
                CloseHandle(port->implementation->handle->win_handle);
                return GENERIC_FUNCTION_ERROR;
            }

            // Hack to get around Windows 2000 multiplying timeout values by 15
            GetCommTimeouts(port->implementation->handle->win_handle, &timeouts);
            if (TX_TIMEOUT_MULTIPLIER > 0) {
                timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER * TX_TIMEOUT_MULTIPLIER / timeouts.WriteTotalTimeoutMultiplier;
            }
            if (TX_TIMEOUT_CONSTANT > 0) {
                timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT * TX_TIMEOUT_CONSTANT / timeouts.WriteTotalTimeoutConstant;
            }
            SetCommTimeouts(port->implementation->handle->win_handle, &timeouts);

            // If the port is Bluetooth, make sure device is active
            PurgeComm(port->implementation->handle->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            WriteFile(port->implementation->handle->win_handle, "?\r", 2, &bytes_written, 0);
            PurgeComm(port->implementation->handle->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            if (bytes_written != 2) { // If Tx timeout occured
                log_msg(LOG_WARNING, "Inactive port detected %s", port->location);
                CloseHandle(port->implementation->handle->win_handle);
                port->ad_serial_state = SERIAL_STATE_OPEN_ERROR;
                port->state = AD_DEVICE_STATE_NOT_READY;
                return GENERIC_FUNCTION_ERROR;
            }
            log_msg(LOG_DEBUG, "Open: Serial openned as windows COM port");
        } else {
            log_msg(LOG_DEBUG, "Open: Serial openned as windows regular file (named pipe)");
        }
    }
    #elif defined OS_POSIX
    {
        assert(0 <= port->baud_rate);
        port->implementation->handle->posix_handle = open(port->location, O_RDWR | O_NOCTTY);
        if (port->implementation->handle->posix_handle < 0) {
            perror(port->location);
            port->state = AD_DEVICE_STATE_NOT_READY;
            if ( errno == ENOENT ) {
                port->ad_serial_state = SERIAL_STATE_DISCONNECTED;
            } else if ( errno == EPERM ) {
                port->ad_serial_state = SERIAL_STATE_MISSING_PERM;
            } else {
                port->ad_serial_state = SERIAL_STATE_OPEN_ERROR;
            }
            return GENERIC_FUNCTION_ERROR;
        }

        tcgetattr(port->implementation->handle->posix_handle, &(port->implementation->oldtio)); /* save current port settings */

        bzero(&(port->implementation->newtio), sizeof(port->implementation->newtio));

        cfsetspeed(&(port->implementation->newtio), port->baud_rate);

        cfmakeraw(&(port->implementation->newtio));
        port->implementation->newtio.c_cflag |= (CLOCAL | CREAD);

        // No parity (8N1):
        port->implementation->newtio.c_cflag &= ~PARENB;
        port->implementation->newtio.c_cflag &= ~CSTOPB;
        port->implementation->newtio.c_cflag &= ~CSIZE;
        port->implementation->newtio.c_cflag |= CS8;

        // disable hardware flow control
        port->implementation->newtio.c_cflag &= ~CRTSCTS ;

        port->implementation->newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
        port->implementation->newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

        tcflush(port->implementation->handle->posix_handle, TCIFLUSH);
        tcsetattr(port->implementation->handle->posix_handle,TCSANOW,&(port->implementation->newtio));
    }
    #else
    #   warning Regular file openning not supported on this OS
    #endif

    port->state = AD_DEVICE_STATE_READY;
    log_msg(LOG_DEBUG, "Open: Serial openned");
    return GENERIC_FUNCTION_SUCCESS;
}
void ad_serial_close(final Serial * port) {
    if ( port == null || port->state != AD_DEVICE_STATE_READY ) {
        log_msg(LOG_INFO, "Close: device not open");
        return;
    }
    #ifdef OS_POSIX
        #ifndef OS_WINDOWS
            if (!ad_device_is_network((Device*)port)) {
                tcsetattr(port->implementation->handle->posix_handle,
                        TCSANOW,
                        &port->implementation->oldtio);
            }
        #endif
    #endif
    ad_object_handle_t_close(port->implementation->handle);
    port->state = AD_DEVICE_STATE_NOT_READY;
}
const char * ad_serial_describe_state(final Serial * port) {
    const char * parent = ad_device_describe_state((Device*)port);

    if ( parent != null ) {
        return parent;
    }

    switch(port->ad_serial_state) {
        case SERIAL_STATE_USER_IGNORED:
            return "User ignored (eg. user ignored the serial port during the scan)";
        case SERIAL_STATE_OPEN_ERROR:
            return "Error while opening serial port (eg permissions not sufficient)";
        case SERIAL_STATE_DISCONNECTED:
            return "Serial port disconnected";
        case SERIAL_STATE_MISSING_PERM:
            return "Missing permissions to open the serial port";
    }

    return null;
}
char * ad_serial_describe_communication_layer(final Serial * serial) {
    char * res;
    asprintf(&res,"Serial (%d bauds)", serial->baud_rate);
    return res;
}
static void clear_data(final Serial* serial) {
    ad_buffer_recycle(serial->recv_buffer);
}
void ad_serial_lock(final Serial * port) {
    pthread_mutex_lock(&port->implementation->lock_mutex);
}
void ad_serial_unlock(final Serial * port) {
    pthread_mutex_unlock(&port->implementation->lock_mutex);
}

Serial * ad_serial_new() {
    final Serial * port = (Serial *)malloc(sizeof(Serial));
    port->implementation = (SerialImplementation *)malloc(sizeof(SerialImplementation));
    port->type = AD_DEVICE_TYPE_SERIAL;
    ad_serial_init(port);
    return port;
}
void ad_serial_reset_to_default(final Serial* serial) {
    serial->echo = true;
    serial->eol = strdup("\r\n");
}

void ad_serial_init(final Serial* serial) {
    ad_serial_reset_to_default(serial);
    serial->location = null;
    serial->state = AD_DEVICE_STATE_UNDEFINED;
    serial->recv_buffer = ad_buffer_new();
    serial->timeout = SERIAL_DEFAULT_TIMEOUT;
    serial->timeout_seq = SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT;
    serial->detected = false;
    serial->open = AD_DEVICE_OPEN(ad_serial_open);
    serial->close = AD_DEVICE_CLOSE(ad_serial_close);
    serial->send = AD_DEVICE_SEND(ad_serial_send);
    serial->recv = AD_DEVICE_RECV(ad_serial_recv);
    serial->free = AD_DEVICE_FREE(ad_serial_free);
    serial->describe_communication_layer = AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(ad_serial_describe_communication_layer);
    serial->parse_data = null;
    serial->set_filter_by_address = null;
    serial->guess_response = AD_SERIAL_GUESS_RESPONSE(ad_serial_guess_response);
    serial->lock = AD_DEVICE_LOCK(ad_serial_lock);
    serial->unlock = AD_DEVICE_UNLOCK(ad_serial_unlock);
    serial->clear_data = AD_DEVICE_CLEAR_DATA(clear_data);
    serial->baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    serial->describe_state = AD_DEVICE_DESCRIBE_STATE(ad_serial_describe_state);
    pthread_mutex_init(&serial->implementation->lock_mutex, NULL);
    serial->implementation->handle = ad_object_handle_t_new();
}

void ad_serial_free(final Serial * port) {
    if ( port != null ) {
        if (port->location != null ) {
            free(port->location);
            port->location = null;
        }
        if ( port->eol != null ) {
            free(port->eol);
            port->eol = null;
        }
        if ( port->recv_buffer != null ) {
            ad_buffer_free(port->recv_buffer);
            port->recv_buffer = null;
        }
        if ( port->implementation != null ) {
            pthread_mutex_destroy(&port->implementation->lock_mutex);
            free(port->implementation);
            port->implementation = null;
        }
        free(port);
        port = null;
    }
}

void ad_serial_dump(final Serial * port) {
    const char * title = "Serial dump (name/baud_rate/state/eol):";
    if ( port == null ) {
        log_msg(LOG_DEBUG, "%s NULL", title);
    } else {
        log_msg(LOG_DEBUG, "%s %s/%d/%d/%s", title, port->location, port->baud_rate, port->state, port->eol);
    }
}

void ad_serial_debug(final Serial * port) {
    if ( port == null ) {
        log_msg(LOG_DEBUG, "Serial debug: NULL");
    } else {
        log_msg(LOG_DEBUG, "Serial: {");
        ad_device_debug((Device*)port);
        log_msg(LOG_DEBUG, "    echo: %s", port->echo ? "true" : "false");
        log_msg(LOG_DEBUG, "    baud_rate: %d", port->baud_rate);
        log_msg(LOG_DEBUG, "    state: %d", port->state);
        log_msg(LOG_DEBUG, "    name: %s", port->location);
        log_msg(LOG_DEBUG, "    eol: %s", port->eol);
        log_msg(LOG_DEBUG, "    timeout: %d ms", port->timeout);
        log_msg(LOG_DEBUG, "    timeout_seq: %d ms", port->timeout_seq);
        log_msg(LOG_DEBUG, "    recv_buffer: %p", port->recv_buffer);
        log_msg(LOG_DEBUG, "    detected: %s", port->detected ? "true" : "false");
        log_msg(LOG_DEBUG, "    guess_response: %p", port->guess_response);
        log_msg(LOG_DEBUG, "}");
    }
}

bool at_is_command(char * command) {
    return strncasecmp(command,"at",2) == 0;
}

char * at_command_va(char * at_command, va_list ap) {
        
    char * atCmd = null;
    asprintf(&atCmd,"at%s", at_command);
    
    char *res = null;
    if ( compat_vasprintf(&res, atCmd, ap) == -1 ) {
        log_msg(LOG_ERROR, "Error with at command");
    }

    return res;
}

char * at_command(char * at_command, ...) {

    va_list ap;
    va_start(ap, at_command);
        
    char * res = at_command_va(at_command,ap);

    va_end(ap);

    return res;
}
char * at_command_boolean(char *cmd, final bool state) {
    return at_command("%s%d", cmd,state);
}
bool ad_serial_send_at_command_internal(final Serial* serial, char *cmd, va_list ap) {
    
    char * cmdAt = at_command_va(cmd, ap);

    va_end(ap);

    if ( serial->send(AD_DEVICE(serial), cmdAt) == DEVICE_ERROR ) {
        return false;
    }
    free(cmdAt);
    return true;
}
bool ad_serial_send_at_command(final Serial* serial, char *cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    return ad_serial_send_at_command_internal(serial, cmd, ap);
}
bool ad_serial_query_at_command(final Serial* serial, char *cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    if ( ! ad_serial_send_at_command_internal(serial, cmd, ap) ) {
        return false;
    }

    ad_buffer_recycle(serial->recv_buffer);
    bool result = serial->recv(AD_DEVICE(serial)) == SERIAL_RESPONSE_OK;
    
    return result;
}
bool ascii_is_control_char(char c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

char *ad_serial_at_reduce(char *str) {
    bool a_detected = false, t_detected = false;
    size_t len = strlen(str);
    char *result = malloc(len + 1);
    if (!result) return NULL;
    int ri = 0;

    for (size_t i = 0; i < len; i++) {
        if (ascii_is_control_char(str[i]) || str[i] == ' ') continue;

        char c = tolower(str[i]);

        if (!a_detected && c == 'a') {
            a_detected = true;
            continue;
        }
        if (a_detected && !t_detected && c == 't') {
            t_detected = true;
            continue;
        }

        if (a_detected && t_detected) {
            result[ri++] = c;
        } else {
            free(result);
            return NULL;
        }
    }

    if (!a_detected || !t_detected) {
        free(result);
        return NULL;
    }

    result[ri] = '\0';
    return result;
}

int ad_serial_at_parse_reduced(char *reduced, char *atcmd) {
    int idx = -1;
    char * cmdFull;
    asprintf(&cmdFull,"at%s",atcmd);
    if ( strcasebeginwith(reduced, cmdFull) ) {
        idx = strlen(cmdFull);
    }
    free(cmdFull);
    return idx;
}