#include "libTest.h"

bool testSerialListOperations(VehicleIFace* iface) {
    object_DeviceTable * table = object_DeviceTable_new();
    device_table_fill(table);
    Serial * first = table->list->list[0];
    return device_table_remove(table, first);
}
