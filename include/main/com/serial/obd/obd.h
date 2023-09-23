#ifndef __SERIAL_OBD_H
#define __SERIAL_OBD_H

#define MODULE_OBD "OBD II",

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "log.h"
#include "globals.h"
#include "com/serial/serial.h"
#include "com/serial/serial_list.h"
#include "buffer.h"

#define OBD_PROTOCOLE_TYPE int

typedef enum {
    OBD_INTERFACE_UNKNOWN,OBD_INTERFACE_ELM320,OBD_INTERFACE_ELM322,
    OBD_INTERFACE_ELM323,OBD_INTERFACE_ELM327,OBD_INTERFACE_OBDLINK,
    OBD_INTERFACE_SCANTOOL,OBD_INTERFACE_GENERIC
} OBD_INTERFACE_TYPE;

typedef struct {
    Serial;
    /**
     * Physical interface type
     */
    OBD_INTERFACE_TYPE type;
    /**
     * Protocol used to communicate on that interface, some interfaces could communicate with multiple protocols.<br />
     * This data represent only the protocol currently in use.<br />
     * e.g. ISO 15765-4 CAN (11-bit ID, 500 kBit/s)<br />
     */
    OBD_PROTOCOLE_TYPE protocol;
    /**
     * OBD response data buffer
     */
    BUFFERLIST obd_data_buffer;
    /**
     * Did responses print space between bytes
     */
    bool printing_of_spaces;
    /**
     * Did responses print headers
     */
    bool printing_of_headers;
} OBDIFace;
/**
 * Also referred as modes previously
 */
typedef enum {
    OBD_SERVICE_NONE,OBD_SERVICE_SHOW_CURRENT_DATA,
    OBD_SERVICE_SHOW_FREEEZE_FRAME_DATA,OBD_SERVICE_SHOW_DTC,
    OBD_SERVICE_CLEAR_DTC,OBD_SERVICE_TESTS_RESULTS,
    OBD_SERVICE_TESTS_RESULTS_OTHER,
    OBD_SERVICE_PENDING_DTC,OBD_SERVICE_CONTROL_OPERATION,
    OBD_SERVICE_REQUEST_VEHICLE_INFORMATION,
    OBD_SERVICE_PERMANENT_DTC
} OBDServiceId;

bool buffer_filter_expected_databytes_start(final BUFFER buffer, final OBDIFace* iface, void *arg);

#include "com/serial/at/general.h"
#include "com/serial/at/obd/obd.h"

// OBD protocol number
#define OBD_PROTO_NONE                      0
#define OBD_PROTO_SAE_J1850_1               1
#define OBD_PROTO_SAE_J1850_2               2
#define OBD_PROTO_ISO_9141_2                3
#define OBD_PROTO_ISO_14230_4_KWP2000_1     4
#define OBD_PROTO_ISO_14230_4_KWP2000_2     5
#define OBD_PROTO_ISO_15765_4_CAN_1         6
#define OBD_PROTO_ISO_15765_4_CAN_2         7
#define OBD_PROTO_ISO_15765_4_CAN_3         8
#define OBD_PROTO_ISO_15765_4_CAN_4         9
// Heavy trucks and cars in USA
#define OBD_PROTO_SAEJ1939                  0xA
#define OBD_PROTO_USER1_CAN                 0xB
#define OBD_PROTO_USER2_CAN                 0xC
#define OBD_PROTO_DESC_UNKNOWN "Unknown"
#define OBD_PROTO_DESC_NONE "N/A"
#define OBD_PROTO_DESC_SAE_J1850_1 "SAE J1850 PWM (41.6 kBit/s)"
#define OBD_PROTO_DESC_SAE_J1850_2 "SAE J1850 VPW (10.4 kBit/s)"
#define OBD_PROTO_DESC_ISO_14230_4_KWP2000_1 "ISO 14230-4 KWP2000 (5-baud init)"
#define OBD_PROTO_DESC_ISO_14230_4_KWP2000_2 "ISO 14230-4 KWP2000 (fast init)"
#define OBD_PROTO_DESC_ISO_15765_4_CAN_1    "ISO 15765-4 (CAN 11-bit ID, 500 kBit/s)"
#define OBD_PROTO_DESC_ISO_15765_4_CAN_2    "ISO 15765-4 (CAN 29-bit ID, 500 kBit/s)"
#define OBD_PROTO_DESC_ISO_15765_4_CAN_3    "ISO 15765-4 (CAN 11-bit ID, 250 kBit/s)"
#define OBD_PROTO_DESC_ISO_15765_4_CAN_4    "ISO 15765-4 (CAN 29-bit ID, 250 kBit/s)"
#define OBD_PROTO_DESC_SAEJ1939             "SAE J1939 (CAN 29-bit ID, 250 kBit/s)"
#define OBD_PROTO_DESC_USER1_CAN            "USER1 CAN"
#define OBD_PROTO_DESC_USER2_CAN            "USER2 CAN"
#define OBD_PROTO_DESC_ISO_9141_2 "ISO 9141-2"
#define OBD_PROTO_DESC_HACK_ELM323 "ISO 9141-2 / ISO 14230-4 (KWP2000)"

