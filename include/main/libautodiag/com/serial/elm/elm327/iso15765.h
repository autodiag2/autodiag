#ifndef __AD_ELM327_ISO_15765_H
#define __AD_ELM327_ISO_15765_H

#include "elm327.h"
#include "libautodiag/com/can/iso15765.h"

bool elm327_iso15765_parse_response(final ELM327Device* elm327, final Vehicle* vehicle);

bool elm327_protocol_is_can_11_bits_id(final ELM327_PROTO proto);
bool elm327_protocol_is_can_29_bits_id(final ELM327_PROTO proto);

#endif
