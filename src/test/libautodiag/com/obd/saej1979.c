#include "libTest.h"
#include "libautodiag/com/obd/saej1979/saej1979.h"

bool testSAEJ1979(VehicleIFace* iface) {
    {
        saej1979_data_is_pid_supported(iface, 0);
        saej1979_data_is_pid_supported(iface, 1);
    }
    {
        for(int i = 0; i < 5; i++) {
            double result = NAN;
            assert(viface_use_signal(iface, ad_signal_get("SAEJ1979.vehicle_speed"), &result, "01", null));
            tf_test_output("current vehicle speed: %.2f km/h", result);
            assert(!isnan(result));
        }
    }
    {
        double result = NAN;
        assert(viface_use_signal(iface, ad_signal_get("SAEJ1979.fuel_pressure"), &result, "01", null));
        tf_test_output("engine fuel pressure: %.2f kPa", result);
        assert(!isnan(result));
    }
    {
        double result = NAN;
        assert(viface_use_signal(iface, ad_signal_get("SAEJ1979.maf_air_flow_rate"), &result, "01", null));
        tf_test_output("current maf air flow rate: %.2f g/s", result);
        assert(!isnan(result));
    }
    {
        double result = NAN;
        assert(viface_use_signal(iface, ad_signal_get("SAEJ1979.throttle_position"), &result, "01", null));
        tf_test_output("current throttle position: %.2f %%", result);
        assert(!isnan(result));
    }
    {
        double result = NAN;
        assert(viface_use_signal(iface, ad_signal_get("SAEJ1979.coolant_temp"), &result, "01", null));
        tf_test_output("engine coolant temperature: %.2f °C", result);
        assert(!isnan(result));
    }
    {
        bool result = true;
        #define iterator(data) \
            result = false;

        viface_clear_data(iface);
        ad_buffer_append_str(((Serial *)iface->device)->recv_buffer,"\r7E804410C59C019\r>");
        
        iface->device->parse_data(iface->device,iface->vehicle);

        log_msg(LOG_DEBUG, "Storing in the corresponding service");
        Vehicle * v = iface->vehicle;
        if ( 0 < v->ecus->size ) {
            for(unsigned i = 0; i < v->ecus->size; i++) {
                ad_object_ECU * ecu = v->ecus->list[i];
                for(unsigned j = 0; j < ecu->data_buffer->size; j++) {
                    final Buffer * data = ecu->data_buffer->list[j];
                    if ( 0 < data->size ) {
                        final byte service_id = data->buffer[0];
                        if ( service_id == OBD_DIAGNOSTIC_SERVICE_NEGATIVE_RESPONSE ) {
                            log_msg(LOG_DEBUG, "Diagnostic Service negative code found (cannot response to the request)");
                            continue;
                        } else {
                            service_id &= ~OBD_DIAGNOSTIC_SERVICE_POSITIVE_RESPONSE;
                            final Buffer * data_copy = ad_buffer_copy(data);
                            ad_buffer_extract_0(data_copy);
                            switch(service_id) {
                                case OBD_SERVICE_SHOW_CURRENT_DATA: ad_list_Buffer_append(ecu->obd_service.current_data,data_copy);break;
                            }
                        }
                    }  else {
                        ad_list_Buffer_remove_at(ecu->data_buffer,j);
                        j--;
                    }
                }
            }
        }
        vehicle_fill_global_data_buffer_from_ecus(v);
        
        int pid = 0x42;
        OBD_ITERATE_ECUS_DATA_BUFFER_WITH_PID(ecu->obd_service.current_data,iterator,pid);
        assert(result);
    }
    return true;
}
