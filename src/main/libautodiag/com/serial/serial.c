#include "libautodiag/com/serial/serial.h"

int serial_guess_response(final char * buffer) {
    for(int i = 0; i < SerialResponseStrNumber; i++) {
        assert(SerialResponseStr[i] != null);
        if ( strncmp(buffer, SerialResponseStr[i], strlen(SerialResponseStr[i])) == 0 ) {
            return SerialResponseOffset + i;
        }
    }
    return DEVICE_RECV_DATA;
}
int serial_send_internal(final Serial * port, char * tx_buf, int bytes_to_send) {
    if ( log_has_level(LOG_DEBUG) ) {
        log_msg(LOG_DEBUG, "client:serial:Sending");
        bytes_dump((byte*)tx_buf,bytes_to_send);
    }
    
    if ( object_handle_t_invalid(port->implementation->handle_rename) ) {
        port->status = SERIAL_STATE_NOT_OPEN;
        return DEVICE_ERROR;
    }

    int write_len_rv = 0;
    int poll_result = object_handle_t_poll_write(port->implementation->handle_rename, port->timeout);
    if ( poll_result == -1 ) {
        log_msg(LOG_ERROR, "client:serial:Error while polling");
        return DEVICE_ERROR;
    } else if ( poll_result == 0 ) {
        log_msg(LOG_ERROR, "client:serial:Timeout while polling for write");
        return 0;
    }
    int result = object_handle_t_write(port->implementation->handle_rename, (byte*)tx_buf, bytes_to_send);
    if ( result == -1 ) {
        serial_close(port);
        return DEVICE_ERROR;
    }
    assert(0 <= result);
    return result;
}

int serial_send(final Serial * port, const char *command) {
    if ( port == null || command == null ) {
        return DEVICE_ERROR;
    } else {
        final int useless_termination = 1;
        final int bytes_to_send = strlen(command) + strlen(port->eol);
        char tx_buf[bytes_to_send + useless_termination];
        sprintf(tx_buf, "%s%s", command, port->eol);
        return serial_send_internal(port, tx_buf, bytes_to_send - useless_termination);
    }
}
int serial_recv_internal(final Serial * port) {
    if ( port == null || port->recv_buffer == null ) {
        return DEVICE_ERROR;
    }
    if ( object_handle_t_invalid(port->implementation->handle_rename) ) {
        port->status = SERIAL_STATE_NOT_OPEN;
        return DEVICE_ERROR;
    }
    final unsigned initial_buffer_sz = port->recv_buffer->size;
    int maxReadLen = 1024;
    int res = object_handle_t_poll_read(port->implementation->handle_rename, null, port->timeout);
    if ( 0 < res ) {
        while ( 0 < res && ! object_handle_t_invalid(port->implementation->handle_rename) ) {
            buffer_ensure_capacity(port->recv_buffer, maxReadLen);
            final int bytes_readed = object_handle_t_read(port->implementation->handle_rename, port->recv_buffer->buffer + port->recv_buffer->size, maxReadLen);
            if( 0 < bytes_readed ) {
                port->recv_buffer->size += bytes_readed;
            } else if ( bytes_readed == 0 ) {
                break;
            } else {
                perror("read");
                break;
            }
            res = object_handle_t_poll_read(port->implementation->handle_rename, null, port->timeout_seq);
        }
        if ( log_has_level(LOG_DEBUG) ) {
            log_msg(LOG_DEBUG, "client:serial:Serial data received");
            buffer_dump(port->recv_buffer);
        }
    } else if ( res == -1 ) {
        perror("poll");
    } else if ( res == 0 ) {
        log_msg(LOG_DEBUG, "client:serial:Timeout while reading data");
    } else {
        log_msg(LOG_ERROR, "client:serial:unexpected happen on serial line");
    }
    
    return port->recv_buffer->size - initial_buffer_sz;
}

