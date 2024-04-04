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

int main() {
    log_set_from_env();
    initLibTest();
    runTest(testBuffer,null);
    runTest(testGlobals,null);
    runTest(testStringList,null);
    runTest(testSIM,null);

    final OBDIFace* iface = port_open(start_elm327_simulation());

    runTest(testIniTools, iface);
    runTest(testELM327, iface);
    runTest(testSerial, iface);
    runTest(testOBD, iface);
    runTest(testCarDatabaseLoad, iface);
    runTest(testSerialListOperations, iface);
    runTest(testSAEJ1979, iface);
    runTest(testElm, iface);

    obd_close(iface);
    obd_free(iface);

    return 0;
}
