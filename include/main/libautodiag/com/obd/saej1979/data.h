#ifndef __SAEJ1979_DATA_H
#define __SAEJ1979_DATA_H

#include "libautodiag/lib.h"
#include "libautodiag/string.h"
#include "libautodiag/log.h"
#include "libautodiag/buffer.h"
#include "libautodiag/com/obd/obd.h"
#include "saej1979.h"

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(type,symbol,data_pid_requested,iterator,errorValue) type symbol(final OBDIFace* iface, bool useFreezedData) { \
    if ( useFreezedData ) { \
        SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,"02" data_pid_requested,iterator,errorValue,ecu->obd_service.freeze_frame_data) \
    } else { \
        SAEJ1979_GENERATE_OBD_REQUEST_ITERATE_BODY(type,"01" data_pid_requested,iterator,errorValue,ecu->obd_service.current_data)  \
    } \
}

#define SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE_PERCENTAGE(sym,data_pid_requested) \
    SAEJ1979_DATA_GENERATE_OBD_REQUEST_ITERATE(double,sym,data_pid_requested,saej1979_data_generic_one_byte_percentage_iterator,SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR)

char *saej1979_data_pid_code_to_str(final unsigned char pid);

unsigned short saej1979_data_buffer_get_short(final Buffer* buffer);
unsigned int saej1979_data_buffer_get_uint(final Buffer* buffer);

#define saej1979_data_generic_one_byte_percentage_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0] * (100.0/255);
#define SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN 0
#define SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX 100
#define SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR -1

typedef struct {
    char * name;
    bool completed;
} SAEJ1979_DATA_Test;
LIST_DEFINE_WITH_MEMBERS_AUTO(SAEJ1979_DATA_Test)
/**
 * Get a list of avaliable tests on the car and the completeness of those
 * @param thisDriveCycleOnly use result of tests occurred this drive cycle
 */
SAEJ1979_DATA_Test_list *saej1979_data_tests(final OBDIFace* iface, bool useFreezedData, bool thisDriveCycleOnly);

/**
 * For given pid number (eg 0, 1, 2, 3, 4, ...) show if it is currently supported by ECU<br />
 * Service 0*00
 */
bool saej1979_data_is_pid_supported(final OBDIFace* iface, bool useFreezedData, final int pid);
/**
 * Service 0*01
 * Get status (on/off) of MIL/CEL
 */

#define saej1979_data_number_of_dtc_iterator(data) { \
    if ( 0 < data->size ) { \
        if ( SAEJ1979_DATA_NUMBER_OF_DTC_ERROR == result ) { \
            result = 0; \
        } \
        result += (data->buffer[0] & 0x7F) ; \
    } \
}
#define SAEJ1979_DATA_NUMBER_OF_DTC_ERROR -1

#define saej1979_data_engine_type_iterator(data) \
    if ( 1 < data->size ) \
        result = bitRetrieve(data->buffer[1],3) ? \
            SAEJ1979_DATA_ENGINE_TYPE_COMPRESSION_IGNITION : SAEJ1979_DATA_ENGINE_TYPE_SPARK_IGNITION;
typedef enum {
    SAEJ1979_DATA_ENGINE_TYPE_SPARK_IGNITION,
    SAEJ1979_DATA_ENGINE_TYPE_COMPRESSION_IGNITION,
    SAEJ1979_DATA_ENGINE_TYPE_UNKNOWN
} SAEJ1979_DATA_ENGINE_TYPES;
final static const char SAEJ1979_DATA_ENGINE_TYPES_STR[][100] = {
    {"Spark Ignition"}, 
    {"Compression ignition"},
    {"Unknown"}
};

#define saej1979_data_mil_status_iterator(data) \
    if ( 0 < data->size ) \
        result |= bitRetrieve(data->buffer[0],7);
bool saej1979_data_mil_status(final OBDIFace* iface, bool useFreezedData);/**
 * Get number of dtcs currently flagged in ECU
 */
int saej1979_data_number_of_dtc(final OBDIFace* iface, bool useFreezedData);

/**
 * Spark ignition, Compression ignition (eg Diesel)
 */
SAEJ1979_DATA_ENGINE_TYPES saej1979_data_engine_type(final OBDIFace* iface, bool useFreezedData);
/**
 * @return malloc'ed string describing current engine type
 */
