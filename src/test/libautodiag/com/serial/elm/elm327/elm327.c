#include "libTest.h"

bool testELM327(VehicleIFace* iface) {
    {
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm = (ELM327Device*)tmp->device;
        for(int i = 0; i < 2; i ++) {
            Buffer * buffer = buffer_new();
            buffer_append_str(buffer,"7E80743010300000000");
            Buffer * bin = elm_ascii_to_bin((ELMDevice*)elm,buffer);
            elm->recv_buffer = buffer;
            if ( ! elm327_iso15765_parse_response(elm,tmp->vehicle) ) return false;
        }
        vehicle_dump(tmp->vehicle);
        if ( tmp->vehicle->ecus_len != 1 ) return false;
    }
    {
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm = (ELM327Device*)tmp->device;
        Buffer * buffer = buffer_new();
        buffer_append_str(buffer,"7E9024300\r7E8024300\r");
        Buffer * bin = elm_ascii_to_bin((ELMDevice*)elm,buffer);
        elm->recv_buffer = buffer;
        assert(elm327_iso15765_parse_response(elm,tmp->vehicle));
        vehicle_dump(tmp->vehicle);
        assert(tmp->vehicle->ecus_len == 2);
    }
    {
        testOutput("Consecutive from one ecu");
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm327 = (ELM327Device*)tmp->device;
        Buffer * buffer = buffer_new();
        buffer_append_str(buffer,"7E9024300\r7E8100E430613361340\r7E82113380087133713\r7E82239AAAAAAAAAAAA\r");
        elm327->recv_buffer = buffer;
        assert(elm327_iso15765_parse_response(elm327,tmp->vehicle));
        vehicle_dump(tmp->vehicle);
        assert(tmp->vehicle->ecus_len == 2);
        
        bool found = false;
        for(unsigned int i = 0; i < tmp->vehicle->ecus_len; i++) {
            final ECU* ecu = tmp->vehicle->ecus[i];
            for(unsigned int j = 0; j < ecu->data_buffer->size; j++) {
                Buffer * data = ecu->data_buffer->list[j];
                if ( 14 == data->size ) {
                    found = true;
                    assert(buffer_equals(buffer_from_ascii_hex("4306133613401338008713371339"),data));
                }
            }
        }
        assert(found);
        
    }
    {
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm327 = (ELM327Device*)tmp->device;
        Buffer * buffer = buffer_new();
        buffer_append_str(buffer,"7E9100E430613361340\r7E8100E430613361340\r7E92113380087133713\r7E82113380087133713\r7E82239AAAAAAAAAAAA\r7E82239AAAAAAAAAAAA\r");
        elm327->recv_buffer = buffer;
        assert(elm327_iso15765_parse_response(elm327,tmp->vehicle));
        vehicle_dump(tmp->vehicle);
        assert(tmp->vehicle->ecus_len == 2);

        final Buffer * should_obtain = buffer_from_ascii_hex("4306133613401338008713371339");
        
        bool found = false;
        for(int i = 0; i < tmp->vehicle->ecus_len; i++) {
            final ECU* ecu = tmp->vehicle->ecus[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                Buffer * data = ecu->data_buffer->list[j];
                assert( 14 == data->size );
                assert(buffer_equals(should_obtain,data));
                found = true;
            }
        }
        assert(found);
    }
    {
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm327 = (ELM327Device*)tmp->device;
        Buffer * buffer = buffer_new();
        buffer_append_str(buffer,"416B1043010300000000\r416B1143010300000000\r");
        elm327->recv_buffer = buffer;
        assert(elm_standard_obd_message_parse_response((ELMDevice*)elm327,tmp->vehicle));
        vehicle_dump(tmp->vehicle);
        assert(tmp->vehicle->ecus_len == 2);
        final Buffer * should_obtain = buffer_from_ascii_hex("43010300000000");
        bool found = false;
        for(int i = 0; i < tmp->vehicle->ecus_len; i++) {
            final ECU* ecu = tmp->vehicle->ecus[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                Buffer * data = ecu->data_buffer->list[j];
                assert(buffer_equals(should_obtain,data));
                found = true;
            }
        }
        assert(found);
    }
    {
        VehicleIFace* tmp = fake_can_iface();
        ELM327Device* elm327 = (ELM327Device*)tmp->device;
        Buffer * buffer = buffer_new();
        elm327->printing_of_spaces = true;
        buffer_append_str(buffer,"48 6B 10 43 01 03 01 04 01 05\r48 6B 10 43 01 06 01 07 01 08\r>");
        elm327->recv_buffer = buffer;
        assert(elm_standard_obd_message_parse_response((ELMDevice*)elm327,tmp->vehicle));
        vehicle_dump(tmp->vehicle);
        assert(tmp->vehicle->ecus_len == 1);

        assert(buffer_equals(buffer_from_ascii_hex("43010301040105"),tmp->vehicle->ecus[0]->data_buffer->list[0]));
        assert(buffer_equals(buffer_from_ascii_hex("43010601070108"),tmp->vehicle->ecus[0]->data_buffer->list[1]));
    }
    return true;
}
