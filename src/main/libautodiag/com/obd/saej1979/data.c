#include "libautodiag/com/obd/saej1979/data.h"

/**
 * Actual data extracted from wikipedia OBDII PIDs page
 */
char *saej1979_data_pid_code_to_str(final unsigned char pid) {
    switch(pid) {
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
        case 0x4F: return strdup("Maximum value for Fuel–Air equivalence ratio, oxygen sensor voltage, oxygen sensor current, and intake manifold absolute pressure");
        case 0x50: return strdup("Maximum value for air flow rate from mass air flow sensor");
        case 0x51: return strdup("Fuel Type");
        case 0x52: return strdup("Ethanol fuel %");
        case 0x53: return strdup("Absolute Evap system Vapor Pressure");
        case 0x54: return strdup("Evap system vapor pressure");
        case 0x55: return strdup("Short term secondary oxygen sensor trim, A: bank 1, B: bank 3");
        case 0x56: return strdup("Long term secondary oxygen sensor trim, A: bank 1, B: bank ");
        case 0x57: return strdup("Short term secondary oxygen sensor trim, A: bank 2, B: bank ");
        case 0x58: return strdup("Long term secondary oxygen sensor trim, A: bank 2, B: bank ");
        case 0x59: return strdup("Fuel rail absolute pressure");
        case 0x5A: return strdup("Relative accelerator pedal position");
        case 0x5B: return strdup("Hybrid battery pack remaining life");
        case 0x5C: return strdup("Engine oil temperature");
        case 0x5D: return strdup("Fuel injection timing");
        case 0x5E: return strdup("Engine fuel rate");
        case 0x5F: return strdup("Emission requirements to which vehicle is designed");
        case 0x60: return strdup("PIDs supported [61 - 80]");
        case 0x61: return strdup("Driver's demand engine - percent torque");
        case 0x62: return strdup("Actual engine - percent torque");
        case 0x63: return strdup("Engine reference torque");
        case 0x64: return strdup("Engine percent torque data");
        case 0x65: return strdup("Auxiliary input / output supported");
        case 0x66: return strdup("Mass air flow sensor");
        case 0x67: return strdup("Engine coolant temperature");
        case 0x68: return strdup("Intake air temperature sensor");
        case 0x69: return strdup("Actual EGR, Commanded EGR, and EGR Error");
        case 0x6A: return strdup("Commanded Diesel intake air flow control and relative intake air flow position");
        case 0x6B: return strdup("Exhaust gas recirculation temperature");
        case 0x6C: return strdup("Commanded throttle actuator control and relative throttle position");
        case 0x6D: return strdup("Fuel pressure control system");
        case 0x6E: return strdup("Injection pressure control system");
        case 0x6F: return strdup("Turbocharger compressor inlet pressure");
        case 0x70: return strdup("Boost pressure control");
        case 0x71: return strdup("Variable Geometry turbo (VGT) control");
        case 0x72: return strdup("Wastegate control");
        case 0x73: return strdup("Exhaust pressure");
        case 0x74: return strdup("Turbocharger RPM");
        case 0x75: return strdup("Turbocharger temperature");
        case 0x76: return strdup("Turbocharger temperature");
        case 0x77: return strdup("Charge air cooler temperature (CACT)");
        case 0x78: return strdup("Exhaust Gas temperature (EGT) Bank 1");
        case 0x79: return strdup("Exhaust Gas temperature (EGT) Bank 2");
        case 0x7A: return strdup("Diesel particulate filter (DPF) differential pressur");
        case 0x7B: return strdup("Diesel particulate filter (DPF)");
        case 0x7C: return strdup("Diesel Particulate filter (DPF) temperature");
        case 0x7D: return strdup("NOx NTE (Not-To-Exceed) control area status");
        case 0x7E: return strdup("PM NTE (Not-To-Exceed) control area status");
        case 0x7F: return strdup("Engine run time ");
        case 0x80: return strdup("PIDs supported [81 - A0]");
        case 0x81: return strdup("Engine run time for Auxiliary Emissions Control Device(AECD)");
        case 0x82: return strdup("Engine run time for Auxiliary Emissions Control Device(AECD)");
        case 0x83: return strdup("NOx sensor");
        case 0x84: return strdup("Manifold surface temperature");
        case 0x85: return strdup("NOx reagent system");
        case 0x86: return strdup("Particulate matter (PM) sensor");
        case 0x87: return strdup("Intake manifold absolute pressure");
        case 0x88: return strdup("SCR Induce System");
        case 0x89: return strdup("Run Time for AECD #11-#15");
        case 0x8A: return strdup("Run Time for AECD #16-#20");
        case 0x8B: return strdup("Diesel Aftertreatment");
        case 0x8C: return strdup("O2 Sensor (Wide Range)");
        case 0x8D: return strdup("Throttle Position G");
        case 0x8E: return strdup("Engine Friction - Percent Torque");
        case 0x8F: return strdup("PM Sensor Bank 1 & 2");
        case 0x90: return strdup("WWH-OBD Vehicle OBD System Information");
        case 0x91: return strdup("WWH-OBD Vehicle OBD System Information");
        case 0x92: return strdup("Fuel System Control");
        case 0x93: return strdup("WWH-OBD Vehicle OBD Counters support");
        case 0x94: return strdup("NOx Warning And Inducement System");
        case 0x98: return strdup("Exhaust Gas Temperature Sensor");
        case 0x99: return strdup("Exhaust Gas Temperature Sensor");
        case 0x9A: return strdup("Hybrid/EV Vehicle System Data, Battery, Voltage");
        case 0x9B: return strdup("Diesel Exhaust Fluid Sensor Data");
        case 0x9C: return strdup("O2 Sensor Data");
        case 0x9D: return strdup("Engine Fuel Rate");
        case 0x9E: return strdup("Engine Exhaust Flow Rate");
        case 0x9F: return strdup("Fuel System Percentage Use");
        case 0xA0: return strdup("PIDs supported [A1 - C0]");
        case 0xA1: return strdup("NOx Sensor Corrected Data");
        case 0xA2: return strdup("Cylinder Fuel Rate");
        case 0xA3: return strdup("Evap System Vapor Pressure");
        case 0xA4: return strdup("Transmission Actual Gear");
        case 0xA5: return strdup("Commanded Diesel Exhaust Fluid Dosing");
        case 0xA6: return strdup("Odometer ");
        case 0xA7: return strdup("NOx Sensor Concentration Sensors 3 and 4");
        case 0xA8: return strdup("NOx Sensor Corrected Concentration Sensors 3 and 4");
        case 0xA9: return strdup("ABS Disable Switch State");
        case 0xC0: return strdup("PIDs supported [C1 - E0]");    
    }
    return null;
}

char * saej1979_data_fuel_system_status_code_to_str(final int fuel_system_status_code) {
    switch(fuel_system_status_code) {
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_MOTOR_OFF: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_MOTOR_OFF);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_USING_FEEDBACK: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_USING_FEEDBACK);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_SYSTEM_FAILURE: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_SYSTEM_FAILURE);
        case SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_WITH_FEEDBACK_FAULT: return strdup(SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_WITH_FEEDBACK_FAULT);
        default:
            log_msg(LOG_ERROR, "Unknown fuel system status code");
            return null;
    }
}

