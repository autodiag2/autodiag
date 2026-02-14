#include "libTest.h"
#include "libautodiag/com/doip/doip.h"

bool testDOIP() {
    Buffer * payload = buffer_from_ascii_hex("1000");
    object_DoIPMessage * msg = object_DoIPMessage_new();
    msg->payload_type = DOIP_DIAGNOSTIC_MESSAGE;
    msg->payload_length = payload->size;
    msg->payload->src_addr = buffer_from_ascii_hex("0000");
    msg->payload->dst_addr = buffer_from_ascii_hex("07E8");
    msg->payload->data = payload;
    Buffer * serialized = doip_serialize_message(msg);
    object_DoIPMessage * msg2 = doip_parse_message(serialized);
    Buffer * serialized2 = doip_serialize_message(msg2);
    assert(buffer_equals(serialized, serialized2));
    return true;
}