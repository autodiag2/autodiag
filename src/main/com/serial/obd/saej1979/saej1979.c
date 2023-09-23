#include "com/serial/obd/saej1979/saej1979.h"

char *saej1979_service_code_to_str(final unsigned char code) {
    switch(code) {
        case 0x01: return strdup("Show current data");
        case 0x02: return strdup("Show freeze frame data");
        case 0x03: return strdup("Show stored Diagnostic Trouble Codes");
        case 0x04: return strdup("Clear Diagnostic Trouble Codes and stored values");
        //case 0x05: return strdup("Test results, oxygen sensor monitoring"); // (non can only)
        //case 0x06: return strdup("Test results, other components/system monitoring (Test results, oxygen sensor monitoring for CAN only)");
        case 0x07: return strdup("Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)");
        case 0x08: return strdup("Control operation of on-board component/system");
        case 0x09: return strdup("Request vehicle information");
        case 0x0A: return strdup("Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)");
        default:
            return null;
    }
}
char *saej1979_pid_code_to_str(final unsigned char service, final unsigned char code) {
    switch(service) {
        case 0x01:
            switch(code) {
                case 0x00: return strdup("PIDs supported [0x01-0x20]");
                case 0x01: return strdup("Monitor status since DTCs cleared. (Includes malfunction indicator lamp (MIL), status and number of DTCs, components tests, DTC readiness checks)");
                case 0x02: return strdup("DTC that caused feeze frame to be stored");
                case 0x03: return strdup("Fuel system status");
                case 0x04: return strdup("Calculated engine load");
                case 0x05: return strdup("Engine coolant temperature");
                case 0x06: return strdup("Short term fuel trim - Bank 1");
                case 0x07: return strdup("Long term fuel trim - Bank 1");
                case 0x08: return strdup("Short term fuel trim - Bank 2");
                case 0x09: return strdup("Long term fuel trim - Bank 2");
                case 0x0A: return strdup("Fuel pressure (Gauge pressure)");
                case 0x0B: return strdup("Intake manifold absolute pressure");
                case 0x0C: return strdup("Engine speed");
                case 0x0D: return strdup("Vehicle speed");
                case 0x0E: return strdup("Timing advance");
                case 0x0F: return strdup("Intake air temperature");
                case 0x10: return strdup("Mass air flow sensor (MAF) air flow rate");
                case 0x11: return strdup("Throttle position");
                case 0x12: return strdup("Commanded secondary air status");
                case 0x13: return strdup("Oxygen sensors present (in 2 banks)");
                case 0x14: return strdup("Oxygen Sensor 1");
                case 0x15: return strdup("Oxygen Sensor 2");
                case 0x16: return strdup("Oxygen Sensor 3");
                case 0x17: return strdup("Oxygen Sensor 4");
                case 0x18: return strdup("Oxygen Sensor 5");
                case 0x19: return strdup("Oxygen Sensor 6");
                case 0x1A: return strdup("Oxygen Sensor 7");
                case 0x1B: return strdup("Oxygen Sensor 8");
                case 0x1C: return strdup("OBD standards this vehicle comforms to");
                case 0x1D: return strdup("Oxygen sensors present (in 4 banks)");
                case 0x1E: return strdup("Auxiliary input status");
                case 0x1F: return strdup("Run time since engine start");
                case 0x20: return strdup("PIDs supported [0x21-0x40]");
                case 0x21: return strdup("Distance traveled with malfunction indicator lamp (MIL) on");
                case 0x22: return strdup("Fuel Rail Pressure (relative to manifold vacuum)");
                case 0x23: return strdup("Fuel Rail Gauge Pressure (diesel, or gasoline direct injection)");
                case 0x24: return strdup("Oxygen Sensor 1");
                case 0x25: return strdup("Oxygen Sensor 2");
                case 0x26: return strdup("Oxygen Sensor 3");
                case 0x27: return strdup("Oxygen Sensor 4");
                case 0x28: return strdup("Oxygen Sensor 5");
                case 0x29: return strdup("Oxygen Sensor 6");
                case 0x2A: return strdup("Oxygen Sensor 7");
                case 0x2B: return strdup("Oxygen Sensor 8");
                case 0x2C: return strdup("Commanded EGR");
                case 0x2D: return strdup("EGR Error");
                case 0x2E: return strdup("Commanded evaporative purge");
                case 0x2F: return strdup("Fuel Tank Level Input");
                case 0x30: return strdup("Warm-ups since codes cleared");
                case 0x31: return strdup("Distance traveled since codes cleared");
                case 0x32: return strdup("Evap. System Vapor Pressure");
                case 0x33: return strdup("Absolute Barometric Pressure");
                case 0x34: return strdup("Oxygen Sensor 1");
                case 0x35: return strdup("Oxygen Sensor 2");
                case 0x36: return strdup("Oxygen Sensor 3");
                case 0x37: return strdup("Oxygen Sensor 4");
                case 0x38: return strdup("Oxygen Sensor 5");
                case 0x39: return strdup("Oxygen Sensor 6");
                case 0x3A: return strdup("Oxygen Sensor 7");
                case 0x3B: return strdup("Oxygen Sensor 8");
                case 0x3C: return strdup("Catalyst Temperature: Bank 1, Sensor 1");
                case 0x3D: return strdup("Catalyst Temperature: Bank 2, Sensor 1");
                case 0x3E: return strdup("Catalyst Temperature: Bank 1, Sensor 2");
                case 0x3F: return strdup("Catalyst Temperature: Bank 2, Sensor 2");
                case 0x40: return strdup("PIDs supported [0x41-0x60]");
                case 0x41: return strdup("Monitor status this drive cycle");
                case 0x42: return strdup("Control module voltage");
                case 0x43: return strdup("Absolute load value");
                case 0x44: return strdup("Commanded Air-Fuel Equivalence Ratio");
                case 0x45: return strdup("Relative throttle position");
                case 0x46: return strdup("Ambient air temperature");
                case 0x47: return strdup("Absolute throttle position B");
                case 0x48: return strdup("Absolute throttle position C");
                case 0x49: return strdup("Accelerator pedal position D");
                case 0x4A: return strdup("Accelerator pedal position E");
                case 0x4B: return strdup("Accelerator pedal position F");
                case 0x4C: return strdup("Commanded throttle actuator");
                case 0x4D: return strdup("Time run with MIL on");
                case 0x4E: return strdup("Time since trouble codes cleared");
            }
    }
    return null;
}
char *saej1979_decode_to_str(final unsigned char code[2]) {
    char * res;
    asprintf(&res,"%s%s",saej1979_service_code_to_str(code[0]),saej1979_pid_code_to_str(code[0], code[1]));
    return res;
}
void* saej1979_is_pid_supported_handler(final OBDIFACE iface, final int response, void *arg) {
    final int pid = *(int*)arg;
    final int pid_set_inc = 0x20;
    final int current_set = pid - (pid % pid_set_inc);
    final int pid_as_bitmask = 1 << (pid - current_set);
    bool *result = (bool*)malloc(sizeof(bool));
    *result = false;

    if ( 0 <= response ) {
        for( int i = 0; i < iface->obd_data_buffer->size && !result; i++) {
            Buffer * buffer = iface->obd_data_buffer->list[i];
            int buffer_as_32bits = 0;
            final unsigned char * ptr = (unsigned char*)&buffer_as_32bits;
            if ( O32_HOST_ORDER == O32_LITTLE_ENDIAN ) {
                memcpy(&buffer_as_32bits,buffer->buffer, 4);
            } else if ( O32_HOST_ORDER == O32_BIG_ENDIAN ) {
                for(int i = 0; i < 4; i++) {
                    *(ptr+i) = buffer->buffer[3 - i];
                }
            } else {
                unsupported_configuration();
            }
            for(int i = 0; i < 4; i ++) {
                ptr[i] = byte_reverse(ptr[i]);
            }
            *result |= (pid_as_bitmask&buffer_as_32bits) != 0;
        }
    } else {
        log_msg("Reception error", LOG_ERROR);
        *result = false;
    }
    return result;
}
bool saej1979_is_pid_supported(final OBDIFACE iface, int pid) {
    assert(0 <= pid);
    if ( 0 == pid ) {
        return true;
    } else {
        pid -= 1;
        final int pid_set_inc = 0x20;
        final int current_set = pid - (pid % pid_set_inc);
        char * request;
        final int service_id = 1;
        asprintf(&request,"%02x%02x", service_id, current_set);
        
        final bool result = *(bool*)obd_request(iface,saej1979_is_pid_supported_handler,&pid,request,false);
        free(request);
        return result;
    }
}

