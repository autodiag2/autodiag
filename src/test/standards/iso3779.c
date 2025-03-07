#include "libTest.h"
#include "standards/iso3779.h"

bool testISO3779() {
    {
        final Buffer *vin = ascii_to_bin_buffer("VF10");
        char *country = iso3779decode_country_from(vin);
        assert(strcmp(country, "France") == 0);
    }
    return true;
}