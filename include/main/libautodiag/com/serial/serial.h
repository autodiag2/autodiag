/**
 * All this lib is designed to be used on a simple thread,
 * to avoid concurrency, you must lock the serial port before using it
 * and unlock it after usage.
 */
#ifndef __SERIAL_H
#define __SERIAL_H

#define _MODULE_SERIAL "Serial"
#define MODULE_SERIAL _MODULE_SERIAL,

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libautodiag/string.h"
#include <unistd.h>
#include <errno.h>
#include "libautodiag/com/device.h"
#include "libautodiag/lib.h"
#include <ctype.h>
#if defined OS_POSIX
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <unistd.h>
#   include <errno.h>
#endif
#if defined OS_WINDOWS
#   include "libautodiag/windows.h"
#elif defined OS_POSIX
#   include <termios.h>
#else
#   warning Unsupported OS
#endif

typedef enum {
    SERIAL_STATE_UNDEFINED, 
    SERIAL_STATE_READY, SERIAL_STATE_NOT_OPEN, SERIAL_STATE_USER_IGNORED,
    SERIAL_STATE_OPEN_ERROR,
    SERIAL_STATE_DISCONNECTED,
    SERIAL_STATE_MISSING_PERM
} SerialStatus;

// timeout values in ms
#define SERIAL_DEFAULT_TIMEOUT 1000
#define SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT 0
#define SERIAL_DEFAULT_BAUD_RATE 38400

// Implementation related fields, to avoid breaking bindings
typedef struct {
    pthread_mutex_t lock_mutex;    // thread lock on the port (both buffer and other data)
#if defined OS_WINDOWS
    HANDLE handle;
#elif defined OS_POSIX
    int handle;
    struct termios oldtio,newtio;
#else
#   warning Unsupported OS
#endif
} SerialImplementation;

typedef struct {
    Device;
    bool echo;                  // Not part of the serial standard but defined here for convenience
    int baud_rate;              // RS232 speed in bauds
    SerialStatus status;
    char *location;             // location of the device accross OSs
    char *eol;                  // Not part of the serial standard but defined here for convenience
    int timeout;                // timeout in ms before considering no reply from the remote
    int timeout_seq;            // timeout in ms for burst reception
    Buffer * recv_buffer;       // buffer for input data
    bool detected;              // did the serial port has been detected during the previous scan
    /**
     * Guess error responses and specicial response sent back by
     * the adaptater
     */
    int (*guess_response)(char *ptr);
    SerialImplementation* implementation;
} Serial;
#define CAST_SERIAL(var) ((Serial*)var)
#define CAST_SERIAL_GUESS_RESPONSE(var) ((int (*)(char* ptr))var)

/**
 * Responses defined by Hayes command set:
 *  https://en.wikipedia.org/wiki/Hayes_command_set
 *  /usr/share/doc/python3-serial/examples/at_protocol.py
 *  mm_serial_parser_v1_new from ModemManager
 */
#define SERIAL_RESPONSE_PROMPT                  0xF0
#define SERIAL_RESPONSE_OK                      0xF1
#define SERIAL_CONNECT                          0xF2
#define SERIAL_NO_CARRIER                       0xF3
#define SERIAL_CME_ERROR                        0xF4
#define SERIAL_ERROR                            0xF5
#define SERIAL_COMMAND_NOT_SUPPORT              0xF6
#define SERIAL_BUSY                             0xF7
#define SERIAL_NO_ANSWER                        0xF8
#define SERIAL_NO_DIALTONE                      0xF9
#define SERIAL_NA                               0xFA

static int SerialResponseOffset = 0xF0;
static int SerialResponseStrNumber = 11;
static char * SerialResponseStr[] = {
    ">", "OK",
    "CONNECT", "NO CARRIER", "+CME ERROR:",
    "ERROR", "COMMAND NOT SUPPORT",
    "BUSY", "NO ANSWER", "NO DIALTONE",
    "NA"
};
int serial_guess_response(final char * buffer);

/**
 * Iterate through buffer lines (eol separated) and call a handle defined as a macro that takes the current
 * position in the text in argument
 * handle: (ptr, end_ptr) where end_ptr is the excluded bound (exclude eol), ptr the start (end_ptr-ptr is the size)
 */
