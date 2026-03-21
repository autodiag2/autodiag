#ifndef __AD_SIM_ELM327_BUS_H
#define __AD_SIM_ELM327_BUS_H

#include "libautodiag/sim/elm327/elm327.h"

#define ELM327_CAN_28_BITS_DEFAULT_PRIO 0x18

char * sim_elm327_bus(SimELM327 * elm327, char * hex_string_request);
/**
 * Define if incoming is hexa string request.
 */
bool sim_elm327_parse_request(SimELM327 * elm327, char * hex_string_request, bool * isHexString_rv, bool * hasSpaces_rv);

#endif