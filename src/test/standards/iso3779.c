#include "libTest.h"
#include "standards/iso3779.h"

bool testISO3779() {
    final Buffer * vin = buffer_from_ascii("VF1BB05CF26010203");
    {
        char *country = iso3779decode_country_from(vin);
        assert(strcmp(country, "France") == 0);
    }
    {
        char *country = iso3779decode_region_from(vin);
        assert(strcmp(country, "Europe") == 0);
    }
    {
        char *manufaturer = iso3779decode_manufacturer_from(vin);
        assert(strcmp(manufaturer, "RENAULT") == 0);
    }
    {
        char *year = ISO3779_vis_get_year_from(vin);
        assert(strcmp(year,"2002") == 0);
    }
    return true;
}