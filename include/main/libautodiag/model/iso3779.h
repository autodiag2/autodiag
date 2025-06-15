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

#define ISO3779_WMI_MANUFACTURER_LESS_500 '9'
typedef struct {
    struct {
        char * country;
        char * manufacturer;
    } wmi;
    struct {
        char *data;
    } vds;
    struct {
        int year;
        char *data;
    } vis;

} ISO3779_decoded;

ISO3779_decoded * ISO3779_vin_new();
void ISO3779_vin_free(ISO3779_decoded *vin);
/**
 * Decode fields from the vin number.
 */
ISO3779_decoded* ISO3779_decode_from(final Buffer *vin);
/**
 * Get the country associated with the VIN.
 */
char * ISO3779_decode_country_from(final Buffer *vin_raw);
/**
 * Get the geographical region associated with VIN.
 */
char * ISO3779_decode_region_from(final Buffer *vin_raw);
char * ISO3779_decode_manufacturer_from(final Buffer *vin_raw);
char* ISO3779_vis_get_year_from(final Buffer *vin_raw);
char* ISO3779_vis_serial_number_from(final Buffer *vin_raw);
void ISO3779_dump(final Buffer *vin);

#endif