char * saej1979_data_secondary_air_status_to_string(SAEJ1979_DATA_SECONDARY_AIR_STATUS data) {
    switch(data) {
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_UPSTREAM: return strdup("Upstream");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_DOWNSTREAM: return strdup("Downstream of catalytic converter");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_OUTSIDE_OR_OFF: return strdup("From the outside atmosphere or off");
        case SAEJ1979_DATA_SECONDARY_AIR_STATUS_PUMP: return strdup("Pump commanded on for diagnostics");
        default:
            return null;
    }
}

unsigned short saej1979_data_buffer_get_short(final Buffer* buffer) {
    assert(1 < buffer->size);
    return (unsigned short)buffer->buffer[0] * 256 + buffer->buffer[1];
} 

unsigned int saej1979_data_buffer_get_uint(final Buffer* buffer) {
    assert(3 < buffer->size);
    return (buffer->buffer[0] >> 24) + (buffer->buffer[1] >> 16) + (buffer->buffer[2] >> 8) + buffer->buffer[3];
}

bool saej1979_data_is_pid_supported(final OBDIFace* iface, bool useFreezedData, int pid) {
    return saej1979_is_pid_supported(iface,1 + useFreezedData, 0x20, pid);
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_number_of_dtc,
                        "01",
                        saej1979_data_number_of_dtc_iterator,
                        SAEJ1979_DATA_NUMBER_OF_DTC_ERROR
                    )


SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_ENGINE_TYPES,
                        saej1979_data_engine_type,
                        "01",
                        saej1979_data_engine_type_iterator,
                        SAEJ1979_DATA_ENGINE_TYPE_UNKNOWN
                    )
char* saej1979_data_engine_type_as_string(final OBDIFace* iface, bool useFreezedData) {
    return strdup(SAEJ1979_DATA_ENGINE_TYPES_STR[saej1979_data_engine_type(iface,useFreezedData)]);
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        bool,
                        saej1979_data_mil_status,
                        "01",
                        saej1979_data_mil_status_iterator,
                        false
                    )

#define saej1979_data_status_iterator(data) \
    if ( 3 < data->size ) result = data->buffer;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte*,
                        saej1979_data_status,
                        "01",
                        saej1979_data_status_iterator,
                        null
                    )

LIST_SRC(SAEJ1979_DATA_Test)

list_SAEJ1979_DATA_Test *saej1979_data_tests_generic(final OBDIFace* iface, bool useFreezedData, byte* (*data_accessor)(final OBDIFace* iface, bool useFreezedData)) {
    list_SAEJ1979_DATA_Test *list = list_SAEJ1979_DATA_Test_new();

    byte * buffer = data_accessor(iface,useFreezedData);
    if ( buffer != null ) {
        byte B = buffer[1];
        char *names1[100] = {"Misfire","Fuel System", "Components"};
        for (int i = 0; i < 3; i++) {
            if ( bitRetrieve(B, i) ) {
                SAEJ1979_DATA_Test * data = (SAEJ1979_DATA_Test*)malloc(sizeof(SAEJ1979_DATA_Test));
                data->name = strdup(names1[i]);
                data->completed = ! bitRetrieve(B, 4 + i);
                list_SAEJ1979_DATA_Test_append(list, data);
            }
        }
        bool isCompressionIgnition = bitRetrieve(B, 3);
        byte C = buffer[2];
        byte D = buffer[3];
        char ** names;
        char * namesCompressionIgnition[100] = {
            "EGR and/or VVT System","PM filter monitoring",
            "Exhaust Gas Sensor","- Reserved -","Boost Pressure","- Reserved -",
            "NOx/SCR Monitor","NMHC Catalyst"
        }, * namesSparkIgnition[100] = {
            "EGR System","Oxygen Sensor Heater",
            "Oxygen Sensor", "A/C Refrigerant",
            "Secondary Air System", "Evaporative System",
            "Heated Catalyst", "Catalyst"
        };
        names = isCompressionIgnition ? namesCompressionIgnition : namesSparkIgnition;
        for(int i = 0; i < 8; i++ ) {
            if ( bitRetrieve(C, i) ) {
                SAEJ1979_DATA_Test * data = (SAEJ1979_DATA_Test*)malloc(sizeof(SAEJ1979_DATA_Test));
                data->name = strdup(names[i]);
                data->completed = ! bitRetrieve(D, i);
                list_SAEJ1979_DATA_Test_append(list, data);
            }
        }
    }

    return list;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte*,
                        saej1979_data_status_this_cycle,
                        "41",
                        saej1979_data_status_iterator,
                        null
                    )

list_SAEJ1979_DATA_Test *saej1979_data_tests(final OBDIFace* iface, bool useFreezedData, bool thisDriveCycleOnly) {
    if ( thisDriveCycleOnly ) {
        return saej1979_data_tests_generic(iface, useFreezedData, saej1979_data_status_this_cycle);
    } else {
        return saej1979_data_tests_generic(iface, useFreezedData, saej1979_data_status);
    }
}
bool saej1979_data_freeze_frame(final OBDIFace* iface) {
    obd_lock(iface);
    bool result = obd_send(iface, "0102") <= 0;
    obd_clear_data(iface);
    result = ( obd_recv(iface) < 0 );
    obd_unlock(iface);
    return result;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        char**,
                        saej1979_data_fuel_system_status,
                        "03",
                        saej1979_data_fuel_system_status_iterator,
                        null
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_engine_load,
                        "04",
                        saej1979_data_engine_load_iterator,
                        SAEJ1979_DATA_ENGINE_LOAD_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_engine_coolant_temperature,
                        "05",
                        saej1979_data_engine_coolant_temperature_iterator,
                        SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_ERROR
                    )

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_FUEL_TRIM_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_get_fuel_trim_bank_generic_iterator, SAEJ1979_DATA_FUEL_TRIM_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_FUEL_TRIM_GENERIC(
    saej1979_data_short_term_fuel_trim_bank_1,"06")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_FUEL_TRIM_GENERIC(
    saej1979_data_long_term_fuel_trim_bank_1,"07")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_FUEL_TRIM_GENERIC(
    saej1979_data_short_term_fuel_trim_bank_2,"08")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_FUEL_TRIM_GENERIC(
    saej1979_data_long_term_fuel_trim_bank_2,"09")

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_fuel_pressure,
                        "0A",
                        saej1979_data_fuel_pressure_iterator,
                        SAEJ1979_DATA_FUEL_PRESSURE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_intake_manifold_pressure,
                        "0B",
                        saej1979_data_intake_manifold_pressure_iterator,
                        SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_engine_speed,
                        "0C",
                        saej1979_data_engine_speed_iterator,
                        SAEJ1979_DATA_ENGINE_SPEED_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_vehicle_speed,
                        "0D",
                        saej1979_data_vehicle_speed_iterator,
                        SAEJ1979_DATA_VEHICLE_SPEED_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_timing_advance_cycle_1,
                        "0E",
                        saej1979_data_timing_advance_cycle_1_iterator,
                        SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_intake_air_temperature,
                        "0F",
                        saej1979_data_intake_air_temperature_iterator,
                        SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_maf_air_flow_rate,
                        "10",
                        saej1979_data_maf_air_flow_rate_iterator,
                        SAEJ1979_DATA_VEHICLE_MAF_AIR_FLOW_RATE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_throttle_position,
                        "11",
                        saej1979_data_throttle_position_iterator,
                        SAEJ1979_DATA_THROTTLE_POSITION_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_SECONDARY_AIR_STATUS,
                        saej1979_data_secondary_air_status,
                        "12",
                        saej1979_data_secondary_air_status_iterator,
                        SAEJ1979_DATA_SECONDARY_AIR_STATUS_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte,
                        saej1979_data_oxygen_sensors_present_generic,
                        "13",
                        saej1979_data_oxygen_sensors_present_generic_iterator,
                        0
                    )
