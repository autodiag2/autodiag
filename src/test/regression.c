#include "libTest.h"

bool testHashMap();
bool testBuffer();
bool testGlobals();
bool testSerial(VehicleIFace* iface);
bool testOBD(VehicleIFace* iface);
bool testELM327(VehicleIFace* iface);
bool testSAEJ1979(VehicleIFace* iface);
bool testSerialListOperations(VehicleIFace* iface);
bool testCarDatabaseLoad(VehicleIFace* iface);
bool testIniTools(VehicleIFace* iface);
bool testElm(final VehicleIFace* iface);
bool testSIM();
bool testISO3779();
bool testUDS();

int main(int argc, char **argv) {
    log_set_from_env();
    initLibTest();

    /*
    runTestMaybe(testHashMap, null);
    runTestMaybe(testBuffer,null);
    runTestMaybe(testGlobals,null);
    runTestMaybe(testSIM,null);
    runTestMaybe(testISO3779,null);*/
    
    final VehicleIFace* iface = port_open(start_elm327_simulation());

    /*
    runTestMaybe(testIniTools, iface);
    runTestMaybe(testELM327, iface);
    runTestMaybe(testSerial, iface);
    runTestMaybe(testOBD, iface);
    runTestMaybe(testCarDatabaseLoad, iface);
    runTestMaybe(testSerialListOperations, iface);
    runTestMaybe(testSAEJ1979, iface);
    runTestMaybe(testElm, iface);*/
    runTestMaybe(testUDS, null);

    viface_close(iface);
    viface_free(iface);

    return 0;
}
