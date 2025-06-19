#ifndef __SERIAL_SHOW_DTC_SAEJ1979_H
#define __SERIAL_SHOW_DTC_SAEJ1979_H

#define _GNU_SOURCE
#include <stdio.h>
#include "libautodiag/dirent.h"
#include "libautodiag/list.h"
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
} SAEJ1979_DTC_DESCRIPTION_list;

SAEJ1979_DTC_DESCRIPTION_list * SAEJ1979_DTC_DESCRIPTION_list_new();
void SAEJ1979_DTC_DESCRIPTION_list_free(SAEJ1979_DTC_DESCRIPTION_list * list);
void SAEJ1979_DTC_DESCRIPTION_list_append(SAEJ1979_DTC_DESCRIPTION_list * list, SAEJ1979_DTC_DESCRIPTION *desc);

typedef struct {
    ISO15031_DTC_TYPE type;
    /**
     * Ascii value of the dtc (less the type)
     */
    char number[5];
    /**
     * Error corresponding to the DTC
     */
    SAEJ1979_DTC_DESCRIPTION_list * description;
} SAEJ1979_DTC;

SAEJ1979_DTC * saej1979_dtc_new();
void saej1979_dtc_free(SAEJ1979_DTC *dtc);
char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc);

LIST_H(SAEJ1979_DTC)
void SAEJ1979_DTC_list_append_list(SAEJ1979_DTC_list * list, SAEJ1979_DTC_list * another);
SAEJ1979_DTC * SAEJ1979_DTC_list_get(SAEJ1979_DTC_list * list, char *dtc);

/**
 * Service 03 retrieve stored DTCs
 */
SAEJ1979_DTC_list * saej1979_retrieve_stored_dtcs(final OBDIFace* iface, final Vehicle *filter);
SAEJ1979_DTC_list * saej1979_retrieve_pending_dtcs(final OBDIFace* iface, final Vehicle *filter);
SAEJ1979_DTC_list * saej1979_retrieve_permanent_dtcs(final OBDIFace* iface, final Vehicle *filter);
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