bool saej1979_get_current_dtc_status(final OBDIFACE iface) {
    bool result = true;
    obd_lock(iface);
    obd_serial_send_with_one_line_expected(iface, "0101");
    obd_clear_data(iface);
    if ( obd_recv_with_expected_databytes_start(iface,"0101") <= 0 ) {
        log_msg("Error during mil retrieve", LOG_ERROR);
        result = false;
    }
    obd_unlock(iface);
    return result;
}

bool saej1979_get_current_mil_status_from_fetch(final OBDIFACE iface) {
    bool result = false;
    for( int i = 0; i < iface->obd_data_buffer->size; i++) {
        if ( 0 < iface->obd_data_buffer->list[i]->size_used ) {
            result |= bitRetrieve(iface->obd_data_buffer->list[i]->buffer[0],7);
        }
    }
    return result;
}

void* saej1979_get_current_mil_status_handler(final OBDIFACE iface, final int response, void *arg) {
    bool * result = (bool*)malloc(sizeof(bool));
    if ( response <= 0 ) {
        log_msg("Error during mil retrieve", LOG_ERROR);
        *result = false;
    } else {
        *result = saej1979_get_current_mil_status_from_fetch(iface);
    }
    return result;
}

bool saej1979_get_current_mil_status(final OBDIFACE iface) {
    return obd_request_one_line_bool(iface, saej1979_get_current_mil_status_handler, "0101");
}

