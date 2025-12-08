#include "libTest.h"

bool testElm(final VehicleIFace* iface) {
    {
        VehicleIFace* tmp = fake_can_iface();
        ELMDevice * elm = (ELMDevice *)tmp->device;
        elm->printing_of_spaces = false;
        Buffer * buffer = buffer_new();
        buffer_append_bytes(buffer,"000102030405060708090A0B0C0D0E0F",32);
        Buffer * bin = elm_ascii_to_bin(elm,buffer);
        if ( bin->size != 16 ) return false;
        for(int i = 0; i < bin->size; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    {
        VehicleIFace* tmp = fake_can_iface();
        ELMDevice * elm = (ELMDevice *)tmp->device;        
        elm->printing_of_spaces = true;
        Buffer * buffer = buffer_new();
        buffer_append_bytes(buffer,"00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F",48);
        Buffer * bin = elm_ascii_to_bin(elm,buffer);
        if ( bin->size != 16 ) return false;
        for(int i = 0; i < bin->size; i++) {
            if ( bin->buffer[i] != i ) return false;
        }
    }
    {
        SimELM327* elm327 = sim_elm327_new();
        LIST_SIM_ECU(elm327->ecus)[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
        sim_elm327_loop_as_daemon(elm327);
        sim_elm327_loop_daemon_wait_ready(elm327);
        final VehicleIFace* iface2 = port_open(strdup(elm327->device_location));
        ELM327Device * device = (ELM327Device*)iface2->device;
        if ( device->protocol < ELM327_PROTO_ISO_15765_4_CAN_1 ) {
            device->send(device, "attp 6");
            device->clear_data(device);
            device->recv(device);
        } else {
            device->send(device, "attp 2");
            device->clear_data(device);
            device->recv(device);
        }
        viface_send_str(iface2, "0100");
        viface_clear_data(iface2);
        viface_recv(iface2);
        assert(0 < iface2->vehicle->data_buffer->size);
    }
    return true;
}