bool saej1979_data_oxygen_sensors_present(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte b = saej1979_data_oxygen_sensors_present_generic(iface,useFreezedData);
    int bitmask = 1 >> (sensor_i-1);
    return (bitmask & b) != 0;
}

/**
 * Service 0114 0115 0116 0117 0118 0119 011A 011B
 */
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_voltage_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_1_voltage,
                        "14"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_2_voltage,
                        "15"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_3_voltage,
                        "16"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_4_voltage,
                        "17"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_5_voltage,
                        "18"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_6_voltage,
                        "19"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_7_voltage,
                        "1A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_GENERIC(
                        saej1979_data_oxygen_sensor_8_voltage,
                        "1B"
                    )
double saej1979_data_oxygen_sensor_voltage(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_voltage(iface,useFreezedData);
        case 2: return saej1979_data_oxygen_sensor_2_voltage(iface,useFreezedData);
        case 3: return saej1979_data_oxygen_sensor_3_voltage(iface,useFreezedData);
        case 4: return saej1979_data_oxygen_sensor_4_voltage(iface,useFreezedData);
        case 5: return saej1979_data_oxygen_sensor_5_voltage(iface,useFreezedData);
        case 6: return saej1979_data_oxygen_sensor_6_voltage(iface,useFreezedData);
        case 7: return saej1979_data_oxygen_sensor_7_voltage(iface,useFreezedData);
        case 8: return saej1979_data_oxygen_sensor_8_voltage(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR;
}
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_trim_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_1_trim,
                        "14"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_2_trim,
                        "15"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_3_trim,
                        "16"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_4_trim,
                        "17"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_5_trim,
                        "18"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_6_trim,
                        "19"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_7_trim,
                        "1A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_TRIM_GENERIC(
                        saej1979_data_oxygen_sensor_8_trim,
                        "1B"
                    )
double saej1979_data_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_trim(iface,useFreezedData);
        case 2: return saej1979_data_oxygen_sensor_2_trim(iface,useFreezedData);
        case 3: return saej1979_data_oxygen_sensor_3_trim(iface,useFreezedData);
        case 4: return saej1979_data_oxygen_sensor_4_trim(iface,useFreezedData);
        case 5: return saej1979_data_oxygen_sensor_5_trim(iface,useFreezedData);
        case 6: return saej1979_data_oxygen_sensor_6_trim(iface,useFreezedData);
        case 7: return saej1979_data_oxygen_sensor_7_trim(iface,useFreezedData);
        case 8: return saej1979_data_oxygen_sensor_8_trim(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_ERROR;
}