int saej1979_get_current_number_of_dtc_from_fetch(final OBDIFACE iface) {
    int result = 0;
    for( int i = 0; i < iface->obd_data_buffer->size; i++) {
        if ( 0 < iface->obd_data_buffer->list[i]->size_used ) {
            result += iface->obd_data_buffer->list[i]->buffer[0] & 0x7F;
        }
    }
    return result;
}

void* saej1979_get_current_number_of_dtc_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 ) {
        *result = SAEJ1979_NUMBER_OF_DTC_ERROR;
    } else {
        *result = saej1979_get_current_number_of_dtc_from_fetch(iface);
    }
    return (void*)result;
}

int saej1979_get_current_number_of_dtc(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_number_of_dtc_handler, "0101");
}

void* saej1979_get_current_engine_type_handler(final OBDIFACE iface, final int response, void *arg) {
    SAEJ1979_ENGINE_TYPES * result = (SAEJ1979_ENGINE_TYPES*)malloc(sizeof(SAEJ1979_ENGINE_TYPES));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_ENGINE_TYPE_UNKNOWN;
    } else {
        *result = bitRetrieve(iface->obd_data_buffer->list[0]->buffer[1],3) ? SAEJ1979_ENGINE_TYPE_COMPRESSION_IGNITION : SAEJ1979_ENGINE_TYPE_SPARK_IGNITION;
    }
    return (void*)result;
}

SAEJ1979_ENGINE_TYPES saej1979_get_current_engine_type(final OBDIFACE iface) {
    return *(SAEJ1979_ENGINE_TYPES*)obd_request_one_line(iface, saej1979_get_current_engine_type_handler, "0101");
}

char* saej1979_get_current_engine_type_as_string(final OBDIFACE iface) {
    return strdup(SAEJ1979_ENGINE_TYPES_STR[saej1979_get_current_engine_type(iface)]);
}

