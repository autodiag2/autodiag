#ifndef __ELM327_ISO_15765
#define __ELM327_ISO_15765

#include "com/serial/obd/elm327/elm327.h"
#include "com/serial/can/iso15765.h"

bool elm327_can_is_11_bits_id(final OBDIFACE iface);
bool elm327_can_is_29_bits_id(final OBDIFACE iface);
bool elm327_iso15765_parse_response(final OBDIFACE iface);

#endif
