#include "libTest.h"

bool testSerialListOperations(VehicleIFace* iface) {
    object_SerialTable * table = object_SerialTable_new();
    serial_table_fill(table);
    Serial * first = table->list->list[0];
    return serial_table_remove(table, first);
}