bool saej1979_freeze_dtc(final OBDIFACE iface) {
    obd_lock(iface);
    bool result = obd_serial_send_with_one_line_expected(iface, "0103") <= 0;
    elm327_send_wait_remote_confirmation(iface);
    obd_unlock(iface);
    return result;
}

void* saej1979_get_current_fuel_system_status_handler(final OBDIFACE iface, final int response, void *arg) {
    char*** result = (char***)malloc(sizeof(char**));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = null;
    } else {
        final char **strings = (char**)malloc(sizeof(char*) * 2);
        strings[0] = saej1979_fuel_system_status_code_to_str((int)iface->obd_data_buffer->list[0]->buffer[0]);
        strings[1] = saej1979_fuel_system_status_code_to_str((int)iface->obd_data_buffer->list[0]->buffer[1]);
        *result = strings;
    }
    return (void*)result;
}

char** saej1979_get_current_fuel_system_status(final OBDIFACE iface) {
    return *(char***)obd_request_one_line(iface, saej1979_get_current_fuel_system_status_handler, "0103");
}

char * saej1979_fuel_system_status_code_to_str(final int fuel_system_status_code) {
    switch(fuel_system_status_code) {
        case SAEJ1979_FUEL_SYSTEM_STATUS_MOTOR_OFF: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_MOTOR_OFF);
        case SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP);
        case SAEJ1979_FUEL_SYSTEM_STATUS_CLOSED_LOOP_USING_FEEDBACK: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_USING_FEEDBACK);
        case SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION);
        case SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_SYSTEM_FAILURE: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_SYSTEM_FAILURE);
        case SAEJ1979_FUEL_SYSTEM_STATUS_CLOSED_LOOP_WITH_FEEDBACK_FAULT: return strdup(SAEJ1979_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_WITH_FEEDBACK_FAULT);
        default:
            log_msg("Unknown fuel system status code", LOG_ERROR);
            return null;
    }
}

void* saej1979_get_current_engine_load_handler(final OBDIFACE iface, final int response, void *arg) {
    double *result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_ENGINE_LOAD_ERROR;
    } else {
        *result = ((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) / 2.55;
    }
    return (void*)result;
}

double saej1979_get_current_engine_load(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_engine_load_handler, "0104");
}

void* saej1979_get_current_engine_coolant_temperature_handler(final OBDIFACE iface, final int response, void *arg) {
    final int* result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_ENGINE_COOLANT_TEMPERATURE_ERROR;
    } else {
        *result = ((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) - 40;
    }
    return (void*)result;
}

int saej1979_get_current_engine_coolant_temperature(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_engine_coolant_temperature_handler, "0105");
}

void* saej1979_get_fuel_trim_bank_generic_handler(final OBDIFACE iface, final int response, void *arg) {
    final int* result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_FUEL_TRIM_ERROR;
    } else {
        *result = ((unsigned char)iface->obd_data_buffer->list[0]->buffer[0])/1.28 - 100;
    }
    return (void*)result;
}

int saej1979_get_fuel_trim_bank_generic(final OBDIFACE iface, final const char *cmd) {
    return obd_request_one_line_int(iface, saej1979_get_fuel_trim_bank_generic_handler, cmd);
}
int saej1979_get_current_short_term_fuel_trim_bank_1(final OBDIFACE iface) {
    return saej1979_get_fuel_trim_bank_generic(iface, "0106");
}
int saej1979_get_current_long_term_fuel_trim_bank_1(final OBDIFACE iface) {
    return saej1979_get_fuel_trim_bank_generic(iface, "0107");
}
int saej1979_get_current_short_term_fuel_trim_bank_2(final OBDIFACE iface) {
    return saej1979_get_fuel_trim_bank_generic(iface, "0108");
}
int saej1979_get_current_long_term_fuel_trim_bank_2(final OBDIFACE iface) {
    return saej1979_get_fuel_trim_bank_generic(iface, "0109");
}

