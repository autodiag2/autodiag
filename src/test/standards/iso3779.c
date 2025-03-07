#include "libTest.h"
#include "standards/iso3779.h"

bool testISO3779() {
    {
        final Buffer *vin = buffer_from_ascii("VF10");
        char *country = iso3779decode_country_from(vin);
        assert(strcmp(country, "France") == 0);
    }
    {
        final Buffer *vin = buffer_from_ascii("VF10");
        char *country = iso3779decode_region_from(vin);
        assert(strcmp(country, "Europe") == 0);
    }
    return true;
}