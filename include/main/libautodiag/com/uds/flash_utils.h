#ifndef __AD_COM_UDS_FLASH_UTILS_H
#define __AD_COM_UDS_FLASH_UTILS_H

#include "libautodiag/com/uds/uds.h"
#include "libautodiag/lang/all.h"
#include "libautodiag/com/vehicle_interface.h"

AD_OBJECT_H(ecu_memory,
    ad_object_ECU * ecu;
    byte fmt_address:4;
    byte fmt_length:4;
    Buffer * address;
    Buffer * length;
    Buffer * content;
)
bool ad_uds_write_memory_by_address(VehicleIFace * iface, Buffer * address, Buffer * length, Buffer * memory);
Buffer * ad_uds_read_memory_by_address(VehicleIFace * iface, Buffer * address, Buffer * length);

#endif