void* saej1979_get_current_fuel_pressure_handler(final OBDIFACE iface, final int response, void *arg) {
    unsigned char * result = (unsigned char*)malloc(sizeof(unsigned char));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_FUEL_PRESSURE_ERROR;
    } else {
        *result = ((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) * 3;
    }
    return (void*)result;
}

int saej1979_get_current_fuel_pressure(final OBDIFACE iface) {
    return *(unsigned char*)obd_request_one_line(iface, saej1979_get_current_fuel_pressure_handler, "010A");
}

void* saej1979_get_current_intake_manifold_pressure_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_INTAKE_MANIFOLD_PRESSURE_ERROR;
    } else {
        *result = (unsigned char)iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}

int saej1979_get_current_intake_manifold_pressure(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_intake_manifold_pressure_handler, "010A");
}

void* saej1979_get_current_engine_speed_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_ENGINE_SPEED_ERROR;
    } else {
        int value = 256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1]);
        *result = value / 4.0;
    }
    return (void*)result;
}

double saej1979_get_current_engine_speed(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_engine_speed_handler, "010C");
}

void* saej1979_get_current_vehicle_speed_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_VEHICULE_SPEED_ERROR;
    } else {
        *result = (unsigned char)iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}

int saej1979_get_current_vehicle_speed(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_vehicle_speed_handler, "010D");
}

void* saej1979_get_current_timing_advance_cycle_1_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_TIMING_ADVANCE_CYCLE_1_ERROR;
    } else {
        *result = (((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) * 256 + ((unsigned char)iface->obd_data_buffer->list[0]->buffer[1])) / 2.0 - 64;
    }
    return (void*)result;
}

double saej1979_get_current_timing_advance_cycle_1(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_timing_advance_cycle_1_handler, "010E");
}

void* saej1979_get_current_intake_air_temperature_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_INTAKE_AIR_TEMPERATURE_ERROR;
    } else {
        *result = (unsigned char)iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}

int saej1979_get_current_intake_air_temperature(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_intake_air_temperature_handler, "010F");
}

void* saej1979_get_current_maf_air_flow_rate_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_VEHICULE_MAF_AIR_FLOW_RATE_ERROR;
    } else {
        *result = ((((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) * 256) + ((unsigned char)iface->obd_data_buffer->list[0]->buffer[1])) / 100.0;
    }
    return (void*)result;
}

double saej1979_get_current_maf_air_flow_rate(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_maf_air_flow_rate_handler, "0110");
}

void* saej1979_get_current_throttle_position_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_THROTTLE_POSITION_ERROR;
    } else {
        *result = ((unsigned char)iface->obd_data_buffer->list[0]->buffer[0]) / 2.55;
    }
    return (void*)result;
}

double saej1979_get_current_throttle_position(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_throttle_position_handler, "0111");
}

void* saej1979_get_current_secondary_air_status_handler(final OBDIFACE iface, final int response, void *arg) {
    SAEJ1979_SECONDARY_AIR_STATUS * result = (SAEJ1979_SECONDARY_AIR_STATUS*)malloc(sizeof(SAEJ1979_SECONDARY_AIR_STATUS));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_SECONDARY_AIR_STATUS_ERROR;
    } else {
        *result = iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}

SAEJ1979_SECONDARY_AIR_STATUS saej1979_get_current_secondary_air_status(final OBDIFACE iface) {
    return *(SAEJ1979_SECONDARY_AIR_STATUS*)obd_request_one_line(iface, saej1979_get_current_secondary_air_status_handler, "0112");
}