/**
 * OBD new port
 */
OBDIFace* obd_new();
OBDIFace* obd_new_from_serial(final nonnull SERIAL serial);
/**
 * Send an OBD query
 */
int obd_send(final OBDIFace * iface, const char *command);
/**
 * OBD send optimized for sending over serial link
 */
int obd_serial_send_with_one_line_expected(final OBDIFace * iface, const char *command);
/**
 * @param oneLineOptimized this enable an optimization (eg for sensors data acquisition)
 * @param arg enable to send parameter to the handler
 */
void* obd_request(final OBDIFace * iface, void * (*responseHandler)(final OBDIFace * iface, final int response, void *arg), void *arg, char *obd_request, bool oneLineOptimized);
/**
 * From obd point of view we do not care what ECU has sent the message we only focus on the message "the car has sent back theses errors".
 * @return number of data buffer received or OBD_RECV_ERROR on error (no obd data available, serial error or any error more deep in the stack)
 */
int obd_recv(final OBDIFace* iface, final bool (*filter)(final Buffer* buffer, final OBDIFace* iface, final void * arg),void *arg);
#define OBD_RECV_ERROR -1
/**
 * Get connectivity specification
 */
const char *obd_get_protocol_string(final OBD_INTERFACE_TYPE interface_type, final OBD_PROTOCOLE_TYPE protocol_id);
void module_init_obd();
/**
 * Open OBD iface: guess what type of interface is below, how to communicate.<br />
 * Serial given as parameter will be free()
 */
OBDIFace * obd_open_from_serial(final Serial * serial);
void obd_dump(final OBDIFace * iface);
/**
 * Close an openned obdiface
 */
void obd_close(final OBDIFace *iface);
void obd_free(final OBDIFace *iface);
/**
 * Do not free or close serial part of the interface
 */
void obd_free_only_obd_part(final OBDIFace* iface);

void obd_lock(final OBDIFace *iface);
void obd_unlock(final OBDIFace *iface);
/**
 * Clear reception buffers
 */
void obd_clear_data(final OBDIFace * iface);

/**
 * Perform operation such as lock release on obd interface on thread cancel event
 */
static void obd_thread_cleanup_routine(void *arg) {
    final OBDIFace* iface = (OBDIFace*)serial_list_get_selected();
    log_msg("clean up execution", LOG_INFO);
    if ( iface != null ) {
        obd_unlock(iface);
    }
}


/**
 * OBDIFace is mandatory because sometimes printing of spaces is disabled.<br />
 * Unless you known what you are doing, ascii data must came from the same obd interface.<br />
 */
BUFFER obd_serial_ascii_to_bin(final OBDIFace* iface, final BUFFER ascii);
BUFFER obd_serial_ascii_to_bin_str(final OBDIFace* iface, final char * ascii, final char * end_ptr);
void* obd_request_one_line(final OBDIFace* iface, void * (*responseHandler)(final OBDIFace* iface, final int response, void *arg), char *obd_request);
int obd_request_one_line_int(final OBDIFace* iface, void * (*responseHandler)(final OBDIFace* iface, final int response, void *arg), char *obd_request);
bool obd_request_one_line_bool(final OBDIFace* iface, void * (*responseHandler)(final OBDIFace* iface, final int response, void *arg), char *obd_request);
double obd_request_one_line_double(final OBDIFace* iface, void * (*responseHandler)(final OBDIFace* iface, final int response, void *arg), char *obd_request);
/**
 * Depending on wich obd query has been sent, we can filter by which bytes are at the beginning.<br />
 * WARNING: ascii_hex_bytes is modified during the process.<br />
 */
int obd_recv_with_expected_databytes_start(final OBDIFace* iface, final char * ascii_hex_bytes);
/**
 * Easiers
 */
#define OBDIFACE OBDIFace *
#define BUFFERFILTER BufferFilter *

#include "interface.h"
#include "saej1979/saej1979.h"
#include "com/serial/can/can.h"
#include "com/serial/kwp2000.h"

#endif