byte saej1979_data_oxygen_sensors_present_bank1(final OBDIFace* iface,bool useFreezedData) {
    return 0xF & saej1979_data_oxygen_sensors_present_generic(iface,useFreezedData);
}
byte saej1979_data_oxygen_sensors_present_bank2(final OBDIFace* iface,bool useFreezedData) {
    return (0xF0 & saej1979_data_oxygen_sensors_present_generic(iface,useFreezedData)) >> 4;
}
char * saej1979_data_obd_standard_convert_to_string(final SAEJ1979_DATA_OBD_STANDARD standard) {
    switch(standard) {
        case SAEJ1979_DATA_OBD_STANDARD_OBD_UNKNOWN:                 return strdup("Unknown");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_II_CARB:                 return strdup("OBD-II as defined by CARB");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_EPA:                     return strdup("OBD as defined by EPA");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_AND_OBD_II:              return strdup("OBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_OBD_I:                       return strdup("OBD-I");
        case SAEJ1979_DATA_OBD_STANDARD_NOT_OBD:                     return strdup("Not OBD compliant");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD:                        return strdup("EOBD (Europe)");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_II:             return strdup("EOBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD:                return strdup("EOBD and OBD");
        case SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_AND_OBD_II:     return strdup("EOBD, OBD and OBD-II");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD:                        return strdup("JOBD (Japan)");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_OBD_II:             return strdup("JOBD and OBD II");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD:               return strdup("JOBD and EOBD");
        case SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD_AND_OBD_II:    return strdup("JOBD, EOBD, OBD II");
        case SAEJ1979_DATA_OBD_STANDARD_EMD:                         return strdup("Engine Manufacturer Diagnostics (EMD)");
        case SAEJ1979_DATA_OBD_STANDARD_EMD_ENHANCED:                return strdup("Engine Manufacturer Diagnostics Enhanced (EMD+)");
        case SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD_C:            return strdup("Heavy Duty On-Board Diagnostics (Child/Partial) (HD OBD-C)");
        case SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD:              return strdup("Heavy Duty On-Board Diagnostics (HD OBD)");
        case SAEJ1979_DATA_OBD_STANDARD_WWH_OBD:                     return strdup("World Wide Harmonized OBD (WWH OBD)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD:                     return strdup("Heavy Duty Euro Stage I without NOx control (HD EOBD-I)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_N:                   return strdup("Heavy Duty Euro OBD Start I with NOx control (HD EOBD-I N)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II:                  return strdup("Heavy Duty Euro Stage II without NOx control (HD EOBD-II)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II_N:                return strdup("Heavy Duty Euro OBD Start II with NOx control (HD EOBD-II N)");
        case SAEJ1979_DATA_OBD_STANDARD_OBDBR_1:                     return strdup("Brazil OBD Phase 1 (OBDBr-1)");
        case SAEJ1979_DATA_OBD_STANDARD_OBDBR_2:                     return strdup("Brazil OBD Phase 2 (OBDBr-2)");
        case SAEJ1979_DATA_OBD_STANDARD_KOBD:                        return strdup("Korean OBD (KOBD)");
        case SAEJ1979_DATA_OBD_STANDARD_IOBD:                        return strdup("India OBD I (IOBD I)");
        case SAEJ1979_DATA_OBD_STANDARD_IOBD_II:                     return strdup("India OBD II (IOBD II)");
        case SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_VI:                  return strdup("Heavy Duty Euro OBD Stage VI (HD EOBD-IV)");
    }
    char *res;
    asprintf(&res, "Unknown (%d)", standard);
    return res;
}
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_OBD_STANDARD,
                        saej1979_data_obd_standard,
                        "1C",
                        saej1979_data_obd_standard_iterator,
                        SAEJ1979_DATA_OBD_STANDARD_OBD_UNKNOWN
                    )
char * saej1979_data_obd_standard_as_string(final OBDIFace* iface,bool useFreezedData) {
    return saej1979_data_obd_standard_convert_to_string(saej1979_data_obd_standard(iface,useFreezedData));
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        byte,
                        saej1979_data_oxygen_sensors_present_2_generic,
                        "1D",
                        saej1979_data_oxygen_sensors_present_2_generic_iterator,
                        0
                    )
bool saej1979_data_oxygen_sensors_present_2(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte b = saej1979_data_oxygen_sensors_present_2_generic(iface,useFreezedData);

    int bitmask = 1 >> (sensor_i-1);

    return (bitmask & b) != 0;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        bool,
                        saej1979_data_powertakeoff_status,
                        "1E",
                        saej1979_data_powertakeoff_status_iterator,
                        SAEJ1979_DATA_POWERTAKEOFF_STATUS_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_seconds_since_engine_start,
                        "1F",
                        saej1979_data_seconds_since_engine_start_iterator,
                        SAEJ1979_DATA_SECONDS_SINCE_ENGINE_START_ERROR
                    )
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
char* saej1979_data_time_since_engine_start(final OBDIFace* iface, bool useFreezedData) {
    final int seconds = saej1979_data_seconds_since_engine_start(iface,useFreezedData);
    return seconds == -1 ? null : saej1979_get_seconds_to_time(seconds);
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_distance_since_mil_activated,
                        "21",
                        saej1979_data_distance_since_mil_activated_iterator,
                        SAEJ1979_DATA_DISTANCE_SINCE_MIL_ACTIVATED_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_frp_relative,
                        "22",
                        saej1979_data_frp_relative_iterator,
                        SAEJ1979_DATA_FRP_RELATIVE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_frp_widerange,
                        "23",
                        saej1979_data_frp_widerange_iterator,
                        SAEJ1979_DATA_FRP_WIDERANGE_ERROR
                    )

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_1_air_fuel_equiv_ratio,
                        "24"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_2_air_fuel_equiv_ratio,
                        "25"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_3_air_fuel_equiv_ratio,
                        "26"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_4_air_fuel_equiv_ratio,
                        "27"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_5_air_fuel_equiv_ratio,
                        "28"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_6_air_fuel_equiv_ratio,
                        "29"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_7_air_fuel_equiv_ratio,
                        "2A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_GENERIC(
                        saej1979_data_oxygen_sensor_8_air_fuel_equiv_ratio,
                        "2B"
                    )
double saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_air_fuel_equiv_ratio(iface,useFreezedData);
        case 2: return saej1979_data_oxygen_sensor_2_air_fuel_equiv_ratio(iface,useFreezedData);
        case 3: return saej1979_data_oxygen_sensor_3_air_fuel_equiv_ratio(iface,useFreezedData);
        case 4: return saej1979_data_oxygen_sensor_4_air_fuel_equiv_ratio(iface,useFreezedData);
        case 5: return saej1979_data_oxygen_sensor_5_air_fuel_equiv_ratio(iface,useFreezedData);
        case 6: return saej1979_data_oxygen_sensor_6_air_fuel_equiv_ratio(iface,useFreezedData);
        case 7: return saej1979_data_oxygen_sensor_7_air_fuel_equiv_ratio(iface,useFreezedData);
        case 8: return saej1979_data_oxygen_sensor_8_air_fuel_equiv_ratio(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR;
}
#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        double, sym, obd_request, \
                        saej1979_data_oxygen_sensor_voltage_ext_range_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_1_voltage_ext_range,
                        "24"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_2_voltage_ext_range,
                        "25"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_3_voltage_ext_range,
                        "26"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_4_voltage_ext_range,
                        "27"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_5_voltage_ext_range,
                        "28"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_6_voltage_ext_range,
                        "29"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_7_voltage_ext_range,
                        "2A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_GENERIC(
                        saej1979_data_oxygen_sensor_8_voltage_ext_range,
                        "2B"
                    )
double saej1979_data_oxygen_sensor_voltage_ext_range(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_1_voltage_ext_range(iface,useFreezedData);
        case 2: return saej1979_data_oxygen_sensor_2_voltage_ext_range(iface,useFreezedData);
        case 3: return saej1979_data_oxygen_sensor_3_voltage_ext_range(iface,useFreezedData);
        case 4: return saej1979_data_oxygen_sensor_4_voltage_ext_range(iface,useFreezedData);
        case 5: return saej1979_data_oxygen_sensor_5_voltage_ext_range(iface,useFreezedData);
        case 6: return saej1979_data_oxygen_sensor_6_voltage_ext_range(iface,useFreezedData);
        case 7: return saej1979_data_oxygen_sensor_7_voltage_ext_range(iface,useFreezedData);
        case 8: return saej1979_data_oxygen_sensor_8_voltage_ext_range(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_ERROR;
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_commanded_egr,
                        "2C",
                        saej1979_data_generic_one_byte_percentage_iterator,
                        SAEJ1979_DATA_COMMANDED_EGR_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_egr_error,
                        "2D",
                        saej1979_data_egr_error_iterator,
                        SAEJ1979_DATA_EGR_ERROR_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_commanded_evap_purge,
                        "2E",
                        saej1979_data_generic_one_byte_percentage_iterator,
                        SAEJ1979_DATA_COMMANDED_EVAP_PURGE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_fuel_tank_level_input,
                        "2F",
                        saej1979_data_generic_one_byte_percentage_iterator,
                        SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_warm_ups_since_ecu_reset,
                        "30",
                        saej1979_data_warm_ups_since_ecu_reset_iterator,
                        SAEJ1979_DATA_WARM_UPS_SINCE_ECU_RESET_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_distance_since_ecu_reset,
                        "31",
                        saej1979_data_distance_since_ecu_reset_iterator,
                        SAEJ1979_DATA_DISTANCE_SINCE_ECU_RESET_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_evap_system_vapor_pressure,
                        "32",
                        saej1979_data_evap_system_vapor_pressure_iterator,
                        SAEJ1979_DATA_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR
                    )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_baro_pressure_absolute,
                        "33",
                        saej1979_data_baro_pressure_absolute_iterator,
                        SAEJ1979_DATA_BARO_PRESSURE_ABSOLUTE_ERROR
                    )

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        int, sym, obd_request, \
                        saej1979_data_oxygen_sensor_current_generic_iterator, SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_1,
                        "34"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_2,
                        "35"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_3,
                        "36"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_4,
                        "37"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_5,
                        "38"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_6,
                        "39"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_7,
                        "3A"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_OXYGEN_SENSOR_GENERIC(
                        saej1979_data_oxygen_sensor_current_8,
                        "3B"
                    )
int saej1979_data_oxygen_sensor_current(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_oxygen_sensor_current_1(iface,useFreezedData);
        case 2: return saej1979_data_oxygen_sensor_current_2(iface,useFreezedData);
        case 3: return saej1979_data_oxygen_sensor_current_3(iface,useFreezedData);
        case 4: return saej1979_data_oxygen_sensor_current_4(iface,useFreezedData);
        case 5: return saej1979_data_oxygen_sensor_current_5(iface,useFreezedData);
        case 6: return saej1979_data_oxygen_sensor_current_6(iface,useFreezedData);
        case 7: return saej1979_data_oxygen_sensor_current_7(iface,useFreezedData);
        case 8: return saej1979_data_oxygen_sensor_current_8(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR;
}

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(sym,obd_request) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
                        int, sym, obd_request, \
                        saej1979_data_catalyst_temperature_generic_iterator, SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR \
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_1_temperature,
                        "3C"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_2_temperature,
                        "3D"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_3_temperature,
                        "3E"
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_CATALYST_TEMPERATURE_GENERIC(
                        saej1979_data_catalyst_4_temperature,
                        "3F"
                    )
int saej1979_data_catalyst_tempature_with_bank(final OBDIFace* iface, bool useFreezedData, final int bank_i, final int sensor_i) {
    if ( sensor_i == 1 ) {
        if ( bank_i == 1 ) {
            return saej1979_data_catalyst_1_temperature(iface,useFreezedData);            
        } else if ( bank_i == 2 ) {
            return saej1979_data_catalyst_2_temperature(iface,useFreezedData);        
        }
    } else if ( sensor_i == 2 ) {
        if ( bank_i == 1 ) {
            return saej1979_data_catalyst_3_temperature(iface,useFreezedData);            
        } else if ( bank_i == 2 ) {
            return saej1979_data_catalyst_4_temperature(iface,useFreezedData);        
        }
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR;
}

int saej1979_data_catalyst_tempature(final OBDIFace* iface, bool useFreezedData, final int sensor_i) {
    switch(sensor_i) {
        case 1: return saej1979_data_catalyst_1_temperature(iface,useFreezedData);
        case 2: return saej1979_data_catalyst_2_temperature(iface,useFreezedData);
        case 3: return saej1979_data_catalyst_3_temperature(iface,useFreezedData);
        case 4: return saej1979_data_catalyst_4_temperature(iface,useFreezedData);
    }
    log_msg(LOG_ERROR, "Error");
    return SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR;
}
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_ecu_voltage,
                        "42",
                        saej1979_data_ecu_voltage_iterator,
                        SAEJ1979_DATA_ECU_VOLTAGE_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_absolute_engine_load,
                        "43",
                        saej1979_data_absolute_engine_load_iterator,
                        SAEJ1979_DATA_ABSOLUTE_ENGINE_LOAD_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_commanded_air_fuel_equivalence_ratio,
                        "44",
                        saej1979_data_commanded_air_fuel_equivalence_ratio_iterator,
                        SAEJ1979_DATA_COMMANDED_AIR_FUEL_EQUIVALENCE_RATIO_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,
                        saej1979_data_relative_throttle_position,
                        "45",
                        saej1979_data_generic_one_byte_percentage_iterator,
                        SAEJ1979_DATA_RELATIVE_THROTTLE_POSITION_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_ambient_air_temperature,
                        "46",
                        saej1979_data_ambient_air_temperature_iterator,
                        SAEJ1979_DATA_AMBIENT_AIR_TEMPERATURE_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_absolute_throttle_position_B,
                        "47")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_absolute_throttle_position_C,
                        "48")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_D,
                        "49")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_E,
                        "4A")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_accelerator_pedal_position_F,
                        "4B")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_commanded_throttle_actuator,
                        "4C")
