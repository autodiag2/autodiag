#ifndef __SERIAL_SHOW_DTC_SAEJ1979_H
#define __SERIAL_SHOW_DTC_SAEJ1979_H

#define _GNU_SOURCE
#include <stdio.h>
#include "list.h"
#include "saej1979.h"
#include "ui/config.h"
#include "database.h"

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
    CarModel *car;
} SAEJ1979_DTC_DESCRIPTION;

typedef enum {
    SAEJ1979_DTC_TYPE_POWERTRAIN, SAEJ1979_DTC_TYPE_CHASSIS,
    SAEJ1979_DTC_TYPE_BODY, SAEJ1979_DTC_TYPE_NETWORK
} SAEJ1979_DTC_TYPE;

SAEJ1979_DTC_DESCRIPTION * saej1979_dtc_description_new();
void saej1979_dtc_description_free(SAEJ1979_DTC_DESCRIPTION *desc);
void saej1979_dtc_description_dump(SAEJ1979_DTC_DESCRIPTION *desc);

typedef struct {
    int size;
    SAEJ1979_DTC_DESCRIPTION *list;
} SAEJ1979_DTC_DESCRIPTION_list;

SAEJ1979_DTC_DESCRIPTION_list * saej1979_dtc_description_list_new();
void saej1979_dtc_description_list_free(SAEJ1979_DTC_DESCRIPTION_list * list);
void saej1979_dtc_description_list_append(SAEJ1979_DTC_DESCRIPTION_list * list, SAEJ1979_DTC_DESCRIPTION *desc);

typedef struct {
    SAEJ1979_DTC_TYPE type;
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

typedef struct {
    int size;
    SAEJ1979_DTC **list;
} SAEJ1979_DTC_list;

SAEJ1979_DTC_list * saej1979_dtc_list_new();
void saej1979_dtc_list_free(SAEJ1979_DTC_list * list);
void saej1979_dtc_list_append(SAEJ1979_DTC_list * list, SAEJ1979_DTC *dtc);
void saej1979_dtc_list_append_list(SAEJ1979_DTC_list * list, SAEJ1979_DTC_list * another);
SAEJ1979_DTC * saej1979_dtc_list_get(SAEJ1979_DTC_list * list, char *dtc);

/**
 * Service 03 retrieve stored DTCs
 */
SAEJ1979_DTC_list * saej1979_retrieve_stored_dtcs(final OBDIFACE iface);
SAEJ1979_DTC_list * saej1979_retrieve_pending_dtcs(final OBDIFACE iface);
SAEJ1979_DTC_list * saej1979_retrieve_permanent_dtcs(final OBDIFACE iface);
/**
 * eg. P1122
 * @return dtc as string, result must be then cleared
 */
char * saej1979_dtc_to_string(final SAEJ1979_DTC * dtc);
/**
 * Fill DTC description fields from the file codes.tsv
 */
void saej1979_fill_dtc_from_codes_file(final SAEJ1979_DTC * dtc, final SAEJ1979_DTC_DESCRIPTION * dtc_desc);

char * saej1979_dtc_type_to_string(final SAEJ1979_DTC_TYPE type);
/**
 * @return a string describing the code
 */
char * saej1979_dtc_categorization_string(final SAEJ1979_DTC * dtc);

#endif
