#include "libTest.h"

bool testSerialListOperations(OBDIFace* iface) {
    serial_list_fill();
    SERIAL first = serial_list.list[0];
    return serial_list_remove(first);
}
