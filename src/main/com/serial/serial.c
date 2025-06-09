#include "com/serial/serial.h"
#include "com/serial/serial_list.h"

int serial_guess_response(final char * buffer) {
    for(int i = 0; i < SerialResponseStrNumber; i++) {
        assert(SerialResponseStr[i] != null);
        if ( strncmp(buffer, SerialResponseStr[i], strlen(SerialResponseStr[i])) == 0 ) {
            return SerialResponseOffset + i;
        }
    }
    return DEVICE_RECV_DATA;
}
int serial_send(final Serial * port, const char *command) {
    if ( port == null || command == null ) {
        return DEVICE_ERROR;
    } else {
        final int useless_termination = 1;
        final int bytes_to_send = strlen(command) + strlen(port->eol);
        char tx_buf[bytes_to_send + useless_termination];
        sprintf(tx_buf, "%s%s", command, port->eol);
        int bytes_sent = 0;

        if ( log_has_level(LOG_DEBUG) ) {
            module_debug(MODULE_SERIAL "Sending");
            bytes_dump(tx_buf,bytes_to_send);
        }
        #if defined OS_WINDOWS
            DWORD bytes_written;

            PurgeComm(port->connexion_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
            if (!WriteFile(port->connexion_handle, tx_buf, bytes_to_send, &bytes_written, null)) {
                log_msg(LOG_ERROR, "WriteFile failed with error %lu", GetLastError());
                serial_close(port);
                return DEVICE_ERROR;
            }            
            bytes_sent = (int) bytes_written;
            if (bytes_sent != bytes_to_send) {
                log_msg(LOG_ERROR, "Error while writting to the serial");
                serial_close(port);
                return DEVICE_ERROR;
            }
        #elif defined OS_POSIX
            bytes_sent = write(port->fdtty,tx_buf,bytes_to_send);
	        if ( bytes_sent != bytes_to_send ) {
                perror(port->name);
                log_msg(LOG_ERROR, "Error while writting to the serial");
                serial_close(port);
                return DEVICE_ERROR;
	        } else {
                tcflush(port->fdtty, TCIFLUSH);
	        }
        
        #else
        #   warning Unsupported OS
        #endif
        return bytes_sent;
    }
}
int serial_recv_internal(final SERIAL port) {
    if ( port == null || port->recv_buffer == null ) {
        return DEVICE_ERROR;
    } else {
        final int initial_buffer_sz = port->recv_buffer->size;
        #if defined OS_WINDOWS
            if (port->connexion_handle == INVALID_HANDLE_VALUE) {
               port->status = SERIAL_STATE_NOT_OPEN;
               return DEVICE_ERROR;
            } else {        
                DWORD bytes_readed = 0;

                int readLen = 0;
                file_pool(&port->connexion_handle, &readLen, port->timeout);
                
                while(0 < readLen) {
                    buffer_ensure_capacity(port->recv_buffer, readLen);
                    if ( ReadFile(port->connexion_handle, port->recv_buffer->buffer + port->recv_buffer->size, readLen, &bytes_readed, 0) ) {
                        if( readLen == bytes_readed ) {
                            port->recv_buffer->size += bytes_readed;
                            if ( log_has_level(LOG_DEBUG) ) {
                                module_debug(MODULE_SERIAL "Serial data received");
                                buffer_dump(port->recv_buffer);
                            }
                        } else {
                            log_msg(LOG_ERROR, "ReadFile error");
                        }
                    } else {
                        log_msg(LOG_ERROR, "ReadFile error 2");
                    }
                    file_pool(&port->connexion_handle, &readLen, port->timeout_seq);
                }
            }
        #elif defined OS_POSIX
            if (port->fdtty < 0) {
               port->status = SERIAL_STATE_NOT_OPEN;
               return DEVICE_ERROR;
            } else {
                int res = 1;
                int block_sz = 64;
                res = file_pool(&port->fdtty, null, port->timeout);
                if ( 0 < res ) {
                    while( 0 < res && port->fdtty != -1 ) {
                        buffer_ensure_capacity(port->recv_buffer, block_sz);
                        final int bytes_readed = read(port->fdtty, port->recv_buffer->buffer + port->recv_buffer->size, block_sz);
                        if( 0 < bytes_readed ) {
                            port->recv_buffer->size += bytes_readed;
                        } else if ( bytes_readed == 0 ) {
                            break;
                        } else {
                            perror("read");
                            break;
                        }
                        res = file_pool(&port->fdtty, null, port->timeout_seq);
                    }
                    if ( log_has_level(LOG_DEBUG) ) {
                        module_debug(MODULE_SERIAL "Serial data received");
                        buffer_dump(port->recv_buffer);
                    }
                } else if ( res == -1 ) {
                    perror("poll");
                } else if ( res == 0 ) {
                    log_msg(LOG_DEBUG, "Timeout while reading data");
                } else {
                    log_msg(LOG_ERROR, "unexpected happen on serial line");
                }
            }
        #else
        #   warning Unsupported OS
        #endif
        
        return port->recv_buffer->size - initial_buffer_sz;
    }
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
        log_msg(LOG_INFO, "Open: Cannot open since not serial port info given");
        return GENERIC_FUNCTION_ERROR;
    } else {
        serial_close(port);

        // Do not open serial if it has not been configured.
        if (port->name == null) {
            log_msg(LOG_DEBUG, "Open: Cannot open since no name on the serial");
            return GENERIC_FUNCTION_ERROR;
        }

        assert(0 <= port->baud_rate);

        #if defined OS_WINDOWS

            #define TX_TIMEOUT_MULTIPLIER    0
            #define TX_TIMEOUT_CONSTANT      1000

            DCB dcb;
            COMMTIMEOUTS timeouts;
            DWORD bytes_written;
            port->connexion_handle = CreateFile(port->name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
            if (port->connexion_handle == INVALID_HANDLE_VALUE) {
                log_msg(LOG_WARNING, "Cannot open the port %s", port->name);
                port->status = SERIAL_STATE_OPEN_ERROR;
                return GENERIC_FUNCTION_ERROR;
            } else {
                log_msg(LOG_DEBUG, "Openning port: %s", port->name);
            }
            
            if ( isComport(port->connexion_handle) ) {
                ZeroMemory(&dcb, sizeof(DCB));
                dcb.DCBlength = sizeof(DCB);
                if (!GetCommState(port->connexion_handle, &dcb)) {
                    log_msg(LOG_ERROR, "GetCommState failed for %s", port->name);
                    CloseHandle(port->connexion_handle);
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
                if (!SetCommState(port->connexion_handle, &dcb)) {
                    log_msg(LOG_ERROR, "SetCommState failed for %s", port->name);
                    CloseHandle(port->connexion_handle);
                    return GENERIC_FUNCTION_ERROR;
                }

                ZeroMemory(&timeouts, sizeof(COMMTIMEOUTS));
                timeouts.ReadIntervalTimeout = port->timeout;
                timeouts.ReadTotalTimeoutMultiplier = 0;
                timeouts.ReadTotalTimeoutConstant = port->timeout;
                timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER;
                timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT;
                if (!SetCommTimeouts(port->connexion_handle, &timeouts)) {
                    log_msg(LOG_ERROR, "SetCommTimeouts failed for %s", port->name);
                    CloseHandle(port->connexion_handle);
                    return GENERIC_FUNCTION_ERROR;
                }

                // Hack to get around Windows 2000 multiplying timeout values by 15
                GetCommTimeouts(port->connexion_handle, &timeouts);
                if (TX_TIMEOUT_MULTIPLIER > 0) {
                    timeouts.WriteTotalTimeoutMultiplier = TX_TIMEOUT_MULTIPLIER * TX_TIMEOUT_MULTIPLIER / timeouts.WriteTotalTimeoutMultiplier;
                }
                if (TX_TIMEOUT_CONSTANT > 0) {
                    timeouts.WriteTotalTimeoutConstant = TX_TIMEOUT_CONSTANT * TX_TIMEOUT_CONSTANT / timeouts.WriteTotalTimeoutConstant;
                }
                SetCommTimeouts(port->connexion_handle, &timeouts);

                // If the port is Bluetooth, make sure device is active
                PurgeComm(port->connexion_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
                WriteFile(port->connexion_handle, "?\r", 2, &bytes_written, 0);
                PurgeComm(port->connexion_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
                if (bytes_written != 2) { // If Tx timeout occured
                    log_msg(LOG_WARNING, "Inactive port detected %s", port->name);
                    CloseHandle(port->connexion_handle);
                    port->status = SERIAL_STATE_OPEN_ERROR;
                    return GENERIC_FUNCTION_ERROR;
                }
            }

        #elif defined OS_POSIX
            port->fdtty = open(port->name, O_RDWR | O_NOCTTY);
            if (port->fdtty < 0) {
                perror(port->name);
                if ( errno == ENOENT ) {
                    port->status = SERIAL_STATE_DISCONNECTED;
                } else if ( errno == EPERM ) {
                    port->status = SERIAL_STATE_MISSING_PERM;
                } else {
                    port->status = SERIAL_STATE_OPEN_ERROR;
                }
                return GENERIC_FUNCTION_ERROR;
            }

            tcgetattr(port->fdtty, &(port->oldtio)); /* save current port settings */

            bzero(&(port->newtio), sizeof(port->newtio));

            cfsetspeed(&(port->newtio), port->baud_rate);

            cfmakeraw(&(port->newtio));
            port->newtio.c_cflag |= (CLOCAL | CREAD);

            // No parity (8N1):
            port->newtio.c_cflag &= ~PARENB;
            port->newtio.c_cflag &= ~CSTOPB;
            port->newtio.c_cflag &= ~CSIZE;
            port->newtio.c_cflag |= CS8;

            // disable hardware flow control
            port->newtio.c_cflag &= ~CRTSCTS ;

            port->newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
            port->newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

            tcflush(port->fdtty, TCIFLUSH);
            tcsetattr(port->fdtty,TCSANOW,&(port->newtio));
        #else
        #   warning Unsupported OS            
        #endif

        port->status = SERIAL_STATE_READY;

        module_debug(MODULE_SERIAL "Open: Serial openned");

        return GENERIC_FUNCTION_SUCCESS;
    }
}
void serial_close(final Serial * port) {
    if ( port == null ) {
        log_msg(LOG_INFO, "Open: No serial currently selected");
    } else {
        if (port->status == SERIAL_STATE_READY) {
            #if defined OS_WINDOWS
                PurgeComm(port->connexion_handle, PURGE_TXCLEAR|PURGE_RXCLEAR);
                CloseHandle(port->connexion_handle);
                port->connexion_handle = INVALID_HANDLE_VALUE;
            #elif defined OS_POSIX
                tcsetattr(port->fdtty,TCSANOW,&(port->oldtio));
                close(port->fdtty);
                port->fdtty = -1;
            #else
            #   warning Unsupported OS
            #endif
        }
        port->status = SERIAL_STATE_NOT_OPEN;
    }
}
char * serial_describe_communication_layer(final SERIAL serial) {
    char * res;
    asprintf(&res,"Serial (%d bauds)", serial->baud_rate);
    return res;
}
void serial_clear_data(final Serial* serial) {
    buffer_recycle(serial->recv_buffer);
}
void serial_lock(final nonnull SERIAL port) {
    pthread_mutex_lock(&port->lock_mutex);
}
void serial_unlock(final nonnull SERIAL port) {
    pthread_mutex_unlock(&port->lock_mutex);
}

void serial_strip_char(final Buffer * buffer, final char * char_to_strip) {
    serial_strip_char_internal(buffer,char_to_strip,true,true);
}

void serial_strip_char_internal(final Buffer * buffer, final char * char_to_strip, final bool start, final bool end) {
    int leading_to_remove = 0, trailing_to_remove = 0;
    int current = 0;
    int max = strlen(char_to_strip);
    if ( start ) {
        for(int i = 0; i < buffer->size; i++) {
            if ( buffer->buffer[i] == char_to_strip[current++] ) {
                if ( max == current ) {
                    current = 0;
                    leading_to_remove += max;
                }
            } else {
                current = 0;
                break;
            }
        }
        buffer_left_shift(buffer, leading_to_remove);
    }
    if ( end ) {
        for(int i = buffer->size - 1; 0 <= i ; i--) {
            if ( buffer->buffer[i] == char_to_strip[current++] ) {
                if ( max == current ) {
                    current = 0;
                    buffer->size -= max;
                    buffer->buffer[buffer->size] = 0;     
                }
            } else {
                current = 0;
                break;
            }
        }
    }    
}
SERIAL serial_new() {
    final SERIAL port = (SERIAL)malloc(sizeof(Serial));
    serial_init(port);
    return port;
}
void serial_reset_to_default(final Serial* serial) {
    serial->echo = true;
    serial->eol = strdup("\r\n");
}

void serial_init(final Serial* serial) {
    serial_reset_to_default(serial);
    serial->name = null;
    serial->status = SERIAL_STATE_UNDEFINED;
    serial->recv_buffer = buffer_new();
    serial->timeout = SERIAL_DEFAULT_TIMEOUT;
    serial->timeout_seq = SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT;
    serial->detected = false;
    serial->open = (void (*)(final _Device* ))serial_open;
    serial->close = (void (*)(final _Device* ))serial_close;
    serial->send = (int (*)(_Device *, const char *))serial_send;
    serial->recv = (int (*)(_Device *))serial_recv;
    serial->describe_communication_layer = (char* (*)(_Device*))serial_describe_communication_layer;
    serial->parse_data = null;
    serial->guess_response = serial_guess_response;
    serial->lock = (void(*)(_Device*))serial_lock;
    serial->unlock = (void(*)(_Device*))serial_unlock;
    serial->clear_data = (void (*)(final _Device* device))serial_clear_data;
    serial->baud_rate = SERIAL_DEFAULT_BAUD_RATE;
    pthread_mutex_init(&serial->lock_mutex, NULL);
    #if defined OS_WINDOWS
        serial->connexion_handle = INVALID_HANDLE_VALUE;
    #elif defined OS_POSIX
        serial->fdtty = -1;
    #else
    #   warning Unsupported OS
    #endif
}

void module_init_serial() {
    module_debug_init(_MODULE_SERIAL);
    serial_list_fill();
}

void module_shutdown_serial() {
   serial_close_selected();
   serial_list_free();
}

void serial_free(final Serial * port) {
    if ( port != null ) {
        if (port->name != null ) {
            free(port->name);
            port->name = null;
        }
        if ( port->eol != null ) {
            free(port->eol);
            port->eol = null;
        }
        if ( port->recv_buffer != null ) {
            buffer_free(port->recv_buffer);
            port->recv_buffer = null;
        }
        free(port);
        port = null;
    }
}

void serial_dump(final Serial * port) {
    const char * title = "Serial dump (number/baud_rate/status/name/eol):";
    char *result = null;
    if ( port == null ) {
        asprintf(&result, "%s NULL", title);
        module_debug(MODULE_SERIAL result);
    } else {
        asprintf(&result, "%s %d/%d/%s/%s", title, port->baud_rate, port->status,port->name,port->eol);
        module_debug(MODULE_SERIAL result);
    }
    free(result);
}

void serial_set_name(final Serial * port, final char *name) {
    if ( port->name != NULL ) {
        free(port->name);
    }
    port->name = (char *)malloc(sizeof(char) * (1 + strlen(name)));
    strcpy(port->name, name);
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

bool at_is_command(char * command) {
    return strncmp(command,"at",2) == 0;
}

char * at_command_va(char * at_command, va_list ap) {
        
    char * atCmd = null;
    asprintf(&atCmd,"at%s", at_command);
    
    char *res = null;
    if ( vasprintf(&res, atCmd, ap) == -1 ) {
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
bool serial_send_at_command_internal(final Serial* serial, char *cmd, va_list ap) {
    
    char * cmdAt = at_command_va(cmd, ap);

    va_end(ap);

    if ( serial->send(DEVICE(serial), cmdAt) == DEVICE_ERROR ) {
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
    bool result = serial->recv(DEVICE(serial)) == SERIAL_RESPONSE_OK;
    
    return result;
}
bool serial_at_command_search(char *str, char *atcmd) {
    return serial_at_index_end(str,atcmd) != -1;
}
int serial_at_index_end(char *str, char *atcmd) {
    int idx = -1;
    char * cmdFull;
    asprintf(&cmdFull,"at%s",atcmd);
    if ( strcasebeginwith(str,cmdFull) ) {
        idx = strlen(cmdFull);
    } else {
        free(cmdFull);    
        asprintf(&cmdFull,"at %s",atcmd);
        if ( strcasebeginwith(str,cmdFull) ) {
            idx = strlen(cmdFull);
        }
    }
    free(cmdFull);
    return idx;
}

