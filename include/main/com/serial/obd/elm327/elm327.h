#ifndef __SERIAL_ELM327_H
#define __SERIAL_ELM327_H

#include "com/serial/obd/obd.h"
#include "at/elm327.h"

/**
 * Serial response type to a command
 */
typedef enum {
    // null element of this element
    ELM327_RESPONSE_NULL,
    ELM327_RESPONSE_EMPTY,
    ELM327_RESPONSE_DATA,
    ELM327_RESPONSE_PROMPT,
    // Standard RS232 response
    ELM327_RESPONSE_UNKNOWN,
    ELM327_RESPONSE_OK,
    ELM327_RESPONSE_SEARCHING,
    // User interrupt has been triggered by user during the search
    ELM327_RESPONSE_STOPPED,
    // No response from the vehicle detected
    ELM327_RESPONSE_NO_DATA,
    // Triggered if no RS232 or OBD activity for some time
    ELM327_RESPONSE_ACT_ALERT,
    ELM327_RESPONSE_ACT_ALERT2,
    // Too much data received at too much high speed
    ELM327_RESPONSE_BUFFER_FULL,
    // Too much activity detected on the bus
    ELM327_RESPONSE_BUS_BUSY,
    // Generic error detected on the bus
    ELM327_RESPONSE_BUS_ERROR,
    // Wrong protocol setup or baud speed error
    ELM327_RESPONSE_CAN_ERROR,
    // Information sent back by the vehicle is incorrect or could not be recovered
    ELM327_RESPONSE_DATA_ERROR,
    ELM327_RESPONSE_DATA_ERROR_AT_LINE,
    // Hardware issue probably with ELM327 adaptater - in the format ERRxx where xx is the error number
    ELM327_INTERNAL_ERROR,
    ELM327_FEEDBACK_ERROR,
    // Interface about to switch to low power mode
    ELM327_LOW_POWER,
    ELM327_LOW_POWER2,
    // Reset because of voltage dropping below 5V
    ELM327_LOW_VOLTAGE_RESET,
    ELM327_CAN_DATA_ERROR,
    // Elm327 has tried all protocols and could not detect a compatible one
    ELM327_UNABLE_TO_CONNECT
} ELM327Response;

static int ELM327ResponseStrNumber = 23;
static char * ELM327ResponseStr[] = {
    null,
    null,
    null,
    ">",
    "?",
    "OK",
    "SEARCHING",
    "STOPPED",
    "NO DATA",
    "ACT ALERT",
    "!ACT ALERT",
    "BUFFER FULL",
    "BUS BUSY",
    "BUS ERROR",
    "CAN ERROR",
    "DATA ERROR",
    "<DATA ERROR"
    "ERR",
    "FB ERROR",
    "LP ALERT",
    "!LP ALERT",
    "LV RESET",
    "<RX ERROR",
    "UNABLE TO CONNECT"
};

/**
 * Send data optimized for ELM327 interface
 */
int elm327_optimized_send(final OBDIFACE iface, const char *command, final int expected_lines_of_data);
/**
 * Receive serial data and guess special signals sent back by the interface
 */
int elm327_recv(final OBDIFACE port);
/**
 * Wait for the remote confirmation that sent command has been received.
 */
void elm327_send_wait_remote_confirmation(final nonnull OBDIFACE port);
/**
 * Additionnal procedure to configure elm327 when openning
 */
void elm327_configure(final OBDIFACE iface);
/**
 * Parse response message in traditionnal message (3 bytes header form)
 */
bool elm327_standard_obd_message_parse_response(final OBDIFACE iface);
#define ELM327_OBD_MESSAGE_HEADER_STANDARD_SIZE 3
/**
 * Wait until the data has been fetched by OBD interface.<br />
 * If data is not ready, we wait for it.<br />
 */
int elm327_recv_blocking(final OBDIFACE iface);
ELM327Response elm327_guess_response(final char * buffer);
/**
 * Same as generic AT command but with response confirmation from ELM327
 */
bool serial_at_elm327_echo(final nonnull SERIAL port, final bool state);
int elm327_recv_until_prompt(final OBDIFACE iface);
#include "iso15765.h"
#include "at/elm327.h"

#endif