#define saej1979_data_time_run_mil_on_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_time_run_mil_on,
                        "4D",
                        saej1979_data_time_run_mil_on_iterator,
                        SAEJ1979_DATA_TIME_RUN_MIL_ON_ERROR
                    )
#define saej1979_data_time_since_trouble_codes_cleared_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,
                        saej1979_data_time_since_trouble_codes_cleared,
                        "4E",
                        saej1979_data_time_since_trouble_codes_cleared_iterator,
                        SAEJ1979_DATA_TIME_SINCE_TROUBLE_CODES_CLEARED_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_fuel_air_equiv_ratio,
                        "4F",
                        saej1979_data_max_fuel_air_equiv_ratio_iterator,
                        SAEJ1979_DATA_MAX_FUEL_AIR_EQUIV_RATIO_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_oxygen_sensor_voltage,
                        "4F",
                        saej1979_data_max_oxygen_sensor_voltage_iterator,
                        SAEJ1979_DATA_MAX_OXYGEN_SENSOR_VOLTAGE_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_oxygen_sensor_current,
                        "4F",
                        saej1979_data_max_oxygen_sensor_current_iterator,
                        SAEJ1979_DATA_MAX_OXYGEN_SENSOR_CURRENT_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_intake_manifold_absolute_pressure,
                        "4F",
                        saej1979_data_max_intake_manifold_absolute_pressure_iterator,
                        SAEJ1979_DATA_MAX_INTAKE_MANIFOLD_ABSOLUTE_PRESSURE_ERROR
                    )
#define saej1979_data_max_air_flow_rate_from_maf_sensor_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] * 10;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_max_air_flow_rate_from_maf_sensor,
                        "50",
                        saej1979_data_max_air_flow_rate_from_maf_sensor_iterator,
                        SAEJ1979_DATA_MAX_AIR_FLOW_RATE_FROM_MAF_SENSOR_ERROR
                    )

