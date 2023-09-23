#define _GNU_SOURCE
#include "com/serial/serial.h"
#include "com/serial/serial_list.h"
#include "log.h"
#include "ui/main.h"
#include "ui/options.h"
#include <errno.h>

bool serial_wait_until_prompt(final SERIAL port) {
    if ( port == null ) {
        return false;
    } else {
        do {
            buffer_recycle(port->recv_buffer);
            final int tm = port->timeout;
            port->timeout = 10;
            serial_recv(port);
            port->timeout = tm;
            if ( 0 < port->recv_buffer->size_used ) {
                final char * firstEOL = strstr(port->recv_buffer->buffer, port->eol);
                if ( firstEOL != null ) {
                    final char * secondEOL = strstr(firstEOL, port->eol);
                    if ( secondEOL == null ) {
                        if ( strstr(port->recv_buffer->buffer, ">") != null ) {
                            break;
                        }
                    }
                }
            }
        } while (0 < port->recv_buffer->size_used);
        buffer_recycle(port->recv_buffer);
        return true;
    }
}

void serial_strip_char(final Buffer * buffer, final char * char_to_strip) {
    serial_strip_char_internal(buffer,char_to_strip,true,true);
}

void serial_strip_char_internal(final Buffer * buffer, final char * char_to_strip, final bool start, final bool end) {
    int leading_to_remove = 0, trailing_to_remove = 0;
    int current = 0;
    int max = strlen(char_to_strip);
    if ( start ) {
        for(int i = 0; i < buffer->size_used; i++) {
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
        for(int i = buffer->size_used - 1; 0 <= i ; i--) {
            if ( buffer->buffer[i] == char_to_strip[current++] ) {
                if ( max == current ) {
                    current = 0;
                    buffer->size_used -= max;
                    buffer->buffer[buffer->size_used] = 0;     
                }
            } else {
                current = 0;
                break;
            }
        }
    }    
}
int serial_recv_internal(final SERIAL port) {
    if ( port == null || port->recv_buffer == null ) {
        return SERIAL_INTERNAL_ERROR;
    } else {
        bool manualRemove = 0 < port->recv_buffer->size_used && port->recv_buffer->buffer[port->recv_buffer->size_used-1] == 0;
        if ( manualRemove ) {
            port->recv_buffer->size_used --;
        }
        final int buffer_offset = port->recv_buffer->size_used;
        #ifdef POSIX_TERMIOS
            if (port->fdtty < 0) {
               port->status = SERIAL_STATE_NOT_OPEN;
               return SERIAL_INTERNAL_ERROR;
            } else {
                int res = 1;
                int block_sz = 64;
                final POLLFD fileDescriptor = {
                    .fd = port->fdtty,
                    .events = POLLIN
                };
                res = poll(&fileDescriptor,1,port->timeout);
                if ( 0 < res ) {
                    while( 0 < res && port->fdtty != -1 ) {
                        buffer_ensure_capacity(port->recv_buffer, block_sz);
                        final int bytes_readed = read(port->fdtty, port->recv_buffer->buffer + port->recv_buffer->size_used, block_sz);
                        if( 0 < bytes_readed ) {
                            port->recv_buffer->size_used += bytes_readed;
                        } else if ( bytes_readed == 0 ) {
                            break;
                        } else {
                            perror("read");
                        }
                        res = poll(&fileDescriptor,1,port->timeout_seq);
                    }
                } else if ( res == -1 ) {
                    perror("select");
                } else if ( res == 0 ) {
                    
                } else {
                    log_msg("unexpected happen on serial line", LOG_ERROR);
                }
                final int received_bytes_total = port->recv_buffer->size_used - buffer_offset;
                if ( 0 < received_bytes_total ) {
                    buffer_ensure_termination(port->recv_buffer);
                    serial_strip_char_internal(port->recv_buffer, port->eol,true,false);
                }
            }
        #else
            unsupported_configuration();
        #endif
        if ( manualRemove ) {
            buffer_ensure_termination(port->recv_buffer);
        }
        if ( log_has_level(LOG_DEBUG) ) {
            module_debug(MODULE_SERIAL "Serial data received");
            buffer_dump(port->recv_buffer);
        }
        return port->recv_buffer->size_used - buffer_offset;
    }
}
int serial_recv(final SERIAL port) {
    int offsetStart = port->recv_buffer->size_used;
    int bytes = 0;
    bool terminalReturnFound = false;
    for(int try = 0; try < port->recv_retry && !terminalReturnFound; try ++) {
        final int bts = serial_recv_internal(port);
        if ( bts == SERIAL_INTERNAL_ERROR ) {
            return SERIAL_INTERNAL_ERROR;
        } else {
            if ( 0 < bts ) {
                bytes += bts;
                char * ptrStart = port->recv_buffer->buffer + offsetStart;
                char * tmp_ptr;
                do {
                    if ( strncmp(ptrStart,">",1) == 0 ) {
                        terminalReturnFound = true;
                        break;
                    }
                    tmp_ptr = strstr(ptrStart,port->eol);
                    if ( tmp_ptr != null ) {
                        offsetStart += (tmp_ptr - ptrStart) + strlen(port->eol);
                        ptrStart = tmp_ptr + strlen(port->eol);
                    }
                } while (tmp_ptr != null);
            }
        }
    }
    return terminalReturnFound ? bytes : SERIAL_INTERNAL_ERROR;
}
int serial_send(final Serial * port, const char *command) {
    if ( port == null || command == null ) {
        return SERIAL_INTERNAL_ERROR;
    } else {
        final int useless_termination = 1;
        final int bytes_to_send = strlen(command) + strlen(port->eol);
        char tx_buf[bytes_to_send + useless_termination];
        sprintf(tx_buf, "%s%s", command, port->eol);
        int bytes_sent = 0;

        #ifdef POSIX_TERMIOS
            if ( log_has_level(LOG_DEBUG) ) {
                module_debug(MODULE_SERIAL "Sending");
                bin_dump(tx_buf,bytes_to_send);
            }
            bytes_sent = write(port->fdtty,tx_buf,bytes_to_send);
	        if ( bytes_sent != bytes_to_send ) {
                perror(port->name);
                log_msg("Error while writting to the serial", LOG_ERROR);
                serial_close(port);
                return SERIAL_INTERNAL_ERROR;
	        } else {
                tcflush(port->fdtty, TCIFLUSH);
	        }
        #else
            unsupported_configuration();
        #endif
        return bytes_sent;
    }
}

SERIAL serial_new() {
    final SERIAL port = (SERIAL)malloc(sizeof(Serial));
    // ELM327 is echo on by default
    port->echo = true;
    port->linefeeds = true;
    port->name = null;
    port->number = 0;
    port->baud_rate = 0;
    port->status = SERIAL_STATE_UNDEFINED;
    port->eol = strdup(SERIAL_DEFAULT_EOL);
    port->recv_buffer = buffer_new();
    port->timeout = SERIAL_DEFAULT_TIMEOUT;
    port->timeout_seq = SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT;
    port->recv_retry = OBDIFACE_RECV_RETRY;
    port->send_retry = OBDIFACE_SEND_RETRY;
    pthread_mutex_init(&port->lock, NULL);
    return port;
}

void serial_lock(final nonnull SERIAL port) {
    pthread_mutex_lock(&port->lock);
}

void serial_unlock(final nonnull SERIAL port) {
    pthread_mutex_unlock(&port->lock);
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
    if ( port != NULL ) {
        if (port->name != NULL ) {
            free(port->name);
            port->name = NULL;
        }
        if ( port->eol != null ) {
            free(port->eol);
            port->eol = null;
        }
        if ( port->recv_buffer != null ) {
            buffer_free(port->recv_buffer);
            port->recv_buffer = null;
        }
    }
}

void serial_dump(final Serial * port) {
    const char * title = "Serial dump (number/baud_rate/status/name/eol):";
    char *result = null;
    if ( port == null ) {
        asprintf(&result, "%s NULL", title);
        module_debug(MODULE_SERIAL result);
    } else {
        asprintf(&result, "%s %d/%d/%d/%s/%s", title, port->number, port->baud_rate, port->status,port->name,port->eol);
        module_debug(MODULE_SERIAL result);
    }
    free(result);
}

void serial_close(final Serial * port) {
    if ( port == null ) {
        log_msg("Open: No serial currently selected", LOG_INFO);
    } else {
        if (port->status == SERIAL_STATE_READY) {
            #ifdef POSIX_TERMIOS
                  tcsetattr(port->fdtty,TCSANOW,&(port->oldtio));
                  close(port->fdtty);
                  port->fdtty = -1;
            #else
                  unsupported_configuration();
            #endif
        }
        port->status = SERIAL_STATE_NOT_OPEN;
    }
}

int serial_open(final Serial * port) {
    if ( port == null ) {
        log_msg("Open: Cannot open since not serial port info given", LOG_INFO);
        return GENERIC_FUNCTION_ERROR;
    } else {
        serial_close(port);

        // Do not open serial if it has not been configured.
        if (port->number < 0) {
            port->status = SERIAL_STATE_OPEN_ERROR;
            return GENERIC_FUNCTION_ERROR;
        }

        #ifdef POSIX_TERMIOS
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

            if ( port->baud_rate != BAUD_RATE_UNDEFINED ) {
                cfsetspeed(&(port->newtio), port->baud_rate);
            }

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
            unsupported_configuration();
        #endif

        port->status = SERIAL_STATE_READY;

        module_debug(MODULE_SERIAL "Open: Serial openned");

        return GENERIC_FUNCTION_SUCCESS;
    }
}

void serial_set_name(final Serial * port, final char *name) {
    if ( port->name != NULL ) {
        free(port->name);
    }
    port->name = (char *)malloc(sizeof(char) * (1 + strlen(name)));
    strcpy(port->name, name);
}

char * serial_baud_rate_code_to_text(final int baud_rate) {
    switch(baud_rate) {
        case BAUD_RATE_9600: return strdup("9600");
        case BAUD_RATE_38400: return strdup("38400");
        case BAUD_RATE_115200: return strdup("115200");
        case BAUD_RATE_230400: return strdup("230400");
        default: return NULL;
    }
}

int serial_baud_rate_text_to_code(final char *code) {
    if ( code == null ) {
        return BAUD_RATE_UNDEFINED;
    } else {
      switch(atoi(code)) {
            case 9600: return BAUD_RATE_9600;
            case 38400: return BAUD_RATE_38400;
            case 115200: return BAUD_RATE_115200;
            case 230400: return BAUD_RATE_230400;
            default: return BAUD_RATE_UNDEFINED;
        }
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
