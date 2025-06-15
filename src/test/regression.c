#include "libTest.h"

bool testBuffer();
bool testGlobals();
bool testSerial(OBDIFace* iface);
bool testOBD(OBDIFace* iface);
bool testELM327(OBDIFace* iface);
bool testSAEJ1979(OBDIFace* iface);
bool testSerialListOperations(OBDIFace* iface);
bool testCarDatabaseLoad(OBDIFace* iface);
bool testIniTools(OBDIFace* iface);
bool testElm(final OBDIFace* iface);
bool testStringList();
bool testSIM();
bool testISO3779();

int main(int argc, char **argv) {
    log_set_from_env();
    initLibTest();
    
    
    runTestMaybe(testBuffer,null);
    runTestMaybe(testGlobals,null);
    runTestMaybe(testStringList,null);
    runTestMaybe(testSIM,null);
    runTestMaybe(testISO3779,null);

    final OBDIFace* iface = port_open(start_elm327_simulation());

    runTestMaybe(testIniTools, iface);
    runTestMaybe(testELM327, iface);
    runTestMaybe(testSerial, iface);
    runTestMaybe(testOBD, iface);
    runTestMaybe(testCarDatabaseLoad, iface);
    runTestMaybe(testSerialListOperations, iface);
    runTestMaybe(testSAEJ1979, iface);
    runTestMaybe(testElm, iface);

    obd_close(iface);
    obd_free(iface);

    return 0;
}
