#include "libTest.h"

/**
 * SAEJ1979 allow multiple PIDs to be sent if generator->flavour.is_Iso15765_4
 */
void testMultiplePIDsPerFrame() {
    SimELM327* elm327 = tf_sim_elm327_new();
    LIST_SIM_ECU(elm327->ecus)[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = tf_serial_open(strdup(elm327->device_location));
    {
        log_info("Ensure the non flavoured generator reply only to one PID");
        iface->lock(iface);
        iface->device->send(iface->device, gprintf("atsp %x", ELM327_PROTO_ISO_14230_4_KWP2000_2));
        iface->device->recv(iface->device);
        elm327->protocolRunning = ELM327_PROTO_ISO_14230_4_KWP2000_2;
        ELM327Device * device = (ELM327Device *)iface->device;
        device->protocol = ELM327_PROTO_ISO_14230_4_KWP2000_2;
        iface->unlock(iface);
        iface->send(iface, ad_buffer_from_ascii_hex("010100"));
        iface->clear_data(iface);
        iface->recv(iface);
        assert(iface->vehicle->data_buffer->size == 1);
        Buffer * data_buffer = iface->vehicle->data_buffer->list[0];
        assert(data_buffer->buffer[0] == 0x41);
        assert(data_buffer->buffer[1] == 0x01);
        assert(data_buffer->size == 2 + 4);
    }
    {
        log_info("Ensure the flavoured generator reply to many PIDs");
        iface->lock(iface);
        iface->device->send(iface->device, gprintf("atsp %x", ELM327_PROTO_ISO_15765_4_CAN_1));
        iface->device->recv(iface->device);
        elm327->protocolRunning = ELM327_PROTO_ISO_15765_4_CAN_1;
        ELM327Device * device = (ELM327Device *)iface->device;
        device->protocol = ELM327_PROTO_ISO_15765_4_CAN_1;
        iface->unlock(iface);
        iface->send(iface, ad_buffer_from_ascii_hex("010100"));
        iface->clear_data(iface);
        iface->recv(iface);
        assert(iface->vehicle->data_buffer->size == 1);
        Buffer * data_buffer = iface->vehicle->data_buffer->list[0];
        assert(data_buffer->buffer[0] == 0x41);
        assert(data_buffer->buffer[1] == 0x01);
        assert(data_buffer->size > 2 + 4);
    }
}
bool testSimUDS() {
    testMultiplePIDsPerFrame();
    SimELM327* elm327 = tf_sim_elm327_new();
    LIST_SIM_ECU(elm327->ecus)[0].list[0]->generator = sim_ecu_generator_new_citroen_c5_x7();
    sim_elm327_loop_as_daemon(elm327);
    sim_elm327_loop_daemon_wait_ready(elm327);
    final VehicleIFace* iface = tf_serial_open(strdup(elm327->device_location));
    {
        viface_send(iface, ad_buffer_from_ints(
            UDS_SERVICE_READ_DATA_BY_IDENTIFIER,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information >> 8,
        ));
        viface_clear_data(iface);
        viface_recv(iface);
        bool passed = false;
        for(int i = 0; i < iface->vehicle->ecus->size; i++) {
            final ad_object_ECU * ecu = iface->vehicle->ecus->list[i];
            for(int j = 0; j < ecu->data_buffer->size; j++) {
                final Buffer * data = ecu->data_buffer->list[j];
                assert(data->buffer[0] == UDS_NEGATIVE_RESPONSE);
                assert(data->buffer[1] == UDS_SERVICE_READ_DATA_BY_IDENTIFIER);
                assert(data->buffer[2] == UDS_NRC_IncorrectMessageLengthOrInvalidFormat);
                passed = true;
            }
        }
        assert(passed);
    }
    {
        assert(uds_request_session_cond(iface, UDS_SESSION_EXTENDED_DIAGNOSTIC));
        ad_list_Buffer *result = uds_read_data_by_identifier(iface,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 1);
        assert(result->list[0]->buffer[0] == UDS_SESSION_EXTENDED_DIAGNOSTIC);
    }
    {
        ad_list_ad_list_UDS_DTC * result = uds_read_dtc_first_confirmed_dtc(iface, null);
        assert(result->size == 1);
        ad_list_UDS_DTC * result_per_ecu = result->list[0];
        assert(result_per_ecu->size == 2);
        UDS_DTC * dtc = result_per_ecu->list[0];
        assert(strcmp(dtc->to_string(dtc), "P0103") == 0);
        dtc = result_per_ecu->list[1];
        assert(strcmp(dtc->to_string(dtc), "P0104") == 0);
    }
    {
        assert(uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING));
        uds_viface_stop_tester_present_timer(iface);
        ad_list_Buffer *result = uds_read_data_by_identifier(iface,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 1);
        assert(result->list[0]->buffer[0] == UDS_SESSION_PROGRAMMING);
        sleep(UDS_SESSION_TIMEOUT_MS/1000 - 1);
        assert(uds_tester_present(iface, true));
        sleep((UDS_SESSION_TIMEOUT_MS/1000) * 2);
        result = uds_read_data_by_identifier(iface,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 1);
        assert(result->list[0]->buffer[0] == UDS_SESSION_DEFAULT);
    }
    {
        assert(uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING));
        ad_list_Buffer *result = uds_read_data_by_identifier(iface,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 1);
        assert(result->list[0]->buffer[0] == UDS_SESSION_PROGRAMMING);
        sleep((UDS_SESSION_TIMEOUT_MS/1000) * 2);
        result = uds_read_data_by_identifier(iface,
            UDS_DID_Active_Diagnostic_Session_Data_Identifier_information
        );
        assert(result->size == 1);
        assert(result->list[0]->size == 1);
        assert(result->list[0]->buffer[0] == UDS_SESSION_PROGRAMMING);
    }
    {
        assert(uds_request_session_cond(iface, UDS_SESSION_DEFAULT));
        assert(!uds_security_access_ecu_generator_citroen_c5_x7(iface));
        assert(uds_request_session_cond(iface, UDS_SESSION_PROGRAMMING));
        assert(uds_security_access_ecu_generator_citroen_c5_x7(iface));
    }
    {
        Buffer * vin = ad_buffer_from_ascii("5UXCR6C00N9K68159");
        assert(uds_write_vin(iface, vin));
        if ( iface->uds.enabled && iface->vehicle->vin->size == 0 ) {
            final ad_list_Buffer * result = uds_read_data_by_identifier(iface, UDS_DID_VIN);
            if ( 0 == result->size ) {
                log_err("VIN not received");
            } else if ( 1 == result->size ) {
                iface->vehicle->vin = ad_buffer_copy(result->list[0]);
            } else {
                log_warn("More than on vin received, selecting the first one");
                iface->vehicle->vin = ad_buffer_copy(result->list[0]);
            }
        }
        assert(ad_buffer_cmp(iface->vehicle->vin, vin) == 0);
    }
    return true;
}