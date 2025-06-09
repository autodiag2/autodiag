#ifndef __SAEJ1979_VEHICLE_INFO_H
#define __SAEJ1979_VEHICLE_INFO_H

#include "libautodiag/lib.h"
#include "libautodiag/string.h"
#include "libautodiag/log.h"
#include "libautodiag/buffer.h"
#include "libautodiag/com/obd/obd.h"
#include "saej1979.h"


#define SAEJ1979_VEHICLE_INFO_GENERATE_OBD_REQUEST_ITERATE_BODY(type,sym,errorValue,pid,iterator) type sym(final OBDIFace* iface) { \
    SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,"09" pid,iterator,errorValue,ecu->obd_service.request_vehicle_information) \
}

bool saej1979_vehicle_info_is_pid_supported(final OBDIFace* iface, final int pid);
void* saej1979_vehicle_info_discover_ecus_name(final OBDIFace* iface);

#endif
