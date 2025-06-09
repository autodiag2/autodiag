#include "libTest.h"
#include "libautodiag/com/obd/saej1979/saej1979.h"

bool testSAEJ1979(OBDIFace* iface) {
    {
        saej1979_data_is_pid_supported(iface, false, 0);
        saej1979_data_is_pid_supported(iface, false, 1);
    }
    {
        for(int i = 0; i < 5; i++) {
            int result = saej1979_data_vehicle_speed(iface, false);
            testOutput("current vehicle speed: %d km/h", result);
            assert(SAEJ1979_DATA_VEHICULE_SPEED_ERROR != result);
        }
    }
    {
        int result = saej1979_data_fuel_pressure(iface, false);
        testOutput("engine fuel pressure: %d kPa", result);
        assert(SAEJ1979_DATA_FUEL_PRESSURE_ERROR != result);
    }
    {
        double result = saej1979_data_maf_air_flow_rate(iface, false);
        testOutput("current maf air flow rate: %.2f g/s", result);
        assert(SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_ERROR != result);
    }
    {
        double result = saej1979_data_throttle_position(iface, false);
        testOutput("current throttle position: %.2f", result);
        assert(SAEJ1979_DATA_THROTTLE_POSITION_ERROR != result);
    }
    {
        int result = saej1979_data_engine_coolant_temperature(iface, false);
        testOutput("engine coolant temperature: %d °C", result);
        assert(result != SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_ERROR);
    }
    {
        bool result = true;
        #define iterator(data) \
            result = false;

        obd_clear_data(iface);
        buffer_append_str(((SERIAL)iface->device)->recv_buffer,"\r7E804410C59C019\r>");
        
        iface->device->parse_data(iface->device,iface->vehicle);

        log_msg(LOG_DEBUG, "Storing in the corresponding service");
        VEHICLE v = iface->vehicle;
        if ( 0 < v->ecus_len ) {
            for(int i = 0; i < v->ecus_len; i++) {
                ECU * ecu = v->ecus[i];
                for(int j = 0; j < ecu->obd_data_buffer->size; j++) {
                    final Buffer * data = ecu->obd_data_buffer->list[j];
                    if ( 0 < data->size ) {
                        final byte service_id = data->buffer[0];
                        if ( service_id == OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE ) {
                            log_msg(LOG_DEBUG, "Diagnostic Service negative code found (cannot response to the request)");
                            continue;
                        } else {
                            service_id &= ~OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE;
                            final Buffer * data_copy = buffer_copy(data);
                            buffer_extract_0(data_copy);
                            switch(service_id) {
                                case OBD_SERVICE_SHOW_CURRENT_DATA: BufferList_append(ecu->obd_service.current_data,data_copy);break;
                            }
                        }
                    }  else {
                        BufferList_remove_at(ecu->obd_data_buffer,j);
                        j--;
                    }
                }
            }
        }
        vehicle_fill_global_obd_data_from_ecus(v);
        
        int pid = 0x42;
        OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.current_data,iterator,pid);
        assert(result);
    }
    return true;
}
