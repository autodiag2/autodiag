#include "libTest.h"
#include "libautodiag/model/iso3779.h"

bool testISO3779() {
    {
        final Buffer * vin = buffer_from_ascii("VF1BB05CF26010203");
        {
            char *country = ISO3779_decode_country_from(vin);
            assert(strcmp(country, "France") == 0);
        }
        {
            char *country = ISO3779_decode_region_from(vin);
            assert(strcmp(country, "Europe") == 0);
        }
        {
            char *manufaturer = ISO3779_decode_manufacturer_from(vin);
            assert(strncasecmp(manufaturer, "RENAULT", strlen("RENAULT")) == 0);
        }
        {
            ISO3779_decoded* decoded = ISO3779_decode_from(vin);
            assert(decoded->vis.year == 2002);
        }
    }
    {
        final Buffer * vin = buffer_from_ascii("LA9BB05CF26LC0010");
        char *manufaturer = ISO3779_decode_manufacturer_from(vin);
        printf("manufaturer=%s\n", manufaturer);
        assert(strncasecmp(manufaturer,"byd", 3) == 0);
    }
    {
        final Buffer * vin = buffer_from_ascii("KF9BB05CF26004010");
        char *manufaturer = ISO3779_decode_manufacturer_from(vin);
        assert(strncasecmp(manufaturer,"tomcar", 6) == 0);
    }
    {
        final Buffer * vin = buffer_from_ascii("SA9BB05CF26202010");
        char *manufaturer = ISO3779_decode_manufacturer_from(vin);
        printf("manufaturer=%s\n", manufaturer);
        assert(strncasecmp("morgan",manufaturer,6) == 0);
    }
    return true;
}