#include "libTest.h"

bool testCarDatabaseLoad(VehicleIFace* iface) {
    {
        Vehicle * vehicle = vehicle_new();
        vehicle->manufacturer = "Citroen";
        vehicle->engine_manufacturer = "Citroen";
        vehicle->engine = "";
        
        final SAEJ1979_DTC * dtc = saej1979_dtc_new();
        final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
        dtc_desc->vehicle = vehicle;
        
        dtc->data[0] = 0x11 | (ISO15031_DTC_TYPE_POWERTRAIN << 6);
        dtc->data[1] = 0x52;
        dtc->data[2] = 0x00;
        log_msg(LOG_DEBUG,"as str : %s", dtc->to_string(dtc));
    
        ad_dtc_fetch_from_db(dtc, vehicle);
        for(int i = 0; i < dtc->description->size; i++) {
            DTC_DESCRIPTION desc = dtc->description->list[i];
            log_msg(LOG_DEBUG, "reason: %s", desc.reason);
        }
        assert(0 < dtc->description->size);
    }
    {
        Vehicle * vehicle = vehicle_new();
        vehicle->manufacturer = "Citroen";
        vehicle->engine_manufacturer = "Citroen";
        vehicle->engine = "DV6TED4";
        ad_object_ECU * ecu = ad_object_ECU_new();
        ecu->model = strdup("EDC16C34");
        ecu->manufacturer = strdup("bosch");
        ad_list_ad_object_ECU_append(vehicle->ecus, ecu);
        final SAEJ1979_DTC * dtc = saej1979_dtc_new();
        final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
        dtc_desc->vehicle = vehicle;
        
        dtc->data[0] = 0x11 | (ISO15031_DTC_TYPE_POWERTRAIN << 6);
        dtc->data[1] = 0x52;
        dtc->data[2] = 0x00;
        log_info("as str : %s", dtc->to_string(dtc));
    
        ad_dtc_fetch_from_db(dtc, vehicle);
        for(int i = 0; i < dtc->description->size; i++) {
            DTC_DESCRIPTION desc = dtc->description->list[i];
            log_msg(LOG_INFO, "reason: %s", desc.reason);
        }
        assert(1 == dtc->description->size);
    }
    {
        Vehicle * vehicle = vehicle_new();
        vehicle->manufacturer = "Citroen";
        vehicle->engine_manufacturer = "Citroen";
        vehicle->engine = "EP6CDT";
        ad_object_ECU * ecu = ad_object_ECU_new();
        ecu->model = strdup("MED 17.4");
        ecu->manufacturer = strdup("bosch");
        ad_list_ad_object_ECU_append(vehicle->ecus, ecu);
        final SAEJ1979_DTC * dtc = saej1979_dtc_new();
        final DTC_DESCRIPTION * dtc_desc = dtc_description_new();
        dtc_desc->vehicle = vehicle;
        dtc->from_string(dtc, "P11A3");
        log_info("as str : %s", dtc->to_string(dtc));
    
        ad_dtc_fetch_from_db(dtc, vehicle);
        for(int i = 0; i < dtc->description->size; i++) {
            DTC_DESCRIPTION desc = dtc->description->list[i];
            log_msg(LOG_INFO, "reason: %s", desc.reason);
        }
        assert(1 == dtc->description->size);
    }
    return true;
}
