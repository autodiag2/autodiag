#ifndef __AD_ELM329_ISO_15765_H
#define __AD_ELM329_ISO_15765_H

#include "elm329.h"
#include "libautodiag/com/can/iso15765.h"

bool elm329_iso15765_parse_response(final ELM329Device* elm329, final Vehicle* vehicle);

#endif
