#ifndef __AD_DATABASE_H
#define __AD_DATABASE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "libautodiag/string.h"
#include "libautodiag/lib.h"
#include "libautodiag/log.h"
#include "libautodiag/initools.h"
#include "libautodiag/installation.h"
#include "libautodiag/model/vehicle.h"
#include "libautodiag/model/dtc.h"

void ad_dtc_fetch_from_db(final DTC *dtc, final Vehicle *filter);

#endif
