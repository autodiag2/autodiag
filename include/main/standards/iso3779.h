/**
 * Defines how to decode a VIN number
 * https://fr.wikipedia.org/wiki/Code_constructeur-WMI
 */
#ifndef _STD_ISO3779_H
#define _STD_ISO3779_H

#include "lib/buffer.h"
#include <string.h>

#define ISO3779_WMI_MANUFACTURER_LESS_500 9
typedef struct {
    struct {
        char * country;
        char * manufacturer;
    } wmi;

} iso3779decoded;

iso3779decoded * iso3779_vin_new();
void iso3779_vin_free(iso3779decoded *vin);
/**
 * Decode fields from the vin number.
 */
iso3779decoded* iso3779decode_from(final Buffer *vin);
/**
 * Get the country associated with the VIN.
 */
char * iso3779decode_country_from(final Buffer *vin_raw);
/**
 * Get the geographical region associated with VIN.
 */
char * iso3779decode_region_from(final Buffer *vin_raw);
char * iso3779decode_manufacturer_from(final Buffer *vin_raw);

#endif