#define SERIAL_BUFFER_ITERATE(serial,handle) { \
    Buffer * recv_buffer = buffer_copy(serial->recv_buffer); \
    buffer_ensure_termination(recv_buffer); \
    char *ptr = (char*)recv_buffer->buffer; \
    final char* buffer_last_position = (char*)recv_buffer->buffer + recv_buffer->size; \
    while(ptr < buffer_last_position && *ptr != 0) { \
        char* end_ptr = strstr(ptr, serial->eol); \
        if ( end_ptr == null ) { \
            end_ptr = buffer_last_position; \
        } \
        if ( ptr == end_ptr ) { \
            log_msg(LOG_DEBUG, "Skip one empty line"); \
        } else { \
            handle(ptr,end_ptr) \
        } \
        ptr = end_ptr + strlen(serial->eol); \
    } \
    buffer_free(recv_buffer); \
}

void module_init_serial();
void module_shutdown_serial();
void serial_init(final Serial * serial);
/**
 * Open currently selected serial or reopen if it is already openned. 
 */
int serial_open(final Serial * port);
/**
 * Close currently openned serial.
 */
void serial_close(final Serial * port);
Serial * serial_new();

/**
 * Acquire and release exclusive lock over the serial port (no operation can be performed on the port while it is under lock)
 */
void serial_lock(final Serial * port);
void serial_unlock(final Serial * port);
/**
 * Debug information on a Serial
 */
void serial_dump(final Serial * port);
void serial_debug(final Serial * port);
void serial_free(final Serial * port);
/**
 * Get the selected Serial (port on which we are currently working) or NULL if no port currently selected.
 */
Serial * list_serial_get_selected();
void list_serial_set_selected_by_location(char *location);

/**
 * Timeout before considering remote has not alive (ms)
 */
#define SERIAL_AT_TIMEOUT        1000
/**
 * The standard version of serial sending some data, it just the raw bytes.
 * Send commands over the serial link at the specified speed (baud rate).
 * @return number of bytes sent or DEVICE_ERROR on error
 */
int serial_send_internal(final Serial * port, char * tx_buf, int bytes_to_send);
/**
 * A non standard wrapper.
 */
int serial_send(final Serial * port, const char *command);

#define GEN_SERIAL_RECV(sym,type,ITERATOR) int sym(final type* serial) { \
    if ( serial == null || serial->recv_buffer == null ) { \
        return DEVICE_ERROR; \
    } else { \
        int deduced_response = DEVICE_RECV_NULL; \
        bool continue_reception = true; \
        while(continue_reception) { \
            final int bytes_received = serial_recv_internal((Serial *)serial); \
            if ( bytes_received == DEVICE_ERROR ) { \
                return DEVICE_ERROR; \
            } else { \
                if ( bytes_received == 0 ) { \
                    return DEVICE_RECV_NULL; \
                } else { \
                    SERIAL_BUFFER_ITERATE(((Serial *)serial),ITERATOR) \
                } \
            } \
        } \
        return deduced_response; \
    } \
}

/**
 * serial_recv_internal with taking care of removing some eols, adding a null termination
 * and wait for the prompt char to be sent back back the serial terminal to which we connect to
 * @return SerialResponse(s)
 */
int serial_recv(final Serial * port);
/**
 * @return number of bytes received or DEVICE_ERROR on error
 */
int serial_recv_internal(final Serial * port);
/**
 * @return allocated with malloc
 */
char * serial_status_to_string(final SerialStatus status);
char * serial_describe_status(final Serial * port);

char * at_command(char * at_command, ...);
char * at_command_va(char * at_command, va_list ap);
char * at_command_boolean(char *cmd, final bool state);
bool at_is_command(char * command);
char * serial_describe_communication_layer(final Serial * serial);
bool serial_query_at_command(final Serial* serial, char *cmd, ...);
bool serial_send_at_command(final Serial* serial, char *cmd, ...);
/**
 * Default of ELM327 (non standard)
 */
void serial_reset_to_default(final Serial* serial);

/**
 * Scan the str for AT .*, at.*, AT.*, at .*
 */
char *serial_at_reduce(char *str);
int serial_at_parse_reduced(char *reduced, char *atcmd);
/**
 * If network location is an ip addresse + port
 */
bool serial_location_is_network(final Serial *port);

#endif