char* saej1979_current_oxygen_sensors_present_to_string(byte b) {
    char * msg;
    asprintf(&msg, "Oxygen Sensor 1: %s, 2: %s, 3: %s, 4: %s", 
        bitRetrieve(b,3)?"present":"absent",
        bitRetrieve(b,2)?"present":"absent",
        bitRetrieve(b,1)?"present":"absent",
        bitRetrieve(b,0)?"present":"absent"
    );
    return msg;
}
byte saej1979_current_oxygen_sensors_present_bank1(final OBDIFACE iface) {
    return 0xF & saej1979_current_oxygen_sensors_present_generic(iface);
}
byte saej1979_current_oxygen_sensors_present_bank2(final OBDIFACE iface) {
    return (0xF0 & saej1979_current_oxygen_sensors_present_generic(iface)) >> 4;
}
void* saej1979_current_oxygen_sensors_present_generic_handler(final OBDIFACE iface, final int response, void *arg) {
    byte * result = (byte*)malloc(sizeof(byte));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_SECONDARY_AIR_STATUS_ERROR;
    } else {
        *result = iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}
byte saej1979_current_oxygen_sensors_present_generic(final OBDIFACE iface) {
    return *(byte*)obd_request_one_line(iface, saej1979_current_oxygen_sensors_present_generic_handler, "0113");
}
char * saej1979_secondary_air_status_to_string(SAEJ1979_SECONDARY_AIR_STATUS data) {
    switch(data) {
        case SAEJ1979_SECONDARY_AIR_STATUS_UPSTREAM: return strdup("Upstream");
        case SAEJ1979_SECONDARY_AIR_STATUS_DOWNSTREAM: return strdup("Downstream of catalytic converter");
        case SAEJ1979_SECONDARY_AIR_STATUS_OUTSIDE_OR_OFF: return strdup("From the outside atmosphere or off");
        case SAEJ1979_SECONDARY_AIR_STATUS_PUMP: return strdup("Pump commanded on for diagnostics");
        default:
            return null;
    }
}
void* saej1979_get_current_obd_standard_handler(final OBDIFACE iface, final int response, void *arg) {
    SAEJ1979_OBD_STANDARD * result = (SAEJ1979_OBD_STANDARD*)malloc(sizeof(SAEJ1979_OBD_STANDARD));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_OBD_STANDARD_OBD_UNKNOWN;
    } else {
        *result = (SAEJ1979_OBD_STANDARD)iface->obd_data_buffer->list[0]->buffer[0];
    }
    return (void*)result;
}
SAEJ1979_OBD_STANDARD saej1979_get_current_obd_standard(final OBDIFACE iface) {
    return *(SAEJ1979_OBD_STANDARD*)obd_request_one_line(iface, saej1979_current_oxygen_sensors_present_generic_handler, "011C");
}

