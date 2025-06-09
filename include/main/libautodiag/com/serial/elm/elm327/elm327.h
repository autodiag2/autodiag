#ifndef __SERIAL_ELM327_H
#define __SERIAL_ELM327_H

#include "libautodiag/com/obd/obd.h"
#include "../elm.h"

typedef enum {
    ELM327_PROTO_NONE, ELM327_PROTO_SAE_J1850_1,
    ELM327_PROTO_SAE_J1850_2, ELM327_PROTO_ISO_9141_2,
    ELM327_PROTO_ISO_14230_4_KWP2000_1, ELM327_PROTO_ISO_14230_4_KWP2000_2,
    ELM327_PROTO_ISO_15765_4_CAN_1,ELM327_PROTO_ISO_15765_4_CAN_2,
    ELM327_PROTO_ISO_15765_4_CAN_3,ELM327_PROTO_ISO_15765_4_CAN_4,
    ELM327_PROTO_SAEJ1939,ELM327_PROTO_USER1_CAN,
    ELM327_PROTO_USER2_CAN
} ELM327_PROTO;

typedef struct {
    ELMDevice;
    /**
     * Protocol used to communicate on that interface, some interfaces could communicate with multiple protocols.<br />
     * This data represent only the protocol currently in use.<br />
     * e.g. ISO 15765-4 CAN (11-bit ID, 500 kBit/s)<br />
     */
    ELM327_PROTO protocol;
} ELM327Device;

/**
 * Serial response type to a command
 */
#define ELM327_RESPONSE_OFFSET                  0xF000
#define ELM327_RESPONSE_SEARCHING               0xF000
#define ELM327_RESPONSE_STOPPED                 0xF001
#define ELM327_RESPONSE_NO_DATA                 0xF002
#define ELM327_RESPONSE_ACT_ALERT               0xF003
#define ELM327_RESPONSE_ACT_ALERT2              0xF004
#define ELM327_RESPONSE_BUFFER_FULL             0xF005
#define ELM327_RESPONSE_BUS_BUSY                0xF006
#define ELM327_RESPONSE_BUS_ERROR               0xF007
#define ELM327_RESPONSE_CAN_ERROR               0xF008
#define ELM327_RESPONSE_DATA_ERROR              0xF009
#define ELM327_RESPONSE_DATA_ERROR_AT_LINE      0xF00A
#define ELM327_INTERNAL_ERROR                   0xF00B
#define ELM327_FEEDBACK_ERROR                   0xF00C
#define ELM327_LOW_POWER                        0xF00D
#define ELM327_LOW_POWER2                       0xF00E
#define ELM327_LOW_VOLTAGE_RESET                0xF00F
#define ELM327_CAN_DATA_ERROR                   0xF010
#define ELM327_UNABLE_TO_CONNECT                0xF011

static int ELM327ResponseStrNumber = 18;
static char * ELM327ResponseStr[] = {
    "SEARCHING",
    "STOPPED",
    "NO DATA",
    "ACT ALERT",
    "!ACT ALERT",
    "Buffer * FULL",
    "BUS BUSY",
    "BUS ERROR",
    "CAN ERROR",
    "DATA ERROR",
    "<DATA ERROR",
    "ERR",
    "FB ERROR",
    "LP ALERT",
    "!LP ALERT",
    "LV RESET",
    "<RX ERROR",
    "UNABLE TO CONNECT"
};

ELM327Device* elm327_new();
ELM327Device* elm327_new_from_serial(final Serial *serial);
/**
 * Receive serial data and guess special signals sent back by the interface.
 * We wait for the prompt char to be sent back by the elm327
 */
int elm327_recv(final ELM327Device*);
/**
 *
 */
int elm327_guess_response(final char * buffer);

/**
 * Parse response message in traditionnal message (3 bytes header form)
 */
bool elm327_standard_obd_message_parse_response(final ELM327Device* elm327, final Vehicle* vehicle);
bool elm327_obd_data_parse(final ELM327Device* elm327, final Vehicle* vehicle);

ELM327_PROTO elm327_get_current_protocol(final ELM327Device* elm327);
char* elm327_protocol_to_string(final ELM327_PROTO proto);
/**
 * @return true on confirmation from the remote, false on missing confirmation
 */
bool elm327_printing_of_spaces(final ELM327Device* elm327, bool state);
/**
 * @return current voltage level or -1 on error
 */
double elm327_get_current_battery_voltage(final ELM327Device* elm327);
/**
 * Calibrate current battery voltage
 */
bool elm327_calibrate_battery_voltage(final ELM327Device* elm327, double voltage);
/**
 * Set can autoformatting state
 */
bool elm327_set_auto_formatting(final ELM327Device* elm327, final bool state);

bool elm327_is_can(final ELM327Device* elm327);
bool elm327_protocol_is_can(final ELM327_PROTO proto);
bool elm327_protocol_is_j1939(final ELM327_PROTO proto);

#include "iso15765.h"

#endif
