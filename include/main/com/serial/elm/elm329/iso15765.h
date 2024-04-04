#ifndef __ELM329_ISO_15765
#define __ELM329_ISO_15765

#include "elm329.h"
#include "com/can/iso15765.h"

bool elm329_iso15765_parse_response(final ELM329Device* elm329, final Vehicle* vehicle);

#endif