char * saej1979_get_current_obd_standard_str(final OBDIFACE iface) {
    switch(saej1979_get_current_obd_standard(iface)) {
        case SAEJ1979_OBD_STANDARD_OBD_II_CARB:                 return strdup("OBD-II as defined by CARB");
        case SAEJ1979_OBD_STANDARD_OBD_EPA:                     return strdup("OBD as defined by EPA");
        case SAEJ1979_OBD_STANDARD_OBD_AND_OBD_II:              return strdup("OBD and OBD-II");
        case SAEJ1979_OBD_STANDARD_OBD_I:                       return strdup("OBD-I");
        case SAEJ1979_OBD_STANDARD_NOT_OBD:                     return strdup("Not OBD compliant");
        case SAEJ1979_OBD_STANDARD_EOBD:                        return strdup("EOBD (Europe)");
        case SAEJ1979_OBD_STANDARD_EOBD_AND_OBD_II:             return strdup("EOBD and OBD-II");
        case SAEJ1979_OBD_STANDARD_EOBD_AND_OBD:                return strdup("EOBD and OBD");
        case SAEJ1979_OBD_STANDARD_EOBD_AND_OBD_AND_OBD_II:     return strdup("EOBD, OBD and OBD-II");
        case SAEJ1979_OBD_STANDARD_JOBD:                        return strdup("JOBD (Japan)");
        case SAEJ1979_OBD_STANDARD_JOBD_AND_OBD_II:             return strdup("JOBD and OBD II");
        case SAEJ1979_OBD_STANDARD_JOBD_AND_EOBD:               return strdup("JOBD and EOBD");
        case SAEJ1979_OBD_STANDARD_JOBD_AND_EOBD_AND_OBD_II:    return strdup("JOBD, EOBD, OBD II");
        case SAEJ1979_OBD_STANDARD_EMD:                         return strdup("Engine Manufacturer Diagnostics (EMD)");
        case SAEJ1979_OBD_STANDARD_EMD_ENHANCED:                return strdup("Engine Manufacturer Diagnostics Enhanced (EMD+)");
        case SAEJ1979_OBD_STANDARD_HEAVY_DUTY_OBD_C:            return strdup("Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)");
        case SAEJ1979_OBD_STANDARD_HEAVY_DUTY_OBD:              return strdup("Heavy Duty On-Board Diagnostics (HD OBD)");
        case SAEJ1979_OBD_STANDARD_WWH_OBD:                     return strdup("World Wide Harmonized OBD (WWH OBD)");
        case SAEJ1979_OBD_STANDARD_HD_EOBD:                     return strdup("Heavy Duty Euro Stage I without NOx control (HD EOBD-I)");
        case SAEJ1979_OBD_STANDARD_HD_EOBD_N:                   return strdup("Heavy Duty Euro OBD Start I with NOx control (HD EOBD-I N)");
        case SAEJ1979_OBD_STANDARD_HD_EOBD_II:                  return strdup("Heavy Duty Euro Stage II without NOx control (HD EOBD-II)");
        case SAEJ1979_OBD_STANDARD_HD_EOBD_II_N:                return strdup("Heavy Duty Euro OBD Start II with NOx control (HD EOBD-II N)");
        case SAEJ1979_OBD_STANDARD_OBDBR_1:                     return strdup("Brazil OBD Phase 1 (OBDBr-1)");
        case SAEJ1979_OBD_STANDARD_OBDBR_2:                     return strdup("Brazil OBD Phase 2 (OBDBr-2)");
        case SAEJ1979_OBD_STANDARD_KOBD:                        return strdup("Korean OBD (KOBD)");
        case SAEJ1979_OBD_STANDARD_IOBD:                        return strdup("India OBD I (IOBD I)");
        case SAEJ1979_OBD_STANDARD_IOBD_II:                     return strdup("India OBD II (IOBD II)");
        case SAEJ1979_OBD_STANDARD_HD_EOBD_VI:                  return strdup("Heavy Duty Euro OBD Stage VI (HD EOBD-IV)");
    }
    return null;
}

char* saej1979_get_seconds_to_time(final int seconds) {
    final int hours = seconds/3600;
    final int minutes = (seconds % 3600)/60;
    final int secs = seconds % 60;
    char * res;
    if ( 0 < hours ) {
        asprintf(&res, "%d hours %d minutes %d seconds", hours, minutes, secs);
    } else {
        if ( 0 < minutes ) {
            asprintf(&res, "%d minutes %d seconds", minutes, secs);
        } else {
            asprintf(&res, "%d seconds", secs);
        }
    }
    return res;
}

