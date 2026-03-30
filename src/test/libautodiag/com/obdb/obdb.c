#include "libTest.h"
#include "libautodiag/com/obdb/obdb.h"

bool testOBDB() {
    assert(ad_obdb_fetch_signals("SAEJ1979"));
    assert(ad_obdb_fetch_signals("ACURA-TLX"));
    ad_object_vehicle_signal * signal = ad_signal_get("OBDb.tlx_generator");
    assert(signal != null);
    tf_test_output("OBDb.tlx_generator: formula = %s\n", signal->rv_formula);
    ad_object_vehicle_signal_dump(signal);
    return true;
}