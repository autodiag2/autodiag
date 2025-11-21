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

typedef struct {
    /**
     * Text reason describing the DTC
     */
    char * reason;
    /**
     * Recommended solution
     */
    char * solution;
    /**
     * A DTC is linked to a car model
     */
    Vehicle *vehicle;
} SAEJ1979_DTC_DESCRIPTION;

SAEJ1979_DTC_DESCRIPTION * saej1979_dtc_description_new();
void saej1979_dtc_description_free(SAEJ1979_DTC_DESCRIPTION *desc);
void saej1979_dtc_description_dump(SAEJ1979_DTC_DESCRIPTION *desc);

typedef struct {
    int size;
    SAEJ1979_DTC_DESCRIPTION *list;
} list_SAEJ1979_DTC_DESCRIPTION;

list_SAEJ1979_DTC_DESCRIPTION * list_SAEJ1979_DTC_DESCRIPTION_new();
void list_SAEJ1979_DTC_DESCRIPTION_free(list_SAEJ1979_DTC_DESCRIPTION * list);
void list_SAEJ1979_DTC_DESCRIPTION_append(list_SAEJ1979_DTC_DESCRIPTION * list, SAEJ1979_DTC_DESCRIPTION *desc);

typedef struct {
    ISO15031_DTC_TYPE type;
    /**
     * Ascii value of the dtc (less the type)
     */
    char number[5];
    /**
     * Error corresponding to the DTC
     */
    list_SAEJ1979_DTC_DESCRIPTION * description;
} SAEJ1979_DTC;

SAEJ1979_DTC * saej1979_dtc_new();
void saej1979_dtc_free(SAEJ1979_DTC *dtc);
char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc);

AD_LIST_H(SAEJ1979_DTC)
void list_SAEJ1979_DTC_append_list(list_SAEJ1979_DTC * list, list_SAEJ1979_DTC * another);
SAEJ1979_DTC * list_SAEJ1979_DTC_get(list_SAEJ1979_DTC * list, char *dtc);

/**
 * Service 03 retrieve stored DTCs
 */
list_SAEJ1979_DTC * saej1979_retrieve_stored_dtcs(final VehicleIFace* iface, final Vehicle *filter);
list_SAEJ1979_DTC * saej1979_retrieve_pending_dtcs(final VehicleIFace* iface, final Vehicle *filter);
list_SAEJ1979_DTC * saej1979_retrieve_permanent_dtcs(final VehicleIFace* iface, final Vehicle *filter);
/**
 * eg. P1122
 * @return dtc as string, result must be then cleared
 */
char * saej1979_dtc_to_string(final SAEJ1979_DTC * dtc);
/**
 * eg. 1122
 */
Buffer* saej1979_dtc_bin_from_string(char *dtc_string);
/**
 * Fill DTC description fields from the file codes.tsv
 */
void saej1979_fill_dtc_from_codes_file(final SAEJ1979_DTC * dtc, final SAEJ1979_DTC_DESCRIPTION * dtc_desc);

#endif
