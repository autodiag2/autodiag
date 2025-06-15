#include "libTest.h"
#include "libautodiag/model/iso3779.h"

bool testISO3779() {
    {
        final Buffer * vin = buffer_from_ascii("VF1BB05CF26010203");
        final ISO3779 * decoder = ISO3779_new();
        ISO3779_decode(decoder, vin);
        assert(strcmp(decoder->country, "france") == 0);
        char *region = ISO3779_region(decoder);
        assert(strcmp(region, "Europe") == 0);
        assert(strncasecmp(decoder->manufacturer, "RENAULT",strlen("RENAULT")) == 0);
        assert(decoder->year == 2002);
    }
    {
        final Buffer * vin = buffer_from_ascii("LA9BB05CF26LC0010");
        final ISO3779 * decoder = ISO3779_new();
        ISO3779_decode(decoder,vin);
        assert(strncasecmp(decoder->manufacturer, "byd", 3) == 0);
    }
    {
        final Buffer * vin = buffer_from_ascii("KF9BB05CF26004010");
        final ISO3779 * decoder = ISO3779_new();
        ISO3779_decode(decoder, vin);
        assert(strncasecmp(decoder->manufacturer, "tomcar", 3) == 0);
    }
    {
        final Buffer * vin = buffer_from_ascii("SA9BB05CF26202010");
        final ISO3779 * decoder = ISO3779_new();
        ISO3779_decode(decoder, vin);
        assert(strncasecmp(decoder->manufacturer, "morgan", 6) == 0);
    }
    return true;
}