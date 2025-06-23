#include "libTest.h"

bool testSerialListOperations(OBDIFace* iface) {
    list_serial_fill();
    Serial * first = list_serial.list[0];
    return list_serial_remove(first);
}
