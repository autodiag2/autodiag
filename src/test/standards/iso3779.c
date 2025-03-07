#include "libTest.h"
#include "standards/iso3779.h"

bool testISO3779() {
    {
        final Buffer *vin = buffer_new();
        buffer_append_byte(vin, 'V');
        buffer_append_byte(vin, 'F');
        buffer_append_byte(vin, '1');
        buffer_append_byte(vin, '0');
        char *country = iso3779decode_country_from(vin);
        assert(strcmp(country, "France") == 0);
    }
    return true;
}