char * saej1979_data_fuel_type_convert_to_string(SAEJ1979_DATA_FUEL_TYPE t) {
    switch(t) {
        default:
        case SAEJ1979_DATA_FUEL_TYPE_NOT_AVALIABLE:                 break;
        case SAEJ1979_DATA_FUEL_TYPE_GASOLINE:                      return strdup("Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_METHANOL:                      return strdup("Methanol");
        case SAEJ1979_DATA_FUEL_TYPE_ETHANOL:                       return strdup("Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_DIESEL:                        return strdup("Diesel");
        case SAEJ1979_DATA_FUEL_TYPE_LPG:                           return strdup("LPG");
        case SAEJ1979_DATA_FUEL_TYPE_CNG:                           return strdup("CNG");
        case SAEJ1979_DATA_FUEL_TYPE_PROPANE:                       return strdup("Propane");
        case SAEJ1979_DATA_FUEL_TYPE_ELECTRIC:                      return strdup("Electric");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_GASOLINE:               return strdup("Bifuel running Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_METHANOL:               return strdup("Bifuel running Methanol");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ETHANOL:                return strdup("Bifuel running Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_LPG:                    return strdup("Bifuel running LPG");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_CNG:                    return strdup("Bifuel running CNG");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_PROPANE:                return strdup("Bifuel running Propane");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY:            return strdup("Bifuel running Electricity");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY_COMBUSTION: return strdup("Bifuel running electric and combustion engine");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_GASOLINE:               return strdup("Hybrid Gasoline");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ETHANOL:                return strdup("Hybrid Ethanol");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_DIESEL:                 return strdup("Hybrid Diesel");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC:               return strdup("Hybrid Electric");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC_COMBUSTION:    return strdup("Hybrid running electric and combustion engine");
        case SAEJ1979_DATA_FUEL_TYPE_HYBRID_REGENERATIVE:           return strdup("Hybrid Regenerative");
        case SAEJ1979_DATA_FUEL_TYPE_BIFUEL_DIESEL:                 return strdup("Bifuel runnig diesel");
    }
    return strdup("Not available");
}
#define saej1979_data_fuel_type_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0];
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        SAEJ1979_DATA_FUEL_TYPE,saej1979_data_fuel_type,
                        "51",
                        saej1979_data_fuel_type_iterator,
                        SAEJ1979_DATA_FUEL_TYPE_NOT_AVALIABLE
                    )
char * saej1979_data_fuel_type_as_string(final OBDIFace* iface, bool useFreezedData) {
    return saej1979_data_fuel_type_convert_to_string(saej1979_data_fuel_type(iface, useFreezedData));
}
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_ethanol_fuel_percent,
                        "52")
#define saej1979_data_absolute_evap_system_vapor_pressure_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data)/200.0;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        double,saej1979_data_absolute_evap_system_vapor_pressure,
                        "53",
                        saej1979_data_absolute_evap_system_vapor_pressure_iterator,
                        SAEJ1979_DATA_ABSOLUTE_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR
                    )
#define saej1979_data_relative_evap_system_vapor_pressure_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_relative_evap_system_vapor_pressure,
                        "54",
                        saej1979_data_relative_evap_system_vapor_pressure_iterator,
                        SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR
                    )

#define saej1979_data_secondary_oxygen_sensor_trim_iterator(data) \
    if ( 0 < data->size ) result = (100/128.0) * data->buffer[0] - 100;
#define saej1979_data_secondary_oxygen_sensor_trim_2_iterator(data) \
    if ( 1 < data->size ) result = (100/128.0) * data->buffer[1] - 100;

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_1,
                        "55",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_2,
                        "57",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                    
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_3,
                        "55",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )  
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_short_term_secondary_oxygen_sensor_trim_4,
                        "57",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    ) 
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_1,
                        "56",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_2,
                        "58",
                        saej1979_data_secondary_oxygen_sensor_trim_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                    
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_3,
                        "56",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )  
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_long_term_secondary_oxygen_sensor_trim_4,
                        "58",
                        saej1979_data_secondary_oxygen_sensor_trim_2_iterator,
                        SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR
                    )                                                              
int saej1979_data_short_term_secondary_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, int bank_i) {
    assert(1 <= bank_i && bank_i <= 4);
    switch(bank_i) {
        case 1: return saej1979_data_short_term_secondary_oxygen_sensor_trim_1(iface, useFreezedData);
        case 2: return saej1979_data_short_term_secondary_oxygen_sensor_trim_2(iface, useFreezedData);
        case 3: return saej1979_data_short_term_secondary_oxygen_sensor_trim_3(iface, useFreezedData);
        case 4: return saej1979_data_short_term_secondary_oxygen_sensor_trim_4(iface, useFreezedData);
    }
    return SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR;
}

int saej1979_data_long_term_secondary_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, int bank_i) {
    assert(1 <= bank_i && bank_i <= 4);
    switch(bank_i) {
        case 1: return saej1979_data_long_term_secondary_oxygen_sensor_trim_1(iface, useFreezedData);
        case 2: return saej1979_data_long_term_secondary_oxygen_sensor_trim_2(iface, useFreezedData);
        case 3: return saej1979_data_long_term_secondary_oxygen_sensor_trim_3(iface, useFreezedData);
        case 4: return saej1979_data_long_term_secondary_oxygen_sensor_trim_4(iface, useFreezedData);
    }
    return SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR;
}
#define saej1979_data_fuel_rail_absolute_pressure_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data) * 10;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                        int,saej1979_data_fuel_rail_absolute_pressure,
                        "59",
                        saej1979_data_fuel_rail_absolute_pressure_iterator,
                        SAEJ1979_DATA_FUEL_RAIL_ABSOLUTE_PRESSURE_ERROR
                    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_relative_accelerator_pedal_position,
                        "5A")
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(
                        saej1979_data_hybrid_battery_pack_remaining_life,
                        "5B")
#define saej1979_data_engine_oil_temperature_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_oil_temperature,
                    "5C",
                    saej1979_data_engine_oil_temperature_iterator,
                    SAEJ1979_DATA_ENGINE_OIL_TEMPERATURE_ERROR
                )
#define saej1979_data_fuel_injection_timing_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data) / 128.0 - 210;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    double,saej1979_data_fuel_injection_timing,
                    "5D",
                    saej1979_data_fuel_injection_timing_iterator,
                    SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR
                )
#define saej1979_data_engine_fuel_rate_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data) / 20.0;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    double,saej1979_data_engine_fuel_rate,
                    "5E",
                    saej1979_data_engine_fuel_rate_iterator,
                    SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR
                )
#define saej1979_data_engine_torque_percent_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_driver_demand_engine_percent_torque,
                    "61",
                    saej1979_data_engine_torque_percent_iterator,
                    SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_ERROR
                )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_actual_engine_percent_torque,
                    "62",
                    saej1979_data_engine_torque_percent_iterator,
                    SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_ERROR
                )
#define saej1979_data_engine_reference_torque_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_reference_torque,
                    "63",
                    saej1979_data_engine_reference_torque_iterator,
                    SAEJ1979_DATA_ENGINE_REFERENCE_TORQUE_ERROR
                )    

#define saej1979_data_engine_percent_torque_data_idle_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_percent_torque_data_idle,
                    "64",
                    saej1979_data_engine_percent_torque_data_idle_iterator,
                    SAEJ1979_DATA_ENGINE_PERCENT_TORQUE_DATA_ERROR
                ) 

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_TORQUE_DATA_I(point_i) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE( \
        int,saej1979_data_engine_percent_torque_data_point_##point_i, \
        "64", \
        saej1979_data_engine_percent_torque_data_point_##point_i##_iterator, \
        SAEJ1979_DATA_ENGINE_PERCENT_TORQUE_DATA_ERROR \
    ) 

#define saej1979_data_engine_percent_torque_data_point_1_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_TORQUE_DATA_I(1)
#define saej1979_data_engine_percent_torque_data_point_2_iterator(data) \
    if ( 2 < data->size ) result = data->buffer[2] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_TORQUE_DATA_I(2)
#define saej1979_data_engine_percent_torque_data_point_3_iterator(data) \
    if ( 3 < data->size ) result = data->buffer[3] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_TORQUE_DATA_I(3)
