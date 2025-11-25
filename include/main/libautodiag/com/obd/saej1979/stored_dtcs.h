#ifndef __SERIAL_SHOW_DTC_SAEJ1979_H
#define __SERIAL_SHOW_DTC_SAEJ1979_H

#define _GNU_SOURCE
#include <stdio.h>
#include "libautodiag/dirent.h"
#include "libautodiag/lang/list.h"
#include "saej1979.h"
#include "libautodiag/installation.h"
#include "libautodiag/model/database.h"
#include "../iso15031/iso15031_6.h"
#include "libautodiag/file.h"
#include "libautodiag/model/dtc.h"
#include <string.h>

typedef struct {
    DTC;
    ISO15031_DTC_TYPE type;
    /**
     * Ascii value of the dtc (less the type)
     */
    char number[5];
} SAEJ1979_DTC;

SAEJ1979_DTC * saej1979_dtc_new();
void saej1979_dtc_free(SAEJ1979_DTC *dtc);
char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc);

/**
 * Service 03 retrieve stored DTCs
 */
list_DTC * saej1979_retrieve_stored_dtcs(final VehicleIFace* iface, final Vehicle *filter);
list_DTC * saej1979_retrieve_pending_dtcs(final VehicleIFace* iface, final Vehicle *filter);
list_DTC * saej1979_retrieve_permanent_dtcs(final VehicleIFace* iface, final Vehicle *filter);
/**
 * eg. P1122
 * @return dtc as string, result must be then cleared
 */
char * saej1979_dtc_to_string(final SAEJ1979_DTC * dtc);
/**
 * eg. 1122
 */
Buffer* saej1979_dtc_bin_from_string(char *dtc_string);

#endif
