#ifndef __ELM_H
#define __ELM_H
/**
 * An ELM device provides at least the following commands
 */
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/serial/list_serial.h"
#include "elm_iso15765.h"

typedef struct {
    Serial;
    /**
     * Did responses print space between bytes
     */
    bool printing_of_spaces;
    /**
     * Mainly send at commands to prepare interface.
     */
    bool (*configure)(final Device* elm);
} ELMDevice;

#define CAST_ELM_DEVICE_CONFIGURE(var) ((bool (*)(final Device*))var)
#define CAST_ELM_DEVICE(var) ((ELMDevice*)var)

#define ELM_RESPONSE_UNKNOWN                 0xF00

static int ELMResponseOffset = 0xF00;
static int ELMResponseStrNumber = 1;
static char * ELMResponseStr[] = {
    "?"
};

int elm_guess_response(final char * buffer);
int elm_linefeeds(final Serial * port, final bool state);
int elm_echo(final Serial * port, final bool state);
/**
 * Make the elm to print it's identification string
 * @warning there is no lock of interface here, caller must ensure exclusive access to the device
 */
char * elm_print_id(final Serial * port);
/**
 * Try to open an elm device from the provided serial
 */
ELMDevice* elm_open_from_serial(final Serial * port);

Buffer * elm_ascii_to_bin(final ELMDevice * elm, final Buffer * ascii);
Buffer * elm_ascii_to_bin_str(final ELMDevice * elm, final char * ascii, final char * end_ptr);
void elm_ascii_to_bin_internal(final bool printing_of_spaces, final Buffer * bin, final char * ascii, final char * end_ptr);
char* elm_ascii_from_bin(final bool printing_of_spaces, final Buffer * bin);
void elm_debug(final ELMDevice * elm);
bool elm_standard_obd_message_parse_response(final ELMDevice* elm, final Vehicle* vehicle);

/**
 * Ensure that connection to vehicle is right, for some reason some devices incorrectly choose protocol.
 * @param protocol_max_value maximum protocol value included
 */
bool elm_ensure_protocol_config_success(final ELMDevice* elm, final int protocol_max_value);

#include "elm329/elm329.h"
#include "elm327/elm327.h"
#include "elm323/elm323.h"
#include "elm322.h"
#include "elm320.h"

#endif