char* saej1979_data_engine_type_as_string(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*02
 * Freeze the current frame
 */
bool saej1979_data_freeze_frame(final OBDIFace* iface);
/**
 * Service 0*03
 * memory must be there freed
 * @return 2 strings describing the fuel system or null on error
 */
#define saej1979_data_fuel_system_status_iterator(data) { \
    if ( 1 < data->size ) { \
        result = (char**)malloc(sizeof(char*) * 2); \
        result[0] = saej1979_data_fuel_system_status_code_to_str((int)data->buffer[0]); \
        result[1] = saej1979_data_fuel_system_status_code_to_str((int)data->buffer[1]); \
    } \
}
char * saej1979_data_fuel_system_status_code_to_str(final int fuel_system_status_code);
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_MOTOR_OFF_ERROR null
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_MOTOR_OFF 0
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_MOTOR_OFF "The motor is off"
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP 1
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP "Open loop due to insufficient engine temperature"
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_USING_FEEDBACK 2
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_USING_FEEDBACK "Close loop, using oxygen sensor feedback to determine fuel mix"
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION 4
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION "Open loop due to engine load OR fuel cut due to deceleration"
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_OPEN_LOOP_SYSTEM_FAILURE 8
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_SYSTEM_FAILURE "Open loop due to system failure"
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_CLOSED_LOOP_WITH_FEEDBACK_FAULT 16
#define SAEJ1979_DATA_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_WITH_FEEDBACK_FAULT "Closed loop, using at least one oxygen sensor but there is a fault in the feedback system"
char** saej1979_data_fuel_system_status(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*04
 * @return calculated engine load (%) [0;100]
 */
#define saej1979_data_engine_load_iterator(data) \
    if ( 0 < data->size ) \
        result = ((unsigned char)data->buffer[0]) / 2.55;

#define SAEJ1979_DATA_ENGINE_LOAD_ERROR -1
double saej1979_data_engine_load(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*05
 * @return °C [-40;215] or INTEGER_MIN in case of error
 */
#define saej1979_data_engine_coolant_temperature_iterator(data) \
    if ( 0 < data->size ) \
        result = ((unsigned char)data->buffer[0]) - 40;

#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_MAX 215
#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_ERROR INT_MIN
int saej1979_data_engine_coolant_temperature(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*06-0*09
 * Fuel trim status
 * Fuel trim balance
 * -100 reduce fuel: too rich, 99 add fuel: too lean
 * @return percent [-100;99.2]
 */
#define saej1979_data_get_fuel_trim_bank_generic_iterator(data) \
    if ( 0 < data->size ) \
        result = ((unsigned char)data->buffer[0])/1.28 - 100; 

#define SAEJ1979_DATA_FUEL_TRIM_MIN -100
#define SAEJ1979_DATA_FUEL_TRIM_MAX 99.2
#define SAEJ1979_DATA_FUEL_TRIM_ERROR INT_MIN
int saej1979_data_short_term_fuel_trim_bank_1(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_long_term_fuel_trim_bank_1(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_short_term_fuel_trim_bank_2(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_long_term_fuel_trim_bank_2(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0A
 * @return kPa [0;765] (gauge)
 */
#define saej1979_data_fuel_pressure_iterator(data) \
    if ( 0 < data->size ) \
        result = ((unsigned char)data->buffer[0]) * 3; 

#define SAEJ1979_DATA_FUEL_PRESSURE_MIN 0
#define SAEJ1979_DATA_FUEL_PRESSURE_MAX 765
#define SAEJ1979_DATA_FUEL_PRESSURE_ERROR -1
int saej1979_data_fuel_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0B
 * @return kPa [0;255] (absolute)
 */
#define saej1979_data_intake_manifold_pressure_iterator(data) \
    if ( 0 < data->size ) \
        result = (unsigned char)data->buffer[0]; 

#define SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MIN 0
#define SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_MAX 255
#define SAEJ1979_DATA_INTAKE_MANIFOLD_PRESSURE_ERROR -1
int saej1979_data_intake_manifold_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0C
 * @return rpm [0;16,383.75] or -1 on error
 */
#define saej1979_data_engine_speed_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) / 4.0;

#define SAEJ1979_DATA_ENGINE_SPEED_MIN 0
#define SAEJ1979_DATA_ENGINE_SPEED_MAX 16383.75
#define SAEJ1979_DATA_ENGINE_SPEED_ERROR -1
double saej1979_data_engine_speed(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0D
 * @return km/h [0;255] or -1 on error
 */
#define saej1979_data_vehicle_speed_iterator(data) \
    if ( 0 < data->size ) \
        result = (int)((unsigned char)data->buffer[0]); 

#define SAEJ1979_DATA_VEHICULE_SPEED_MIN 0
#define SAEJ1979_DATA_VEHICULE_SPEED_MAX 255
#define SAEJ1979_DATA_VEHICULE_SPEED_ERROR -1
int saej1979_data_vehicle_speed(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0E
 * Timing Advance (Cyl. #1)
 * @return [-64;63.5] ° before TDC or -65 in case of error
 */
#define saej1979_data_timing_advance_cycle_1_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) / 2.0 - 64;

#define SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MIN -64
#define SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_MAX 63.5
#define SAEJ1979_DATA_TIMING_ADVANCE_CYCLE_1_ERROR -65
double saej1979_data_timing_advance_cycle_1(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*0F
 * @return °C [-40;215]
 */
#define saej1979_data_intake_air_temperature_iterator(data) \
    if ( 0 < data->size ) \
        result = (unsigned char)data->buffer[0];

#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_MAX 215
#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_ERROR INT_MIN
int saej1979_data_intake_air_temperature(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*10
 * @return g/s [0;655.35]
 */
#define saej1979_data_maf_air_flow_rate_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) / 100.0;

#define SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_MIN 0
#define SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_MAX 655.35
#define SAEJ1979_DATA_VEHICULE_MAF_AIR_FLOW_RATE_ERROR -1
double saej1979_data_maf_air_flow_rate(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*11
 * @return throttle position (%) [0;100]
 */
#define saej1979_data_throttle_position_iterator(data) \
    if ( 0 < data->size ) \
        result = ((unsigned char)data->buffer[0]) / 2.55; 

#define SAEJ1979_DATA_THROTTLE_POSITION_ERROR -1
double saej1979_data_throttle_position(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*12
 */
#define saej1979_data_secondary_air_status_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0]; 

typedef int SAEJ1979_DATA_SECONDARY_AIR_STATUS;
char * saej1979_data_secondary_air_status_to_string(SAEJ1979_DATA_SECONDARY_AIR_STATUS data);
#define SAEJ1979_DATA_SECONDARY_AIR_STATUS_UPSTREAM 1
#define SAEJ1979_DATA_SECONDARY_AIR_STATUS_DOWNSTREAM 2
#define SAEJ1979_DATA_SECONDARY_AIR_STATUS_OUTSIDE_OR_OFF 4
#define SAEJ1979_DATA_SECONDARY_AIR_STATUS_PUMP 8
#define SAEJ1979_DATA_SECONDARY_AIR_STATUS_ERROR 0x80
SAEJ1979_DATA_SECONDARY_AIR_STATUS saej1979_data_secondary_air_status(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*13
 * In the spec sensors are numbered as sensor 1-4 bank 1, sensors 1-4 bank 2
 *  we use uniformity sensors are 1 to 8
 */
#define saej1979_data_oxygen_sensors_present_generic_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0]; 
bool saej1979_data_oxygen_sensors_present(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
/**
 * Service 0*14 0*15 0*16 0*17 0*18 0*19 0*1A 0*1B
 * @return V
 */
#define saej1979_data_oxygen_sensor_voltage_generic_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0] / 200.0; 

#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_MIN 0
#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_MAX 1.275
#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_ERROR -1
#define saej1979_data_oxygen_sensor_trim_generic_iterator(data) { \
    if ( 1 < data->size ) { \
        if ( data->buffer[1] != 0xFF ) { \
            result = data->buffer[1] * (100.0/128) - 100; \
        } \
    } \
}
#define SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_MIN -100
#define SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_MAX 99.2
#define SAEJ1979_DATA_OXYGEN_SENSOR_TRIM_ERROR -101
double saej1979_data_oxygen_sensor_voltage(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
double saej1979_data_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
/**
 * Service 0*1C
 * @return obd standard
 */
typedef enum {
    SAEJ1979_DATA_OBD_STANDARD_OBD_UNKNOWN,
    SAEJ1979_DATA_OBD_STANDARD_OBD_II_CARB,
    SAEJ1979_DATA_OBD_STANDARD_OBD_EPA,
    SAEJ1979_DATA_OBD_STANDARD_OBD_AND_OBD_II,
    SAEJ1979_DATA_OBD_STANDARD_OBD_I,
    SAEJ1979_DATA_OBD_STANDARD_NOT_OBD,
    SAEJ1979_DATA_OBD_STANDARD_EOBD,
    SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_II,
    SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD,
    SAEJ1979_DATA_OBD_STANDARD_EOBD_AND_OBD_AND_OBD_II,
    SAEJ1979_DATA_OBD_STANDARD_JOBD,
    SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_OBD_II,
    SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD,
    SAEJ1979_DATA_OBD_STANDARD_JOBD_AND_EOBD_AND_OBD_II,
    SAEJ1979_DATA_OBD_STANDARD_EMD,
    SAEJ1979_DATA_OBD_STANDARD_EMD_ENHANCED,
    SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD_C,
    SAEJ1979_DATA_OBD_STANDARD_HEAVY_DUTY_OBD,
    SAEJ1979_DATA_OBD_STANDARD_WWH_OBD,
    SAEJ1979_DATA_OBD_STANDARD_HD_EOBD,
    SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_N,
    SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II,
    SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_II_N,
    SAEJ1979_DATA_OBD_STANDARD_OBDBR_1,
    SAEJ1979_DATA_OBD_STANDARD_OBDBR_2,
    SAEJ1979_DATA_OBD_STANDARD_KOBD,
    SAEJ1979_DATA_OBD_STANDARD_IOBD,
    SAEJ1979_DATA_OBD_STANDARD_IOBD_II,
    SAEJ1979_DATA_OBD_STANDARD_HD_EOBD_VI
} SAEJ1979_DATA_OBD_STANDARD;
#define saej1979_data_obd_standard_iterator(data) \
    if ( 0 < data->size ) \
        result = (SAEJ1979_DATA_OBD_STANDARD)data->buffer[0]; 

char * saej1979_data_obd_standard_convert_to_string(final SAEJ1979_DATA_OBD_STANDARD standard);
SAEJ1979_DATA_OBD_STANDARD saej1979_data_obd_standard(final OBDIFace* iface, bool useFreezedData);
char * saej1979_data_obd_standard_as_string(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*1D
 * alternative way to detect oxygen sensors they may be divided in 4 banks
 */
#define saej1979_data_oxygen_sensors_present_2_generic_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0]; 
bool saej1979_data_oxygen_sensors_present_2(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
/**
 * Service 0*1E
 */
#define saej1979_data_powertakeoff_status_iterator(data) \
    if ( 0 < data->size ) \
        result = bitRetrieve(data->buffer[0],0);

#define SAEJ1979_DATA_POWERTAKEOFF_STATUS_ERROR false
bool saej1979_data_powertakeoff_status(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*1F
 * @return seconds since engine start or -1 on error
 */
#define saej1979_data_seconds_since_engine_start_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data);

#define SAEJ1979_DATA_SECONDS_SINCE_ENGINE_START_ERROR -1
int saej1979_data_seconds_since_engine_start(final OBDIFace* iface, bool useFreezedData);
/**
 * @return string describing time since or null on error
 */
char * saej1979_data_time_since_engine_start(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*21
 * @return km or -1 on error
 */
#define saej1979_data_distance_since_mil_activated_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data);

#define SAEJ1979_DATA_DISTANCE_SINCE_MIL_ACTIVATED_ERROR -1
int saej1979_data_distance_since_mil_activated(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*22
 * fuel rail pressure relative to manifold vacuum
 * @return [0;5177.265] kPa or -1 on error
 */
#define saej1979_data_frp_relative_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * 0.079; 

#define SAEJ1979_DATA_FRP_RELATIVE_ERROR -1
#define SAEJ1979_DATA_FRP_RELATIVE_MIN   0
#define SAEJ1979_DATA_FRP_RELATIVE_MAX   5177.265
double saej1979_data_frp_relative(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*23
 * fuel rail pressure (gauge), wide range
 * @return [0;655350] kPa or -1 on error
 */
#define saej1979_data_frp_widerange_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * 10;

#define SAEJ1979_DATA_FRP_WIDERANGE_ERROR -1
int saej1979_data_frp_widerange(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*24 0*25 0*26 0*27 0*28 0*29 0*2A 0*2B
 * @return ratio V
 */
#define saej1979_data_oxygen_sensor_air_fuel_equiv_ratio_generic_iterator(data) \
    if ( 3 < data->size ) \
        result = (2.0/65536) * saej1979_data_buffer_get_short(data);

#define SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_MIN 0
#define SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_MAX 2
#define SAEJ1979_DATA_OXYGEN_SENSOR_AIR_FUEL_EQUIV_RATIO_ERROR -1
#define saej1979_data_oxygen_sensor_voltage_ext_range_generic_iterator(data) { \
    if ( 3 < data->size ) { \
        result = (8.0/65536) * (data->buffer[2] * 256 + data->buffer[3]); \
    } \
}
#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_MIN 0
#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_MAX 8
#define SAEJ1979_DATA_OXYGEN_SENSOR_VOLTAGE_EXT_RANGE_ERROR -1
double saej1979_data_oxygen_sensor_air_fuel_equiv_ratio(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
double saej1979_data_oxygen_sensor_voltage_ext_range(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
/**
 * Service 0*2C
 * Commanded EGR : percent of exhaust gas that is routed back in the intake
 * @return [0;100] or -1 on error
 */
#define SAEJ1979_DATA_COMMANDED_EGR_ERROR -1
double saej1979_data_commanded_egr(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*2D
 * @return [-100;99.2] or -1
 */
#define saej1979_data_egr_error_iterator(data) \
    if ( 0 < data->size ) \
        result = (unsigned char)(data->buffer[0]) * (100.0/128) - 100; 

#define SAEJ1979_DATA_EGR_ERROR_ERROR -1
double saej1979_data_egr_error(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*2E
 */
#define SAEJ1979_DATA_COMMANDED_EVAP_PURGE_ERROR -1
double saej1979_data_commanded_evap_purge(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*2F
 */
#define SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MIN SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN
#define SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_MAX SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX
#define SAEJ1979_DATA_FUEL_TANK_LEVEL_INPUT_ERROR SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR
double saej1979_data_fuel_tank_level_input(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*30
 * warm ups since ECU reset
 * @return count or -1 on error
 */
#define saej1979_data_warm_ups_since_ecu_reset_iterator(data) \
    if ( 0 < data->size ) \
        result = (unsigned char)(data->buffer[0]);

#define SAEJ1979_DATA_WARM_UPS_SINCE_ECU_RESET_ERROR -1
int saej1979_data_warm_ups_since_ecu_reset(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*31
 * Distance since ECU reset
 * @return km or -1 on error
 */
#define saej1979_data_distance_since_ecu_reset_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data); 

#define SAEJ1979_DATA_DISTANCE_SINCE_ECU_RESET_ERROR -1
int saej1979_data_distance_since_ecu_reset(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*32
 * Evap System Vapor Pressure
 * @return Pa or -1 on error
 */
#define saej1979_data_evap_system_vapor_pressure_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * 0.25;
#define SAEJ1979_DATA_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR -1
double saej1979_data_evap_system_vapor_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*33
 * @return kPa or -1 on error
 */
#define saej1979_data_baro_pressure_absolute_iterator(data) { \
    if ( 0 < data->size ) { \
        result = (unsigned char)(data->buffer[0]); \
    } \
}
#define SAEJ1979_DATA_BARO_PRESSURE_ABSOLUTE_ERROR -1
int saej1979_data_baro_pressure_absolute(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*34 0*35 0*36 0*37 0*38 0*39 0*3A 0*3B
 * @return mA
 */
#define saej1979_data_oxygen_sensor_current_generic_iterator(data) { \
    if ( 3 < data->size ) { \
        result = (data->buffer[2] * 256 + data->buffer[3]) / 256.0 - 128; \
    } \
}
#define SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_MIN -128
#define SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_MAX 128
#define SAEJ1979_DATA_OXYGEN_SENSOR_CURRENT_ERROR -129
int saej1979_data_oxygen_sensor_current(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
/**
 * Service 0*3C 0*3D 0*3E 0*3F
 * @return °C
 */
#define saej1979_data_catalyst_temperature_generic_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) / 10.0 - 40;

#define SAEJ1979_DATA_CATALYST_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_CATALYST_TEMPERATURE_MAX 6513.5
#define SAEJ1979_DATA_CATALYST_TEMPERATURE_ERROR -41
int saej1979_data_catalyst_tempature(final OBDIFace* iface, bool useFreezedData, final int sensor_i);
int saej1979_data_catalyst_tempature_with_bank(final OBDIFace* iface, bool useFreezedData, final int bank_i, final int sensor_i);
/**
 * Service 0*41
 */
/**
 * Service 0*42 - control module voltage (ECU)
 * @return V or -1 on error
 */
#define saej1979_data_ecu_voltage_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * 0.001; 

#define SAEJ1979_DATA_ECU_VOLTAGE_ERROR -1
double saej1979_data_ecu_voltage(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*43
 */
#define saej1979_data_absolute_engine_load_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * ( 100.0/255 );

#define SAEJ1979_DATA_ABSOLUTE_ENGINE_LOAD_MIN 0
#define SAEJ1979_DATA_ABSOLUTE_ENGINE_LOAD_MAX 25700
#define SAEJ1979_DATA_ABSOLUTE_ENGINE_LOAD_ERROR -1
double saej1979_data_absolute_engine_load(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*44
 * ratio
 */
#define saej1979_data_commanded_air_fuel_equivalence_ratio_iterator(data) \
    if ( 1 < data->size ) \
        result = saej1979_data_buffer_get_short(data) * ( 2.0/65536 );

#define SAEJ1979_DATA_COMMANDED_AIR_FUEL_EQUIVALENCE_RATIO_MIN 0
#define SAEJ1979_DATA_COMMANDED_AIR_FUEL_EQUIVALENCE_RATIO_MAX 2
#define SAEJ1979_DATA_COMMANDED_AIR_FUEL_EQUIVALENCE_RATIO_ERROR -1
double saej1979_data_commanded_air_fuel_equivalence_ratio(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*45
 * %
 */
#define SAEJ1979_DATA_RELATIVE_THROTTLE_POSITION_MIN SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MIN
#define SAEJ1979_DATA_RELATIVE_THROTTLE_POSITION_MAX SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_MAX
#define SAEJ1979_DATA_RELATIVE_THROTTLE_POSITION_ERROR SAEJ1979_DATA_GENERIC_ONE_BYTE_PERCENTAGE_ERROR
double saej1979_data_relative_throttle_position(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*46
 * °C
 */
#define saej1979_data_ambient_air_temperature_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0] - 40;

#define SAEJ1979_DATA_AMBIENT_AIR_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_AMBIENT_AIR_TEMPERATURE_MAX 215
#define SAEJ1979_DATA_AMBIENT_AIR_TEMPERATURE_ERROR -41
int saej1979_data_ambient_air_temperature(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*47
 * @return %
 */
double saej1979_data_absolute_throttle_position_B(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*48
 * @return %
 */
double saej1979_data_absolute_throttle_position_C(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*49
 * @return %
 */
double saej1979_data_accelerator_pedal_position_D(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4A
 * @return %
 */
double saej1979_data_accelerator_pedal_position_E(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4B
 * @return %
 */
double saej1979_data_accelerator_pedal_position_F(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4C
 * @return %
 */
double saej1979_data_commanded_throttle_actuator(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4D
 * @return minutes
 */
#define SAEJ1979_DATA_TIME_RUN_MIL_ON_ERROR -1
int saej1979_data_time_run_mil_on(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4E
 * @return minutes
 */
#define SAEJ1979_DATA_TIME_SINCE_TROUBLE_CODES_CLEARED_ERROR -1
int saej1979_data_time_since_trouble_codes_cleared(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*4F
 * ratio V mA kPa
 */
#define saej1979_data_max_fuel_air_equiv_ratio_iterator(data) \
    if ( 0 < data->size ) \
        result = data->buffer[0];
#define SAEJ1979_DATA_MAX_FUEL_AIR_EQUIV_RATIO_ERROR -1
#define saej1979_data_max_oxygen_sensor_voltage_iterator(data) \
    if ( 1 < data->size ) \
        result = data->buffer[1];
#define SAEJ1979_DATA_MAX_OXYGEN_SENSOR_VOLTAGE_ERROR -1
#define saej1979_data_max_oxygen_sensor_current_iterator(data) \
    if ( 2 < data->size ) \
        result = data->buffer[2];
#define SAEJ1979_DATA_MAX_OXYGEN_SENSOR_CURRENT_ERROR -1
#define saej1979_data_max_intake_manifold_absolute_pressure_iterator(data) \
    if ( 3 < data->size ) \
        result = data->buffer[3];
#define SAEJ1979_DATA_MAX_INTAKE_MANIFOLD_ABSOLUTE_PRESSURE_ERROR -1
int saej1979_data_max_fuel_air_equiv_ratio(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_max_oxygen_sensor_voltage(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_max_oxygen_sensor_current(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_max_intake_manifold_absolute_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*50
 * g/s
 */
#define SAEJ1979_DATA_MAX_AIR_FLOW_RATE_FROM_MAF_SENSOR_MIN 0
#define SAEJ1979_DATA_MAX_AIR_FLOW_RATE_FROM_MAF_SENSOR_MAX 2550
#define SAEJ1979_DATA_MAX_AIR_FLOW_RATE_FROM_MAF_SENSOR_ERROR -1
int saej1979_data_max_air_flow_rate_from_maf_sensor(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*51
 */
typedef enum {
    SAEJ1979_DATA_FUEL_TYPE_NOT_AVALIABLE, SAEJ1979_DATA_FUEL_TYPE_GASOLINE,
    SAEJ1979_DATA_FUEL_TYPE_METHANOL, SAEJ1979_DATA_FUEL_TYPE_ETHANOL,
    SAEJ1979_DATA_FUEL_TYPE_DIESEL, SAEJ1979_DATA_FUEL_TYPE_LPG, SAEJ1979_DATA_FUEL_TYPE_CNG,
    SAEJ1979_DATA_FUEL_TYPE_PROPANE, SAEJ1979_DATA_FUEL_TYPE_ELECTRIC, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_GASOLINE,
    SAEJ1979_DATA_FUEL_TYPE_BIFUEL_METHANOL, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ETHANOL,
    SAEJ1979_DATA_FUEL_TYPE_BIFUEL_LPG, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_CNG, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_PROPANE,
    SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_ELECTRICITY_COMBUSTION,
    SAEJ1979_DATA_FUEL_TYPE_HYBRID_GASOLINE, SAEJ1979_DATA_FUEL_TYPE_HYBRID_ETHANOL,
    SAEJ1979_DATA_FUEL_TYPE_HYBRID_DIESEL, SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC,
    SAEJ1979_DATA_FUEL_TYPE_HYBRID_ELECTRIC_COMBUSTION,
    SAEJ1979_DATA_FUEL_TYPE_HYBRID_REGENERATIVE, SAEJ1979_DATA_FUEL_TYPE_BIFUEL_DIESEL
} SAEJ1979_DATA_FUEL_TYPE;
char * saej1979_data_fuel_type_as_string(final OBDIFace* iface, bool useFreezedData);
SAEJ1979_DATA_FUEL_TYPE saej1979_data_fuel_type(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*52
 * @return %
 */
double saej1979_data_ethanol_fuel_percent(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*53
 * kPa
 */
#define SAEJ1979_DATA_ABSOLUTE_EVAP_SYSTEM_VAPOR_PRESSURE_MIN 0
#define SAEJ1979_DATA_ABSOLUTE_EVAP_SYSTEM_VAPOR_PRESSURE_MAX 327.675
#define SAEJ1979_DATA_ABSOLUTE_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR -1
double saej1979_data_absolute_evap_system_vapor_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*54
 * Pa
 */
#define SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_MIN -32768
#define SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_MAX 32767
#define SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR SAEJ1979_DATA_RELATIVE_EVAP_SYSTEM_VAPOR_PRESSURE_MIN-1
int saej1979_data_relative_evap_system_vapor_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*55 0*56 0*57 0*58
 * @return [-100;99.2] %
 */
#define SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_ERROR -101
#define SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_MIN -100
#define SAEJ1979_DATA_SECONDARY_OXYGEN_SENSOR_TRIM_MAX 99.2
int saej1979_data_short_term_secondary_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, int bank_i);
int saej1979_data_long_term_secondary_oxygen_sensor_trim(final OBDIFace* iface, bool useFreezedData, int bank_i);
/**
 * Service 0*59
 * kPa
 */
#define SAEJ1979_DATA_FUEL_RAIL_ABSOLUTE_PRESSURE_MIN 0
#define SAEJ1979_DATA_FUEL_RAIL_ABSOLUTE_PRESSURE_MAX 655350
#define SAEJ1979_DATA_FUEL_RAIL_ABSOLUTE_PRESSURE_ERROR -1
int saej1979_data_fuel_rail_absolute_pressure(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5A
 * @return %
 */
double saej1979_data_relative_accelerator_pedal_position(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5B
 * @return %
 */
double saej1979_data_hybrid_battery_pack_remaining_life(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5C
 * °C
 */
#define SAEJ1979_DATA_ENGINE_OIL_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_ENGINE_OIL_TEMPERATURE_MAX 210
#define SAEJ1979_DATA_ENGINE_OIL_TEMPERATURE_ERROR SAEJ1979_DATA_ENGINE_OIL_TEMPERATURE_MIN-1
int saej1979_data_engine_oil_temperature(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5D
 * °
 */
#define SAEJ1979_DATA_FUEL_INJECTION_TIMING_MIN -210
#define SAEJ1979_DATA_FUEL_INJECTION_TIMING_MAX 301.992
#define SAEJ1979_DATA_FUEL_INJECTION_TIMING_ERROR SAEJ1979_DATA_FUEL_INJECTION_TIMING_MIN-1
double saej1979_data_fuel_injection_timing(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5E
 * L/h
 */
#define SAEJ1979_DATA_ENGINE_FUEL_RATE_MIN 0
#define SAEJ1979_DATA_ENGINE_FUEL_RATE_MAX 3212.75
#define SAEJ1979_DATA_ENGINE_FUEL_RATE_ERROR -1
double saej1979_data_engine_fuel_rate(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*5F
 */
/**
 * Service 0*61 0*62
 * @return %
 */
#define SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_MIN -125
#define SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_MAX 130
#define SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_ERROR SAEJ1979_DATA_ENGINE_TORQUE_PERCENT_MIN-1
int saej1979_data_actual_engine_percent_torque(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_driver_demand_engine_percent_torque(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*63
 * N.m
 */
#define SAEJ1979_DATA_ENGINE_REFERENCE_TORQUE_MIN 0
#define SAEJ1979_DATA_ENGINE_REFERENCE_TORQUE_MAX 65535
#define SAEJ1979_DATA_ENGINE_REFERENCE_TORQUE_ERROR -1
int saej1979_data_engine_reference_torque(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*64
 */
#define SAEJ1979_DATA_ENGINE_PERCENT_TORQUE_DATA_ERROR -126
#define SAEJ1979_DATA_ENGINE_PERCENT_TORQUE_DATA_MIN -125
#define SAEJ1979_DATA_ENGINE_PERCENT_TORQUE_DATA_MAX 130
int saej1979_data_engine_percent_torque_data_idle(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_engine_percent_torque_data_point_1(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_engine_percent_torque_data_point_2(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_engine_percent_torque_data_point_3(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_engine_percent_torque_data_point_4(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*65
 * @return grams/sec
 */
#define SAEJ1979_DATA_MAF_SENSOR_ERROR -1
bool saej1979_data_maf_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i);
int saej1979_data_maf_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i);
/**
 * Service 0*66
 * @return °C
 */
#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_ERROR -41
#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_MIN   -40
#define SAEJ1979_DATA_ENGINE_COOLANT_TEMPERATURE_SENSOR_MAX   215
int saej1979_data_engine_coolant_temperature_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i);
bool saej1979_data_engine_coolant_temperature_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i);
/**
 * Service 0*67
 * @return °C
 */
#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_SENSOR_ERROR       -41
#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_SENSOR_MIN         -40
#define SAEJ1979_DATA_INTAKE_AIR_TEMPERATURE_SENSOR_MAX         215
int saej1979_data_intake_air_temperature_sensor(final OBDIFace* iface, bool useFreezedData, int sensor_i);
bool saej1979_data_engine_intake_air_temperature_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i);
/**
 * Service 0*68
 */
/**
 * Service 0*69
 */
/**
 * Service 0*6A
 */
/**
 * Service 0*6B
 */
/**
 * Service 0*6C
 */
/**
 * Service 0*6D
 */
/**
 * Service 0*6E
 */
/**
 * Service 0*6F
 */
/**
 * Service 0*70
 */
/**
 * Service 0*71
 */
/**
 * Service 0*72
 */
/**
 * Service 0*73
 */
/**
 * Service 0*74
 */
/**
 * Service 0*75
 */
/**
 * Service 0*76
 */
/**
 * Service 0*77
 */
/**
 * Service 0*78 0*79
 * @return °C
 */
#define SAEJ1979_EGT_SENSOR_TEMPERATURE_ERROR -41
#define SAEJ1979_EGT_SENSOR_TEMPERATURE_MIN   -40
#define SAEJ1979_EGT_SENSOR_TEMPERATURE_MAX   6513
bool saej1979_egt_sensor_present(final OBDIFace* iface, bool useFreezedData, int sensor_i);
int saej1979_egt_sensor_temperature(final OBDIFace* iface, bool useFreezedData, int sensor_i);
/**
 * Service 0*7A
 */
/**
 * Service 0*7B
 */
/**
 * Service 0*7C
 * °C
 */
#define saej1979_data_diesel_particulate_filter_temperature_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data) / 10.0 - 40;
#define SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_MIN -40
#define SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_MAX (((1 >> 16) - 1)/10 - 40)
#define SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_ERROR SAEJ1979_DATA_DIESEL_PARTICULATE_FILTER_TEMPERATURE_MIN-1
int saej1979_data_diesel_particulate_filter_temperature(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*7D
 */
/**
 * Service 0*7E
 */
/**
 * Service 0*7F
 */
/**
 * Service 0*81
 */
/**
 * Service 0*82
 */
/**
 * Service 0*83
 */
/**
 * Service 0*84
 */
/**
 * Service 0*85
 */
/**
 * Service 0*86
 */
/**
 * Service 0*87
 */
/**
 * Service 0*88
 */
/**
 * Service 0*89
 */
/**
 * Service 0*8A
 */
/**
 * Service 0*8B
 */
/**
 * Service 0*8C
 */
/**
 * Service 0*8D
 */
/**
 * Service 0*8E
 * %
 */
#define saej1979_data_engine_friction_percent_torque_iterator(data) \
    if ( 0 < data->size ) result = data->buffer[0] - 125;
#define SAEJ1979_DATA_ENGINE_FRICTION_PERCENT_TORQUE_MIN -125
#define SAEJ1979_DATA_ENGINE_FRICTION_PERCENT_TORQUE_MAX 130
#define SAEJ1979_DATA_ENGINE_FRICTION_PERCENT_TORQUE_ERROR SAEJ1979_DATA_ENGINE_FRICTION_PERCENT_TORQUE_MIN-1
int saej1979_data_engine_friction_percent_torque(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*8F
 */
/**
 * Service 0*90
 */
/**
 * Service 0*91
 */
/**
 * Service 0*92
 */
/**
 * Service 0*93
 */
/**
 * Service 0*94
 */
/**
 * Service 0*95
 */
/**
 * Service 0*96
 */
/**
 * Service 0*97
 */
/**
 * Service 0*98
 */
/**
 * Service 0*99
 */
/**
 * Service 0*9A
 */
/**
 * Service 0*9B
 */
/**
 * Service 0*9C
 */
/**
 * Service 0*9D
 */
/**
 * Service 0*9E
 */
/**
 * Service 0*9F
 */
/**
 * Service 0*A1
 */
/**
 * Service 0*A2
 * mg/stroke
 */
#define saej1979_data_cylinder_fuel_rate_iterator(data) \
    if ( 1 < data->size ) result = saej1979_data_buffer_get_short(data)/32.0;
#define SAEJ1979_DATA_CYLINDER_FUEL_RATE_MIN 0
#define SAEJ1979_DATA_CYLINDER_FUEL_RATE_MAX 2047.96875
#define SAEJ1979_DATA_CYLINDER_FUEL_RATE_ERROR -1
double saej1979_data_cylinder_fuel_rate(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*A3
 */
/**
 * Service 0*A4
 * @return ratio
 */
#define SAEJ1979_DATA_TRANSMISSION_ACTUAL_GEAR_ERROR    -1
#define SAEJ1979_DATA_TRANSMISSION_ACTUAL_GEAR_MIN      0
#define SAEJ1979_DATA_TRANSMISSION_ACTUAL_GEAR_MAX      65535
bool saej1979_data_transmission_actual_gear_present(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_transmission_actual_gear(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*A5
 * @return %
 */
#define SAEJ1979_DATA_COMMANDED_DIESEL_EXHAUST_FLUID_DOSING_ERROR   -1
#define SAEJ1979_DATA_COMMANDED_DIESEL_EXHAUST_FLUID_DOSING_MIN     0
#define SAEJ1979_DATA_COMMANDED_DIESEL_EXHAUST_FLUID_DOSING_MAX     127
bool saej1979_data_commanded_diesel_exhaust_fluid_dosing_present(final OBDIFace* iface, bool useFreezedData);
int saej1979_data_commanded_diesel_exhaust_fluid_dosing(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*A6
 * km
 */
#define saej1979_data_odometer_iterator(data) \
    if ( 3 < data->size ) result = saej1979_data_buffer_get_uint(data) / 10.0;
#define SAEJ1979_DATA_ODOMETER_MIN 0
#define SAEJ1979_DATA_ODOMETER_MAX 429296729.5
#define SAEJ1979_DATA_ODOMETER_ERROR SAEJ1979_DATA_ODOMETER_MAX+1
double saej1979_data_odometer(final OBDIFace* iface, bool useFreezedData);
/**
 * Service 0*A7
 */
/**
 * Service 0*A8
 */
/**
 * Service 0*A9
 */
bool saej1979_data_abs_switch_present(final OBDIFace* iface, bool useFreezedData); 
bool saej1979_data_abs_switch(final OBDIFace* iface, bool useFreezedData); 
/**
 * Service 0*AA
 */
/**
 * Service 0*AB
 */
/**
 * Service 0*AC
 */
/**
 * Service 0*AD
 */
/**
 * Service 0*AE
 */
/**
 * Service 0*AF
 */
/**
 * Service 0*B0
 */
/**
 * Service 0*B1
 */
/**
 * Service 0*B2
 */
/**
 * Service 0*B3
 */
/**
 * Service 0*B4
 */
/**
 * Service 0*B5
 */
/**
 * Service 0*B6
 */
/**
 * Service 0*B7
 */
/**
 * Service 0*B8
 */
/**
 * Service 0*B9
 */
/**
 * Service 0*BA
 */
/**
 * Service 0*BB
 */
/**
 * Service 0*BC
 */
/**
 * Service 0*BD
 */
/**
 * Service 0*BE
 */
/**
 * Service 0*BF
 */
/**
 * Service 0*C1
 */
/**
 * Service 0*C2
 */
/**
 * Service 0*C3
 */
/**
 * Service 0*C4
 */

#endif
