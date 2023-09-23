#ifndef __SERIAL_H
#define __SERIAL_H

#define _MODULE_SERIAL "Serial"
#define MODULE_SERIAL _MODULE_SERIAL,

#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <pthread.h>
#include "custom/poll.h"
#include "buffer.h"

#define BAUD_RATE_UNDEFINED  0
#ifdef POSIX_TERMIOS
   #include <termios.h>
   #define BAUD_RATE_9600    B9600
   #define BAUD_RATE_38400   B38400
   #define BAUD_RATE_115200  B115200
   #define BAUD_RATE_230400  B230400
#endif

typedef enum {
    SERIAL_STATE_UNDEFINED, 
    SERIAL_STATE_READY, SERIAL_STATE_NOT_OPEN, SERIAL_STATE_USER_IGNORED,
    SERIAL_STATE_OPEN_ERROR,
    SERIAL_STATE_DISCONNECTED,
    SERIAL_STATE_MISSING_PERM
} SerialStatus;

// serial eol
#define SERIAL_DEFAULT_EOL  "\r"
#define SERIAL_DEFAULT_TIMEOUT 5000
#define SERIAL_DEFAULT_SEQUENCIAL_TIMEOUT 0

typedef struct {
    bool echo;
    bool linefeeds;
    int number;
    int baud_rate;
    SerialStatus status;
    char *name;
    char *eol;
    int timeout;             // timeout in ms before considering no reply from the remote
    int timeout_seq;         // timeout in ms for burst reception
    BUFFER recv_buffer;      // buffer for input data
    pthread_mutex_t lock;    // thread lock on the port (both buffer and other data)
    /**
     * OBDSim have a glitch that make the first reception wrong so even there is 5s timeout for the reception, <br />
     * still we receive empty response, only solution is to make many tries
     */
    int recv_retry;
    int send_retry;
#ifdef POSIX_TERMIOS
    int fdtty;
    struct termios oldtio,newtio;
#endif
} Serial;

#define OBDIFACE_RECV_RETRY 3
#define OBDIFACE_SEND_RETRY 2

void module_init_serial();
void module_shutdown_serial();
/**
 * Open currently selected serial or reopen if it is already openned. 
 */
int serial_open(final Serial *port);
/**
 * Close currently openned serial.
 */
void serial_close(final Serial *port);
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
void serial_free(final Serial * port);
/**
 * Get the selected Serial (port on which we are currently working) or NULL if no port currently selected.
 */
Serial * serial_list_get_selected();
void serial_list_set_selected_by_name(char *name);

/**
 * NULL allowed (return baud rate 0)
 */
int serial_baud_rate_text_to_code(final char *code);
/**
 * return value is allocated with malloc and should be free
 */
char * serial_baud_rate_code_to_text(final int baud_rate);
/**
 * Timeout before considering remote has not alive (ms)
 */
#define SERIAL_AT_TIMEOUT        1000
/**
 * Send commands over the serial link at the specified speed (baud rate).
 * @return number of bytes sent or SERIAL_INTERNAL_ERROR on error
 */
int serial_send(final Serial * port, const char *command);
/**
 * Simple wrapper around read() call<br />
 * To confirm that there is no more data to receive, we wait for the prompt send back by adaptater.<br />
 * @return number of bytes received or SERIAL_INTERNAL_ERROR on error
 */
int serial_recv(final Serial * port);
#define SERIAL_INTERNAL_ERROR       -1
/**
 * Remove eol from start/end of the buffer
 */
void serial_strip_char(final Buffer * buffer, final char * char_to_strip);
void serial_strip_char_internal(final Buffer * buffer, final char * char_to_strip, final bool start, final bool end);
/**
 * @return allocated with malloc
 */
char * serial_status_to_string(final SerialStatus status);
/**
 * Wait until remote terminal respond with prompt
 */
bool serial_wait_until_prompt(final Serial * port);
/**
 * Easiers
 */
#define SERIAL Serial *

#endif
