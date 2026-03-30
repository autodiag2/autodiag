#ifndef __AD_COM_OBDB_H
#define __AD_COM_OBDB_H

#include "libautodiag/lib.h"
#include "libautodiag/com/http/http.h"

/**
 * Retrieve and store signals from OBDb repos
 */
bool ad_obdb_fetch_signals(char *registry);

#endif