#define saej1979_data_engine_percent_torque_data_point_4_iterator(data) \
    if ( 4 < data->size ) result = data->buffer[4] - 125;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_TORQUE_DATA_I(4)

#define saej1979_data_maf_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_maf_sensor_present_1,
                    "66",
                    saej1979_data_maf_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_maf_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_maf_sensor_present_2,
                    "66",
                    saej1979_data_maf_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_maf_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_maf_sensor_present_1(iface, useFreezedData);
        case 2: return saej1979_data_maf_sensor_present_2(iface, useFreezedData);
    }
    return false;
}
#define saej1979_data_maf_sensor_1_iterator(data) \
    if ( 2 < data->size ) result = (data->buffer[1] * 256 + data->buffer[2])/32;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_maf_sensor_1,
                    "66",
                    saej1979_data_maf_sensor_1_iterator,
                    SAEJ1979_DATA_MAF_SENSOR_ERROR
                ) 
#define saej1979_data_maf_sensor_2_iterator(data) \
    if ( 4 < data->size ) result = (data->buffer[3] * 256 + data->buffer[4])/32;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_maf_sensor_2,
                    "66",
                    saej1979_data_maf_sensor_2_iterator,
                    SAEJ1979_DATA_MAF_SENSOR_ERROR
                )                 
int saej1979_data_maf_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_maf_sensor_1(iface, useFreezedData);
        case 2: return saej1979_data_maf_sensor_2(iface, useFreezedData);
    }
    return SAEJ1979_DATA_MAF_SENSOR_ERROR;
}

#define saej1979_data_engine_coolant_temperature_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_coolant_temperature_sensor_present_1,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_engine_coolant_temperature_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_coolant_temperature_sensor_present_2,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_engine_coolant_temperature_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_coolant_temperature_sensor_present_1(iface, useFreezedData);
        case 2: return saej1979_data_engine_coolant_temperature_sensor_present_2(iface, useFreezedData);
    }
    return false;
}
#define saej1979_data_engine_coolant_temperature_sensor_1_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_coolant_temperature_sensor_1,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_1_iterator,
                    SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR
                ) 
#define saej1979_data_engine_coolant_temperature_sensor_2_iterator(data) \
    if ( 2 < data->size ) result = data->buffer[2] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_coolant_temperature_sensor_2,
                    "67",
                    saej1979_data_engine_coolant_temperature_sensor_2_iterator,
                    SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR
                )                 
int saej1979_data_engine_coolant_temperature_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_coolant_temperature_sensor_1(iface, useFreezedData);
        case 2: return saej1979_data_engine_coolant_temperature_sensor_2(iface, useFreezedData);
    }
    return SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR;
}

#define saej1979_data_engine_intake_air_temperature_sensor_present_1_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_intake_air_temperature_sensor_present_1,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_present_1_iterator,
                    false
                )
#define saej1979_data_engine_intake_air_temperature_sensor_present_2_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_engine_intake_air_temperature_sensor_present_2,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_present_2_iterator,
                    false
                )                
bool saej1979_data_engine_intake_air_temperature_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_intake_air_temperature_sensor_present_1(iface, useFreezedData);
        case 2: return saej1979_data_engine_intake_air_temperature_sensor_present_2(iface, useFreezedData);
    }
    return false;
}
#define saej1979_data_engine_intake_air_temperature_sensor_1_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_intake_air_temperature_sensor_1,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_1_iterator,
                    SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR
                ) 
#define saej1979_data_engine_intake_air_temperature_sensor_2_iterator(data) \
    if ( 2 < data->size ) result = data->buffer[2] - 40;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_intake_air_temperature_sensor_2,
                    "68",
                    saej1979_data_engine_intake_air_temperature_sensor_2_iterator,
                    SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR
                )                 
int saej1979_data_engine_intake_air_temperature_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 2);
    switch(sensor_i) {
        case 1: return saej1979_data_engine_intake_air_temperature_sensor_1(iface, useFreezedData);
        case 2: return saej1979_data_engine_intake_air_temperature_sensor_2(iface, useFreezedData);
    }
    return SAEJ1979_DATA_ENGINE_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR;
}

#define saej1979_egt_sensor_present_byte_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0];
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte,saej1979_egt_sensor_bank_1_present_byte,
        "78",
        saej1979_egt_sensor_present_byte_iterator,
        0
    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte,saej1979_egt_sensor_bank_2_present_byte,
        "79",
        saej1979_egt_sensor_present_byte_iterator,
        0
    )
bool saej1979_egt_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    final byte b;
    if ( sensor_i <= 4 ) {
        b = saej1979_egt_sensor_bank_1_present_byte(iface, useFreezedData);
    } else {
        b = saej1979_egt_sensor_bank_2_present_byte(iface, useFreezedData);
        sensor_i -= 4;
    }
    return bitRetrieve(b, sensor_i - 1);
}
#define saej1979_egt_sensor_temperature_iterator(data) \
    if ( 8 < data->size ) result = data->buffer;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte*,saej1979_egt_sensor_temperature_bank_1,
        "78",
        saej1979_egt_sensor_temperature_iterator,
        null
    )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
        byte*,saej1979_egt_sensor_temperature_bank_2,
        "79",
        saej1979_egt_sensor_temperature_iterator,
        null
    )
int saej1979_egt_sensor_temperature(final OBDIFace* iface, bool useFreezedData, int sensor_i) {
    assert(1 <= sensor_i && sensor_i <= 8);
    byte *data;
    if ( sensor_i <= 4 ) {
        data = saej1979_egt_sensor_temperature_bank_1(iface, useFreezedData);
    } else {
        data = saej1979_egt_sensor_temperature_bank_2(iface, useFreezedData);
        sensor_i -= 4;
    }    
    if ( data == null ) {
        return SAEJ1979_EGT_SENSOR_TEMPERATURE_ERROR;
    } else {
        return (data[1 + sensor_i * 2] * 256 + data[1 + sensor_i * 2 + 1])/10 - 40;
    }
}

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_diesel_particulate_filter_temperature,
                    "7C",
                    saej1979_data_diesel_particulate_filter_temperature_iterator,
                    SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_ERROR
                )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_engine_friction_percent_torque,
                    "8E",
                    saej1979_data_engine_friction_percent_torque_iterator,
                    SAEJ1979_DATA_ENGINE_FRICTION_PERCENT_TORQUE_ERROR
                )
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    double,saej1979_data_cylinder_fuel_rate,
                    "A2",
                    saej1979_data_cylinder_fuel_rate_iterator,
                    SAEJ1979_DATA_CYLINDER_FUEL_RATE_ERROR
                )
#define saej1979_data_transmission_actual_gear_present_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_transmission_actual_gear_present,
                    "A4",
                    saej1979_data_transmission_actual_gear_present_iterator,
                    false
                )
#define saej1979_data_transmission_actual_gear_iterator(data) \
    if ( 3 < data->size ) result = (256 * data->buffer[2] + data->buffer[3]) / 1000;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_transmission_actual_gear,
                    "A4",
                    saej1979_data_transmission_actual_gear_iterator,
                    SAEJ1979_DATA_TRANSMISSION_ACTUAL_GEAR_ERROR
                )
                
