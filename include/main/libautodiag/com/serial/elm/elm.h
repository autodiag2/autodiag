#ifndef __ELM_H
#define __ELM_H
/**
 * An ELM device provides at least the following commands
 */
#include "libautodiag/com/serial/serial.h"
#include "libautodiag/com/serial/serial_list.h"
#include "libautodiag/com/obd/obd.h"
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

// Standard RS232 response
#define ELM_RESPONSE_UNKNOWN                 0xF00

static int ELMResponseOffset = 0xF00;
static int ELMResponseStrNumber = 1;
static char * ELMResponseStr[] = {
    "?"
};

int elm_guess_response(final char * buffer);
int elm_linefeeds(final SERIAL port, final bool state);
int elm_echo(final SERIAL port, final bool state);
char * elm_print_id(final SERIAL port);
/**
 * Try to open an elm device from the provided serial
 */
OBDIFace* elm_open_from_serial(final SERIAL port);

Buffer * elm_ascii_to_bin(final ELMDevice * elm, final Buffer * ascii);
Buffer * elm_ascii_to_bin_str(final ELMDevice * elm, final char * ascii, final char * end_ptr);
void elm_ascii_to_bin_internal(final bool printing_of_spaces, final Buffer * bin, final char * ascii, final char * end_ptr);
char* elm_ascii_from_bin(final bool printing_of_spaces, final Buffer * bin);
void elm_debug(final ELMDevice * elm);
bool elm_standard_obd_message_parse_response(final ELMDevice* elm, final Vehicle* vehicle);

#include "elm329/elm329.h"
#include "elm327/elm327.h"
#include "elm323/elm323.h"
#include "elm322.h"
#include "elm320.h"

#endif
