#include "libTest.h"
#include "libautodiag/com/doip/doip.h"

bool testDOIP() {
    Buffer * target_ecu = ad_buffer_from_ascii_hex("07E8");
    Buffer * tester_address = ad_buffer_from_ascii_hex("1234");
    {
        Buffer * payload = ad_buffer_from_ascii_hex("1000");
        object_DoIPMessage * msg = doip_message_diag(target_ecu, tester_address, payload);
        Buffer * serialized = doip_message_serialize(msg);
        assert(serialized != null);
        object_DoIPMessage * msg2 = doip_message_parse(serialized);
        Buffer * serialized2 = doip_message_serialize(msg2);
        assert(serialized2 != null);
        assert(ad_buffer_equals(serialized, serialized2));
        tf_test_output("Serialization is reflective");
    }
    char * location = tf_sim_doip_start();
    {
        object_DoIPDevice * device = object_DoIPDevice_new();
        device->location = location;
        device->open(device);
        Buffer * payload = ad_buffer_from_ascii_hex("1000");
        object_DoIPMessage * msg = doip_message_diag(target_ecu, tester_address, payload);
        doip_message_dump(msg);
        Buffer * serialized = doip_message_serialize(msg);
        char * serialized_str = ad_buffer_to_hex_string(serialized);
        log_msg(LOG_DEBUG, "serialized: '%s'", serialized_str);
        doip_send_internal(device, serialized_str);
        device->clear_data(device);
        device->recv(device);
        ad_buffer_dump(device->recv_buffer);
        assert(strstr(ad_buffer_to_hex_string(device->recv_buffer), "50") != null);
        device->close(device);
    }
    {
        VehicleIFace * iface = tf_doip_open(location);
        list_UDS_DTC * dtcs = uds_read_all_dtcs(iface, null);
        assert(0 < dtcs->size);
        for(int i = 0; i < dtcs->size; i++) {
            log_msg(LOG_INFO, "DTC: %s", UDS_DTC_to_string(dtcs->list[i]));
        }
    }
    return true;
}