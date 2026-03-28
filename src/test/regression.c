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
bool testDOIP();
bool testNetwork();
bool testExpr();
bool testSignals();
bool testSIB();

int main(int argc, char **argv) {
    sim_elm327_non_volatile_wipe_out();
    printf(" ==== REGRESSION TEST ==== \n");

    log_set_from_env();
    tf_init();

    runIfSelected(testSignals, null);
    runIfSelected(testSIB, null);
    runIfSelected(testExpr, null);
    runIfSelected(testHashMap, null);
    runIfSelected(testBuffer,null);
    runIfSelected(testGlobals,null);
    runIfSelected(testSimELM327,null);
    runIfSelected(testISO3779,null);
    runIfSelected(testNetwork,null);
    
    final VehicleIFace* iface = tf_serial_open(tf_sim_elm327_start());

    runIfSelected(testIniTools, iface);
    runIfSelected(testELM327, iface);
    runIfSelected(testSerial, iface);
    runIfSelected(testOBD, iface);
    runIfSelected(testCarDatabaseLoad, iface);
    runIfSelected(testSerialListOperations, iface);
    runIfSelected(testSAEJ1979, iface);
    runIfSelected(testElm, iface);
    runIfSelected(testRecorder, iface);
    runIfSelected(testReplay, null);
    runIfSelected(testUDS, null);
    runIfSelected(testSimUDS, null);
    runIfSelected(testSim, null);
    runIfSelected(testCycle, null);
    runIfSelected(testDOIP, null);

    viface_close(iface);
    viface_free(iface);

    printf(" ==== REGRESSION END ====\n");
    sim_elm327_non_volatile_wipe_out();
    return 0;
}
