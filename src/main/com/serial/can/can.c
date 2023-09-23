#include "com/serial/can/can.h"

bool obd_iface_is_can(final OBDIFACE iface) {
    switch(iface->protocol) {
        case OBD_PROTO_ISO_15765_4_CAN_1:
        case OBD_PROTO_ISO_15765_4_CAN_2:
        case OBD_PROTO_ISO_15765_4_CAN_3:
        case OBD_PROTO_ISO_15765_4_CAN_4:
        case OBD_PROTO_USER1_CAN:
        case OBD_PROTO_USER2_CAN:
        case OBD_PROTO_SAEJ1939:
            return true;
    }
    return false;
}
