#include "libTest.h"

bool testSimUDS() {
    SimELM327* elm327 = sim_elm327_new();
    elm327->ecus[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = port_open(strdup(elm327->device_location));
    {
        viface_send(iface, buffer_to_hex_string(buffer_from_ints(
            UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            UDS_SERVICE_READ_DATA_BY_IDENTIFIER_DID_Active_Diagnostic_Session_Data_Identifier_information >> 8,
        )));
        viface_clear_data(iface);
        viface_recv(iface);
        bool passed = false;
        for(int i = 0; i < iface->vehicle->ecus_len; i++) {
            final ECU * ecu = iface->vehicle->ecus[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * data = ecu->data_buffer->list[j];
                assert(data->buffer[0] == UDS_NEGATIVE_RESPONSE);
                assert(data->buffer[1] == UDS_SERVICE_READ_DATA_BY_IDENTIFIER);
                assert(data->buffer[2] == UDS_NRC_INVALID_MESSAGE_LENGTH);
                passed = true;
            }
        }
        assert(passed);
    }
    {
        list_Buffer *result = uds_read_data_by_identifier(iface,
            UDS_SERVICE_READ_DATA_BY_IDENTIFIER_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 2);
    }
}