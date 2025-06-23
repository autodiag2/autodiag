#include "libTest.h"

bool testSerialListOperations(OBDIFace* iface) {
    serial_list_fill();
    Serial * first = serial_list.list[0];
    return serial_list_remove(first);
}
