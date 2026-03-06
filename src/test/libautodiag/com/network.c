#include "libTest.h"
#include "libautodiag/com/network.h"

bool testNetwork() {
    char * location = "127.0.0.1:13400";
    char * location_res = network_location(network_location_to_object(location));
    assert(strcmp(location, location_res) == 0);
    return true;
}