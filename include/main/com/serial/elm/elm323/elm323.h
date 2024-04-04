#ifndef __ELM323_H
#define __ELM323_H

#include "../elm.h"

typedef struct {
    ELMDevice;
} ELM323Device;

#define ELM323_RESPONSE_OFFSET 0xF000
// The ELM323 tried to send the mode command or initialize the bus, but detected too much activity to insert a message. This could be because the bus was in fact busy, but is often due to wiring problems that result in a continuously active input at OBDIn.
#define ELM323_RESPONSE_BUS_BUSY        0xF000
// This message is sent when a ‘feedback’ error is detected. When the K Line is first energized, a check is made to ensure that the signal is seen at OBDIn. If it does not appear there, this message is displayed. Check your wiring before proceeding.
#define ELM323_RESPONSE_FB_ERROR        0xF001
// There was a response from the vehicle, but the information was incorrect or could not be recovered. In the case of a bus initialization, this error signifies that the format bytes received were not as required, so initiation could not continue. If the error occurs during normal operation, it means that the response did not contain enough bytes to be a valid message (which can occur if the signal is interrupted during a data transmission).
#define ELM323_RESPONSE_DATA_ERROR      0xF002
// The error checksum result was not as expected, indicating a data error in the line pointed to (the ELM323 still shows you what it received). There could have been a noise burst which interfered, or a circuit problem. Try re-sending the command.
#define ELM323_RESPONSE_DATA_ERROR2     0xF003
// The IC waited for the period of time that was set by AT ST, and detected no response from the vehicle. It may be that the vehicle had no data to offer, that the mode requested was not supported, or that the vehicle was attending to higher priority issues and could not respond to the request in the allotted time. Try adjusting the AT ST time to be sure that you have allowed sufficient time to obtain a response.
#define ELM323_RESPONSE_NO_DATA         0xF004

static int ELM323ResponseStrNumber = 5;
static char * ELM323ResponseStr[] = {
    "BUS BUSY",
    "FB ERROR",
    "DATA ERROR",
    "<DATA ERROR",
    "NO DATA"
};

int elm323_guess_response(final char * buffer);
ELM323Device* elm323_new();
ELM323Device* elm323_new_from_serial(final Serial *serial);

#endif