#define saej1979_data_commanded_diesel_exhaust_fluid_dosing_present_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_commanded_diesel_exhaust_fluid_dosing_present,
                    "A5",
                    saej1979_data_commanded_diesel_exhaust_fluid_dosing_present_iterator,
                    false
                )
#define saej1979_data_commanded_diesel_exhaust_fluid_dosing_iterator(data) \
    if ( 1 < data->size ) result = data->buffer[1] / 2;
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    int,saej1979_data_commanded_diesel_exhaust_fluid_dosing,
                    "A5",
                    saej1979_data_commanded_diesel_exhaust_fluid_dosing_iterator,
                    SAEJ1979_DATA_COMMANDED_DIESEL_EXHAUST_FLUID_DOSING_ERROR
                )

SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    double,saej1979_data_odometer,
                    "A6",
                    saej1979_data_odometer_iterator,
                    SAEJ1979_DATA_ODOMETER_ERROR
                )

bool saej1979_data_abs_switch_present(final OBDIFace* iface, bool useFreezedData); 
bool saej1979_data_abs_switch(final OBDIFace* iface, bool useFreezedData);                 

#define saej1979_data_abs_switch_present_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 0);
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_abs_switch_present,
                    "A9",
                    saej1979_data_abs_switch_present_iterator,
                    false
                )
#define saej1979_data_abs_switch_iterator(data) \
    if ( 0 < data->size ) result = bitRetrieve(data->buffer[0], 1);                
SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(
                    bool,saej1979_data_abs_switch,
                    "A9",
                    saej1979_data_abs_switch_iterator,
                    false
                )

hashmap_void_char * _saej1979_data_data_gen_pid_map = null;
int hashmap_void_char_key_comparator(void * k1, void *k2) {
    return ! ( k1 == k2 );
}
HASHMAP_SRC(void, char)
char* saej1979_data_data_gen_pid_map_get(void *key) {
    if ( _saej1979_data_data_gen_pid_map == null ) {
        _saej1979_data_data_gen_pid_map = hashmap_void_char_new();
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_number_of_dtc, strdup("01"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_type, strdup("01"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_mil_status, strdup("01"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_status, strdup("01"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_status_this_cycle, strdup("41"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_system_status, strdup("03"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_load, strdup("04"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_coolant_temperature, strdup("05"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_pressure, strdup("0A"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_intake_manifold_pressure, strdup("0B"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_speed, strdup("0C"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_vehicle_speed, strdup("0D"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_timing_advance_cycle_1, strdup("0E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_intake_air_temperature, strdup("0F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_maf_air_flow_rate, strdup("10"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_throttle_position, strdup("11"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_secondary_air_status, strdup("12"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_oxygen_sensors_present_generic, strdup("13"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_obd_standard, strdup("1C"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_oxygen_sensors_present_2_generic, strdup("1D"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_powertakeoff_status, strdup("1E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_seconds_since_engine_start, strdup("1F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_distance_since_mil_activated, strdup("21"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_frp_relative, strdup("22"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_frp_widerange, strdup("23"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_commanded_egr, strdup("2C"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_egr_error, strdup("2D"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_commanded_evap_purge, strdup("2E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_tank_level_input, strdup("2F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_warm_ups_since_ecu_reset, strdup("30"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_distance_since_ecu_reset, strdup("31"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_evap_system_vapor_pressure, strdup("32"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_baro_pressure_absolute, strdup("33"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_ecu_voltage, strdup("42"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_absolute_engine_load, strdup("43"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_commanded_air_fuel_equivalence_ratio, strdup("44"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_relative_throttle_position, strdup("45"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_ambient_air_temperature, strdup("46"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_time_run_mil_on, strdup("4D"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_time_since_trouble_codes_cleared, strdup("4E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_max_fuel_air_equiv_ratio, strdup("4F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_max_oxygen_sensor_voltage, strdup("4F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_max_oxygen_sensor_current, strdup("4F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_max_intake_manifold_absolute_pressure, strdup("4F"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_max_air_flow_rate_from_maf_sensor, strdup("50"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_type, strdup("51"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_absolute_evap_system_vapor_pressure, strdup("53"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_relative_evap_system_vapor_pressure, strdup("54"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_short_term_secondary_oxygen_sensor_trim_1, strdup("55"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_short_term_secondary_oxygen_sensor_trim_2, strdup("57"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_short_term_secondary_oxygen_sensor_trim_3, strdup("55"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_short_term_secondary_oxygen_sensor_trim_4, strdup("57"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_long_term_secondary_oxygen_sensor_trim_1, strdup("56"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_long_term_secondary_oxygen_sensor_trim_2, strdup("58"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_long_term_secondary_oxygen_sensor_trim_3, strdup("56"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_long_term_secondary_oxygen_sensor_trim_4, strdup("58"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_rail_absolute_pressure, strdup("59"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_oil_temperature, strdup("5C"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_fuel_injection_timing, strdup("5D"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_fuel_rate, strdup("5E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_driver_demand_engine_percent_torque, strdup("61"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_actual_engine_percent_torque, strdup("62"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_reference_torque, strdup("63"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_percent_torque_data_idle, strdup("64"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_maf_sensor_present_1, strdup("66"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_maf_sensor_present_2, strdup("66"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_maf_sensor_1, strdup("66"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_maf_sensor_2, strdup("66"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_coolant_temperature_sensor_present_1, strdup("67"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_coolant_temperature_sensor_present_2, strdup("67"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_coolant_temperature_sensor_1, strdup("67"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_coolant_temperature_sensor_2, strdup("67"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_intake_air_temperature_sensor_present_1, strdup("68"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_intake_air_temperature_sensor_present_2, strdup("68"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_intake_air_temperature_sensor_1, strdup("68"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_intake_air_temperature_sensor_2, strdup("68"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_egt_sensor_bank_1_present_byte, strdup("78"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_egt_sensor_bank_2_present_byte, strdup("79"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_egt_sensor_temperature_bank_1, strdup("78"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_egt_sensor_temperature_bank_2, strdup("79"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_diesel_particulate_filter_temperature, strdup("7C"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_engine_friction_percent_torque, strdup("8E"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_cylinder_fuel_rate, strdup("A2"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_transmission_actual_gear_present, strdup("A4"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_transmission_actual_gear, strdup("A4"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_commanded_diesel_exhaust_fluid_dosing_present, strdup("A5"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_commanded_diesel_exhaust_fluid_dosing, strdup("A5"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_odometer, strdup("A6"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_abs_switch_present, strdup("A9"));
        hashmap_void_char_set(_saej1979_data_data_gen_pid_map, saej1979_data_abs_switch, strdup("A9"));
    }
    char * value = hashmap_void_char_get(_saej1979_data_data_gen_pid_map, key);
    if ( value == null ) {
        return null;
    } else {
        return strdup(value);
    }
}
