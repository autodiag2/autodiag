#ifndef __MODEL_DTC_H
#define __MODEL_DTC_H

#include "libautodiag/lang/all.h"
#include "libautodiag/model/vehicle.h"
#include "libautodiag/model/database.h"

typedef enum {
    SAE_J2012_DA_DTCFormat_00 = 0x00, //	SAE_J2012-DA_DTCFormat_00 (Diagnostic & Analysis)
    ISO_14229_1_DTCFormat = 0x01, //	ISO_14229-1_DTCFormat
    SAE_J1939_73_DTCFormat = 0x02, //	SAE_J1939-73_DTCFormat
    ISO_11992_4_DTCFormat = 0x03, //	ISO_11992-4_DTCFormat
    SAE_J2012_DA_DTCFormat_04 = 0x04 //	SAE_J2012-DA_DTCFormat_04 as per J2012 WWH-OBD
    // 0x05	Under the Design for Next generation of vehicles (EV, HV, etc…)
    // 0x06 – 0xFF	This range is reserved for future use by ISO or SAE department
} DTC_Format;

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
} DTC_DESCRIPTION;

DTC_DESCRIPTION * dtc_description_new();
void dtc_description_free(DTC_DESCRIPTION *desc);
void dtc_description_dump(DTC_DESCRIPTION *desc);

typedef struct {
    int size;
    DTC_DESCRIPTION *list;
} list_DTC_DESCRIPTION;

list_DTC_DESCRIPTION * list_DTC_DESCRIPTION_new();
void list_DTC_DESCRIPTION_free(list_DTC_DESCRIPTION * list);
void list_DTC_DESCRIPTION_append(list_DTC_DESCRIPTION * list, DTC_DESCRIPTION *desc);

struct DTC;
#define DTC_DATA_SZ 3
/**
 * Support DTC format 0x00 -> 0x04
 */
typedef struct {
    /**
     * Bytes of the DTC, stored in big endian order
     */
    byte data[DTC_DATA_SZ];
    /**
     * Error corresponding to the DTC
     */
    list_DTC_DESCRIPTION * description;
    /**
     * Convert bytes to string representation
     */
    char* (*to_string)(final struct DTC * dtc);
    /**
     * ECU who has emitted this DTC.
     */
    ECU * ecu;
    /**
     * How did this DTC has been detected
     */
    list_object_string * detection_method;
} DTC;

#define CAST_DTC_TO_STRING(var) ((char* (*)(final struct DTC*))var)

AD_LIST_H(DTC)
void list_DTC_append_list(list_DTC * list, list_DTC * another);
DTC * list_DTC_get(list_DTC * list, char *dtc);

/**
 * Fill DTC description fields from the file codes.tsv
 */
void dtc_desc_fill_from_codes_file(final DTC * dtc, final DTC_DESCRIPTION * dtc_desc);
void dtc_description_fetch_from_fs(final DTC * dtc, final Vehicle* filter);

#endif