char* saej1979_get_current_time_since_engine_start(final OBDIFACE iface) {
    final int seconds = saej1979_get_current_seconds_since_engine_start(iface);
    return seconds == -1 ? null : saej1979_get_seconds_to_time(seconds);
}
void* saej1979_get_current_seconds_since_engine_start_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_SECONDS_SINCE_ENGINE_START_ERROR;
    } else {
        *result = 256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1]);
    }
    return (void*)result;
}
int saej1979_get_current_seconds_since_engine_start(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_seconds_since_engine_start_handler, "011F");
}
void* saej1979_get_current_distance_since_mil_activated_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_DISTANCE_SINCE_MIL_ACTIVATED_ERROR;
    } else {
        *result = 256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1]);
    }
    return (void*)result;
}
int saej1979_get_current_distance_since_mil_activated(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_seconds_since_engine_start_handler, "0121");
}
void* saej1979_get_current_frp_relative_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_FRP_RELATIVE;
    } else {
        *result = (256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1])) * 0.079;
    }
    return (void*)result;
}
double saej1979_get_current_frp_relative(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_frp_relative_handler, "0122");
}
void* saej1979_get_current_frp_widerange_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_FRP_WIDERANGE_ERROR;
    } else {
        *result = (256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1])) * 10;
    }
    return (void*)result;
}
int saej1979_get_current_frp_widerange(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_frp_widerange_handler, "0123");
}
void* saej1979_get_current_commanded_egr_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_COMMANDED_EGR_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) * (100.0/255);
    }
    return (void*)result;
}
double saej1979_get_current_commanded_egr(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_commanded_egr_handler, "012C");
}
void* saej1979_get_current_egr_error_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_EGR_ERROR_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) * (100.0/128) - 100;
    }
    return (void*)result;
}
double saej1979_get_current_egr_error(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_egr_error_handler, "012D");
}
void* saej1979_get_current_commanded_evap_purge_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_COMMANDED_EVAP_PURGE_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) * (100.0/255);
    }
    return (void*)result;
}
double saej1979_get_current_commanded_evap_purge(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_egr_error_handler, "012E");
}
void* saej1979_get_current_fuel_tank_level_input_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_FUEL_TANK_LEVEL_INPUT_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) * (100.0/255);
    }
    return (void*)result;
}
double saej1979_get_current_fuel_tank_level_input(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_egr_error_handler, "012F");
}
void* saej1979_get_current_warm_ups_since_ecu_reset_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_WARM_UPS_SINCE_ECU_RESET_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]);
    }
    return (void*)result;
}
int saej1979_get_current_warm_ups_since_ecu_reset(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_egr_error_handler, "0130");
}
void* saej1979_get_current_distance_since_ecu_reset_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_DISTANCE_SINCE_ECU_RESET_ERROR;
    } else {
        *result = (256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1]));
    }
    return (void*)result;
}
int saej1979_get_current_distance_since_ecu_reset(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_distance_since_ecu_reset_handler, "0131");
}
void* saej1979_get_current_evap_system_vapor_pressure_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR;
    } else {
        *result = (256 * (int)(iface->obd_data_buffer->list[0]->buffer[0]) + (int)(iface->obd_data_buffer->list[0]->buffer[1])) * 0.25;
    }
    return (void*)result;
}
double saej1979_get_current_evap_system_vapor_pressure(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_evap_system_vapor_pressure_handler, "0132");
}
void* saej1979_get_current_baro_pressure_absolute_handler(final OBDIFACE iface, final int response, void *arg) {
    int * result = (int*)malloc(sizeof(int));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 0 ) {
        *result = SAEJ1979_BARO_PRESSURE_ABSOLUTE_ERROR;
    } else {
        *result = (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]);
    }
    return (void*)result;
}
int saej1979_get_current_baro_pressure_absolute(final OBDIFACE iface) {
    return obd_request_one_line_int(iface, saej1979_get_current_baro_pressure_absolute_handler, "0133");
}
void* saej1979_get_current_ecu_voltage_handler(final OBDIFACE iface, final int response, void *arg) {
    double * result = (double*)malloc(sizeof(double));
    if ( response <= 0 || iface->obd_data_buffer->list[0]->size_used <= 1 ) {
        *result = SAEJ1979_ECU_VOLTAGE_ERROR;
    } else {
        *result = (256 * (unsigned char)(iface->obd_data_buffer->list[0]->buffer[0]) + (unsigned char)(iface->obd_data_buffer->list[0]->buffer[1])) * 0.001;
    }
    return (void*)result;
}
double saej1979_get_current_ecu_voltage(final OBDIFACE iface) {
    return obd_request_one_line_double(iface, saej1979_get_current_ecu_voltage_handler, "0142");
}

bool saej1979_clear_dtc_and_stored_values(final OBDIFACE iface) {
    obd_lock(iface);
    obd_send(iface, "04");
    obd_unlock(iface);
    return true;
}
