#ifndef __SAEJ1979_VEHICLE_INFO_H
#define __SAEJ1979_VEHICLE_INFO_H

#include "lib/lib.h"
#include "lib/string.h"
#include "lib/log.h"
#include "lib/buffer.h"
#include "com/obd/obd.h"
#include "saej1979.h"


#define SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(type,sym,errorValue,pid,iterator) type sym(final OBDIFace* iface) { \
    SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,"09" pid,iterator,errorValue,ecu->obd_service.request_vehicle_information) \
}

bool saej1979_vehicle_info_is_pid_supported(final OBDIFace* iface, final int pid);
void* saej1979_vehicle_info_discover_ecus_name(final OBDIFace* iface);

#endif
