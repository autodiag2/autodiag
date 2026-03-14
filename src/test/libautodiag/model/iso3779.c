#include "libTest.h"
#include "libautodiag/model/iso3779.h"

#define TEST_VIN_DECODE(vinStr) \
    final Buffer * vin = ad_buffer_from_ascii(vinStr); \
    final ISO3779 * decoder = ISO3779_new(); \
    ISO3779_decode(decoder, vin); \
    ISO3779_dump(decoder);

bool testISO3779() {
    {
        TEST_VIN_DECODE("VF1BB05CF26010203");
        assert(strcmp(decoder->country, "FRANCE") == 0);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "Europe") == 0);
        assert(strstr(decoder->manufacturer, "RENAULT") != null);
        assert(decoder->year == 2002);
    }
    {
        TEST_VIN_DECODE("5UXKR6C57E0J71584");
        assert(strstr(decoder->country, "USA") != null);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "North America") == 0);
        assert(strstr(decoder->manufacturer, "BMW") != null);
        assert(decoder->year == 2014);
    }
    {
        TEST_VIN_DECODE("WBAJE7C53KWW25771");
        assert(strstr(decoder->country, "GERMANY") != null);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "Europe") == 0);
        assert(strstr(decoder->manufacturer, "BMW") != null);
        assert(decoder->year == 2019);
    }
    {
        TEST_VIN_DECODE("5UXCR6C00N9K68159");
        assert(strstr(decoder->country, "USA") != null);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "North America") == 0);
        assert(strstr(decoder->manufacturer, "BMW") != null);
        assert(decoder->year == 2022);
    }
    log_msg(LOG_WARNING, "missing case from vpic");
    /*
    {
        TEST_VIN_DECODE("VR3UHZKXZNT090149");
        assert(strstr(decoder->country, "FRANCE") != null);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "Europe") == 0);
        assert(strstr(decoder->manufacturer, "Peugeot") != null);
        assert(decoder->year == 2022);
    }
    */
    return true;
}