#define SERIAL_RECV_ITERATOR(ptr,end_ptr) \
    int recv_value = serial_guess_response(ptr); \
    switch(recv_value) { \
        case SERIAL_RESPONSE_PROMPT: \
            continue_reception = false; \
            break; \
        default: \
            deduced_response = recv_value; \
            break; \
    }

GEN_SERIAL_RECV(serial_recv,Serial,SERIAL_RECV_ITERATOR)

int serial_open(final Serial * port) {
    if ( port == null ) {
        log_msg(LOG_INFO, "client:serial:Open: Cannot open since not serial port info given");
        return GENERIC_FUNCTION_ERROR;
    }
    
    // Do not open serial if it has not been configured.
    if (port->location == null) {
        log_msg(LOG_DEBUG, "client:serial:Open: Cannot open since no name on the serial");
        return GENERIC_FUNCTION_ERROR;
    }

    serial_close(port);

    const char *addr = port->location;
    char host[500];
    char port_str[8] = "35000";
    if ( device_location_is_network((Device*)port) ) {
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

    if (device_location_is_network((Device*)port)) {
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
                perror("inet_pton");
                close(fd);
                return GENERIC_FUNCTION_ERROR;
            }

            if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
                perror("connect");
                close(fd);
                return GENERIC_FUNCTION_ERROR;
            }

            port->implementation->handle_rename->posix_handle = fd;
            port->status = SERIAL_STATE_READY;
            log_msg(LOG_DEBUG, "client:serial:Open: Serial openned as posix socket");
            return GENERIC_FUNCTION_SUCCESS;
        #elif defined OS_WINDOWS
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
                log_msg(LOG_ERROR, "client:serial:WSAStartup failed");
                return GENERIC_FUNCTION_ERROR;
            }

            SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (s == INVALID_SOCKET) {
                log_msg(LOG_ERROR, "client:serial:socket failed: %d", WSAGetLastError());
                WSACleanup();
                return GENERIC_FUNCTION_ERROR;
            }

            struct sockaddr_in sa;
            ZeroMemory(&sa, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_port = htons((u_short)atoi(port_str));

            if (inet_pton(AF_INET, host, &sa.sin_addr) != 1) {
                log_msg(LOG_ERROR, "client:serial:invalid address: %s", host);
                closesocket(s);
                WSACleanup();
                return GENERIC_FUNCTION_ERROR;
            }

            if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR) {
                log_msg(LOG_ERROR, "client:serial:connect failed: %d", WSAGetLastError());
                closesocket(s);
                WSACleanup();
                return GENERIC_FUNCTION_ERROR;
            }

            port->implementation->handle_rename->win_socket = s;
            port->status = SERIAL_STATE_READY;
            log_msg(LOG_DEBUG, "client:serial:Open: Serial openned as windows socket");
            return GENERIC_FUNCTION_SUCCESS;
        #else
        #   warning networking mode unsupported for this OS
        #endif
    }

    #if defined OS_WINDOWS
    {
        port->implementation->handle_rename->win_handle = CreateFile(port->location, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
        if (port->implementation->handle_rename->win_handle == INVALID_HANDLE_VALUE) {
            log_msg(LOG_WARNING, "client:serial:Cannot open the port %s", port->location);
            port->status = SERIAL_STATE_OPEN_ERROR;
            return GENERIC_FUNCTION_ERROR;
        }
        log_msg(LOG_DEBUG, "client:serial:Openning port: %s", port->location);
        
        if ( isComPort(port->implementation->handle_rename->win_handle) ) {
            assert(0 <= port->baud_rate);
            #define TX_TIMEOUT_MULTIPLIER    0
            #define TX_TIMEOUT_CONSTANT      1000

            DCB dcb;
            COMMTIMEOUTS timeouts;
            DWORD bytes_written;

            ZeroMemory(&dcb, sizeof(DCB));
            dcb.DCBlength = sizeof(DCB);
            if (!GetCommState(port->implementation->handle_rename->win_handle, &dcb)) {
                log_msg(LOG_ERROR, "client:serial:GetCommState failed for %s", port->location);
                CloseHandle(port->implementation->handle_rename->win_handle);
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
            if (!SetCommState(port->implementation->handle_rename->win_handle, &dcb)) {
                log_msg(LOG_ERROR, "client:serial:SetCommState failed for %s", port->location);
                CloseHandle(port->implementation->handle_rename->win_handle);
                return GENERIC_FUNCTION_ERROR;
            }

            ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
            timeouts.ReadIntervalTimeout = port->timeout;
            timeouts.ReadTotalTimeoutMultiplier = 0;
            timeouts.ReadTotalTimeoutConstant = port->timeout;
            timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER;
            timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT;
            if (!SetCommTimeouts(port->implementation->handle_rename->win_handle, &timeouts)) {
                log_msg(LOG_ERROR, "client:serial:SetCommTimeouts failed for %s", port->location);
                CloseHandle(port->implementation->handle_rename->win_handle);
                return GENERIC_FUNCTION_ERROR;
            }

            // Hack to get around Windows 2000 multiplying timeout values by 15
            GetCommTimeouts(port->implementation->handle_rename->win_handle, &timeouts);
            if (TX_TIMEOUT_MULTIPLIER > 0) {
                timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER * TX_TIMEOUT_MULTIPLIER / timeouts.WriteTotalTimeoutMultiplier;
            }
            if (TX_TIMEOUT_CONSTANT > 0) {
                timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT * TX_TIMEOUT_CONSTANT / timeouts.WriteTotalTimeoutConstant;
            }
            SetCommTimeouts(port->implementation->handle_rename->win_handle, &timeouts);

            // If the port is Bluetooth, make sure device is active
            PurgeComm(port->implementation->handle_rename->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            WriteFile(port->implementation->handle_rename->win_handle, "?\r", 2, &bytes_written, 0);
            PurgeComm(port->implementation->handle_rename->win_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            if (bytes_written != 2) { // If Tx timeout occured
                log_msg(LOG_WARNING, "client:serial:Inactive port detected %s", port->location);
                CloseHandle(port->implementation->handle_rename->win_handle);
                port->status = SERIAL_STATE_OPEN_ERROR;
                return GENERIC_FUNCTION_ERROR;
            }
            log_msg(LOG_DEBUG, "client:serial:Open: Serial openned as windows COM port");
        } else {
            log_msg(LOG_DEBUG, "client:serial:Open: Serial openned as windows regular file (named pipe)");
        }
    }
    #elif defined OS_POSIX
    {
        assert(0 <= port->baud_rate);
        port->implementation->handle_rename->posix_handle = open(port->location, O_RDWR | O_NOCTTY);
        if (port->implementation->handle_rename->posix_handle < 0) {
            perror(port->location);
            if ( errno == ENOENT ) {
                port->status = SERIAL_STATE_DISCONNECTED;
            } else if ( errno == EPERM ) {
                port->status = SERIAL_STATE_MISSING_PERM;
            } else {
                port->status = SERIAL_STATE_OPEN_ERROR;
            }
            return GENERIC_FUNCTION_ERROR;
        }

        tcgetattr(port->implementation->handle_rename->posix_handle, &(port->implementation->oldtio)); /* save current port settings */

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

        tcflush(port->implementation->handle_rename->posix_handle, TCIFLUSH);
        tcsetattr(port->implementation->handle_rename->posix_handle,TCSANOW,&(port->implementation->newtio));
    }
    #else
    #   warning Regular file openning not supported on this OS
    #endif

    port->status = SERIAL_STATE_READY;
    log_msg(LOG_DEBUG, "client:serial:Open: Serial openned");
    return GENERIC_FUNCTION_SUCCESS;
}
void serial_close(final Serial * port) {
    if ( port == null || port->status != SERIAL_STATE_READY ) {
        log_msg(LOG_INFO, "client:serial:Close: device not open");
        return;
    } 
    object_handle_t_close(port->implementation->handle_rename);
    port->status = SERIAL_STATE_NOT_OPEN;
}
char * serial_describe_communication_layer(final Serial * serial) {
    char * res;
    asprintf(&res,"Serial (%d bauds)", serial->baud_rate);
    return res;
}
static void clear_data(final Serial* serial) {
    buffer_recycle(serial->recv_buffer);
}
void serial_lock(final Serial * port) {
    pthread_mutex_lock(&port->implementation->lock_mutex);
}
void serial_unlock(final Serial * port) {
    pthread_mutex_unlock(&port->implementation->lock_mutex);
}

Serial * serial_new() {
    final Serial * port = (Serial *)malloc(sizeof(Serial));
    port->implementation = (SerialImplementation *)malloc(sizeof(SerialImplementation));
    port->type = AD_DEVICE_TYPE_SERIAL;
    serial_init(port);
    return port;
}
void serial_reset_to_default(final Serial* serial) {
    serial->echo = true;
    serial->eol = strdup("\r\n");
}

void serial_init(final Serial* serial) {
    serial_reset_to_default(serial);
    serial->location = null;
    serial->status = SERIAL_STATE_UNDEFINED;
    serial->recv_buffer = buffer_new();
    serial->timeout = SERIAL_DEFAULT_TIMEOUT;
    serial->timeout_seq = SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT;
    serial->detected = false;
    serial->open = AD_DEVICE_OPEN(serial_open);
    serial->close = AD_DEVICE_CLOSE(serial_close);
    serial->send = AD_DEVICE_SEND(serial_send);
    serial->recv = AD_DEVICE_RECV(serial_recv);
    serial->free = AD_DEVICE_FREE(serial_free);
    serial->describe_communication_layer = AD_DEVICE_DESCRIBE_COMMUNICATION_LAYER(serial_describe_communication_layer);
    serial->parse_data = null;
    serial->set_filter_by_address = null;
    serial->guess_response = AD_SERIAL_GUESS_RESPONSE(serial_guess_response);
    serial->lock = AD_DEVICE_LOCK(serial_lock);
    serial->unlock = AD_DEVICE_UNLOCK(serial_unlock);
    serial->clear_data = AD_DEVICE_CLEAR_DATA(clear_data);
    serial->baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    pthread_mutex_init(&serial->implementation->lock_mutex, NULL);
    serial->implementation->handle_rename = object_handle_t_new();
}

void serial_free(final Serial * port) {
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
            buffer_free(port->recv_buffer);
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

void serial_dump(final Serial * port) {
    const char * title = "Serial dump (name/baud_rate/status/eol):";
    char *result = null;
    if ( port == null ) {
        asprintf(&result, "%s NULL", title);
        module_debug(MODULE_SERIAL result);
    } else {
        asprintf(&result, "%s %s/%d/%d/%s", title, port->location, port->baud_rate, port->status, port->eol);
        module_debug(MODULE_SERIAL result);
    }
    free(result);
}

void serial_debug(final Serial * port) {
    if ( port == null ) {
        log_msg(LOG_DEBUG, "client:serial:Serial debug: NULL");
    } else {
        log_msg(LOG_DEBUG, "client:serial:Serial: {");
        log_msg(LOG_DEBUG, "client:serial:    Device: {");
        log_msg(LOG_DEBUG, "client:serial:        send: %p", port->send);
        log_msg(LOG_DEBUG, "client:serial:        recv: %p", port->recv);
        log_msg(LOG_DEBUG, "client:serial:        open: %p", port->open);
        log_msg(LOG_DEBUG, "client:serial:        close: %p", port->close);
        log_msg(LOG_DEBUG, "client:serial:        describe_communication_layer: %p", port->describe_communication_layer);
        log_msg(LOG_DEBUG, "client:serial:        parse_data: %p", port->parse_data);
        log_msg(LOG_DEBUG, "client:serial:        clear_data: %p", port->clear_data);
        log_msg(LOG_DEBUG, "client:serial:        lock: %p", port->lock);
        log_msg(LOG_DEBUG, "client:serial:        unlock: %p", port->unlock);
        log_msg(LOG_DEBUG, "client:serial:    }");
        log_msg(LOG_DEBUG, "client:serial:    echo: %s", port->echo ? "true" : "false");
        log_msg(LOG_DEBUG, "client:serial:    baud_rate: %d", port->baud_rate);
        log_msg(LOG_DEBUG, "client:serial:    status: %d", port->status);
        log_msg(LOG_DEBUG, "client:serial:    name: %s", port->location);
        log_msg(LOG_DEBUG, "client:serial:    eol: %s", port->eol);
        log_msg(LOG_DEBUG, "client:serial:    timeout: %d ms", port->timeout);
        log_msg(LOG_DEBUG, "client:serial:    timeout_seq: %d ms", port->timeout_seq);
        log_msg(LOG_DEBUG, "client:serial:    recv_buffer: %p", port->recv_buffer);
        log_msg(LOG_DEBUG, "client:serial:    detected: %s", port->detected ? "true" : "false");
        log_msg(LOG_DEBUG, "client:serial:    guess_response: %p", port->guess_response);
        log_msg(LOG_DEBUG, "client:serial:}");
    }
}

char * serial_status_to_string(final SerialStatus status) {
    switch(status) {
        case SERIAL_STATE_UNDEFINED: return strdup("undefined");
        case SERIAL_STATE_READY: return strdup("ready");
        case SERIAL_STATE_USER_IGNORED: return strdup("user ignored");        
        case SERIAL_STATE_OPEN_ERROR: return strdup("open error");
        case SERIAL_STATE_DISCONNECTED: return strdup("disconnected");
        case SERIAL_STATE_MISSING_PERM: return strdup("missing permissions");
        default: return strdup("unknown");
    }
}

char * serial_describe_status(final Serial * port) {
    assert(port != null);
    char *status = serial_status_to_string(port->status);
    char *result = null;
    asprintf(&result, "Serial status: %s", status);
    free(status);
    return result;
}

bool at_is_command(char * command) {
    return strncasecmp(command,"at",2) == 0;
}

char * at_command_va(char * at_command, va_list ap) {
        
    char * atCmd = null;
    asprintf(&atCmd,"at%s", at_command);
    
    char *res = null;
    if ( compat_vasprintf(&res, atCmd, ap) == -1 ) {
        log_msg(LOG_ERROR, "client:serial:Error with at command");
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
bool serial_send_at_command_internal(final Serial* serial, char *cmd, va_list ap) {
    
    char * cmdAt = at_command_va(cmd, ap);

    va_end(ap);

    if ( serial->send(AD_DEVICE(serial), cmdAt) == DEVICE_ERROR ) {
        return false;
    }
    free(cmdAt);
    return true;
}
bool serial_send_at_command(final Serial* serial, char *cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    return serial_send_at_command_internal(serial, cmd, ap);
}
bool serial_query_at_command(final Serial* serial, char *cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    if ( ! serial_send_at_command_internal(serial, cmd, ap) ) {
        return false;
    }

    buffer_recycle(serial->recv_buffer);
    bool result = serial->recv(AD_DEVICE(serial)) == SERIAL_RESPONSE_OK;
    
    return result;
}
bool ascii_is_control_char(char c) {
    return (c >= 0 && c <= 31) || (c == 127);
}

char *serial_at_reduce(char *str) {
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

int serial_at_parse_reduced(char *reduced, char *atcmd) {
    int idx = -1;
    char * cmdFull;
    asprintf(&cmdFull,"at%s",atcmd);
    if ( strcasebeginwith(reduced, cmdFull) ) {
        idx = strlen(cmdFull);
    }
    free(cmdFull);
    return idx;
}