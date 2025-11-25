#ifndef __MODEL_DTC_H
#define __MODEL_DTC_H

#include "libautodiag/lang/all.h"
#include "libautodiag/model/vehicle.h"

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

/**
 * Support DTC format 0x00 -> 0x04
 */
typedef struct {
    byte data[3];
    list_DTC_DESCRIPTION * description;
} DTC;

#endif