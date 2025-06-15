/**
 * Defines how to decode a VIN number
 * https://fr.wikipedia.org/wiki/Code_constructeur-WMI
 */
#ifndef _STD_ISO3779_H
#define _STD_ISO3779_H

#include "libautodiag/buffer.h"
#include "libautodiag/byte.h"
#include <string.h>
#include <time.h>
#include "libautodiag/installation.h"
#include "libautodiag/file.h"
#include "libautodiag/lib.h"

#define ISO3779_WMI_MANUFACTURER_LESS_500 '9'

/**
 * 1  2  3    4  5  6  7  8  9   10 11 12 13 14 15 16 17
 * WMI-----   VDS------          VIS-----------
 */
typedef struct {
    char * country;
    char * manufacturer;
    int year;
    byte * wmi;
    byte * vds;
    byte * vis;
    Buffer * vin;
} ISO3779;

ISO3779 * ISO3779_new(final Buffer * vin);
void ISO3779_free(ISO3779 *decoder);
void ISO3779_set(ISO3779 *decoder, final Buffer * vin);
void ISO3779_decode(final ISO3779 *decoder);
void ISO3779_decode_at_year(final ISO3779 *decoder, final int year);
bool ISO3779_manufacturer_is_less_500(final ISO3779 *decoder);
/**
 * Get the geographical region associated with VIN.
 */
char * ISO3779_region(final ISO3779 *decoder);
void ISO3779_dump(final ISO3779 *decoder);
#endif