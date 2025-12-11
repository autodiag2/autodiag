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
bool testSimELM327();
bool testSim();
bool testISO3779();
bool testUDS();
bool testReplay();
bool testRecorder(VehicleIFace* iface);
bool testSimUDS();
bool testCycle();

int main(int argc, char **argv) {
    sim_elm327_non_volatile_wipe_out();
    printf(" ==== REGRESSION TEST ==== \n");

    log_set_from_env();
    initLibTest();

    runTestMaybe(testHashMap, null);
    runTestMaybe(testBuffer,null);
    runTestMaybe(testGlobals,null);
    runTestMaybe(testSimELM327,null);
    runTestMaybe(testISO3779,null);
    
    final VehicleIFace* iface = port_open(start_elm327_simulation());

    runTestMaybe(testIniTools, iface);
    runTestMaybe(testELM327, iface);
    runTestMaybe(testSerial, iface);
    runTestMaybe(testOBD, iface);
    runTestMaybe(testCarDatabaseLoad, iface);
    runTestMaybe(testSerialListOperations, iface);
    runTestMaybe(testSAEJ1979, iface);
    runTestMaybe(testElm, iface);
    runTestMaybe(testRecorder, iface);
    runTestMaybe(testReplay, null);
    runTestMaybe(testUDS, null);
    runTestMaybe(testSimUDS, null);
    runTestMaybe(testSimELM327, null);
    runTestMaybe(testSim, null);
    runTestMaybe(testCycle, null);

    viface_close(iface);
    viface_free(iface);

    printf(" ==== REGRESSION END ====\n");
    sim_elm327_non_volatile_wipe_out();
    return 0;
}
