#include "libTest.h"
#include "libautodiag/com/doip/doip.h"

bool testDOIP() {
    {
        Buffer * payload = buffer_from_ascii_hex("1000");
        object_DoIPDiagMessage * msg = object_DoIPDiagMessage_new();
        msg->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
        msg->payload.src_addr = buffer_from_ascii_hex("0000");
        msg->payload.dst_addr = buffer_from_ascii_hex("07E8");
        msg->payload.data = payload;
        Buffer * serialized = doip_diag_message_serialize(msg);
        object_DoIPDiagMessage * msg2 = doip_diag_message_parse(serialized);
        Buffer * serialized2 = doip_diag_message_serialize(msg2);
        assert(buffer_equals(serialized, serialized2));
        tf_test_output("Serialization is reflective");
    }
    char * location = tf_sim_doip_start();
    {
        object_DoIPDevice * device = object_DoIPDevice_new();
        device->location = location;
        device->open(device);
        object_DoIPDiagMessage * msg = object_DoIPDiagMessage_new();
        msg->payload.data = buffer_from_ascii_hex("1000");
        msg->payload.dst_addr = buffer_from_ascii_hex("07E8");
        msg->payload.src_addr = buffer_from_ascii_hex("0000");
        doip_diag_message_dump(msg);
        Buffer * serialized = doip_diag_message_serialize(msg);
        char * serialized_str = buffer_to_hex_string(serialized);
        log_msg(LOG_DEBUG, "serialized: '%s'", serialized_str);
        doip_send_internal(device, serialized_str);
        device->clear_data(device);
        doip_recv_internal(device);
        buffer_dump(device->recv_buffer);
        assert(strstr(buffer_to_hex_string(device->recv_buffer), "50") != null);
        device->close(device);
    }
    {
        VehicleIFace * iface = tf_doip_open(location);
        list_UDS_DTC * dtcs = uds_read_all_dtcs(iface, null);
        assert(0 < dtcs->size);
        for(int i = 0; i < dtcs->size; i++) {
            log_msg(LOG_DEBUG, "DTC: %s", UDS_DTC_to_string(dtcs->list[i]));
        }
    }
    return true;
}