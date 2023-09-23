#ifndef __SERIAL_SAEJ1979_H
#define __SERIAL_SAEJ1979_H

/**
 * C API to interact with SAEJ1979 compliant vehicles.<br />
 * When there is multiple ECU on the line, the best possible is done regarding to the logic of the function.<br />
 * Example: for the number of dtc currently displayed we sum number of dtcs reported by each ecu.<br />
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include "com/serial/at/obd/obd.h"
#include "globals.h"
#include "buffer.h"
#include "database.h"

/**
 * Returned strings are malloc'ed
 */
char *saej1979_service_code_to_str(final unsigned char code);
char *saej1979_pid_code_to_str(final unsigned char service, final unsigned char code);
char *saej1979_decode_to_str(final unsigned char code[2]);
/**
 * For given pid number (eg 0, 1, 2, 3, 4, ...) show if it is currently supported by ECU<br />
 * Service 0100
 */
bool saej1979_is_pid_supported(final OBDIFACE iface, final int pid);
/**
 * Service 0101
 * Get status (on/off) of MIL/CEL
 */
bool saej1979_get_current_mil_status(final OBDIFACE iface);
bool saej1979_get_current_mil_status_from_fetch(final OBDIFACE iface);
/**
 * Get number of dtcs currently flagged in ECU
 */
int saej1979_get_current_number_of_dtc(final OBDIFACE iface);
int saej1979_get_current_number_of_dtc_from_fetch(final OBDIFACE iface);
typedef enum {
    SAEJ1979_ENGINE_TYPE_SPARK_IGNITION,
    SAEJ1979_ENGINE_TYPE_COMPRESSION_IGNITION,
    SAEJ1979_ENGINE_TYPE_UNKNOWN
} SAEJ1979_ENGINE_TYPES;
final static const char SAEJ1979_ENGINE_TYPES_STR[][100] = {
    {"Spark Ignition"}, 
    {"Compression ignition"},
    {"Unknown"}
};
#define SAEJ1979_NUMBER_OF_DTC_ERROR -1
/**
 * Spark ignition, Compression ignition (eg Diesel)
 */
SAEJ1979_ENGINE_TYPES saej1979_get_current_engine_type(final OBDIFACE iface);
/**
 * @return malloc'ed string describing current engine type
 */
char* saej1979_get_current_engine_type_as_string(final OBDIFACE iface);
/**
 * Service 0102
 * Freeze the current frame
 */
bool saej1979_freeze_dtc(final OBDIFACE iface);
/**
 * Service 0103
 * memory must be there freed
 * @return 2 strings describing the fuel system or null on error
 */
char** saej1979_get_current_fuel_system_status(final OBDIFACE iface);
#define SAEJ1979_FUEL_SYSTEM_STATUS_MOTOR_OFF_ERROR null
#define SAEJ1979_FUEL_SYSTEM_STATUS_MOTOR_OFF 0
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_MOTOR_OFF "The motor is off"
#define SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP 1
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_INSUFFICIENT_ENGINE_TEMP "Open loop due to insufficient engine temperature"
#define SAEJ1979_FUEL_SYSTEM_STATUS_CLOSED_LOOP_USING_FEEDBACK 2
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_USING_FEEDBACK "Close loop, using oxygen sensor feedback to determine fuel mix"
#define SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION 4
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_ENGINE_LOAD_OR_DECELERATION "Open loop due to engine load OR fuel cut due to deceleration"
#define SAEJ1979_FUEL_SYSTEM_STATUS_OPEN_LOOP_SYSTEM_FAILURE 8
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_OPEN_LOOP_SYSTEM_FAILURE "Open loop due to system failure"
#define SAEJ1979_FUEL_SYSTEM_STATUS_CLOSED_LOOP_WITH_FEEDBACK_FAULT 16
#define SAEJ1979_FUEL_SYSTEM_STATUS_STR_CLOSED_LOOP_WITH_FEEDBACK_FAULT "Closed loop, using at least one oxygen sensor but there is a fault in the feedback system"
char * saej1979_fuel_system_status_code_to_str(final int fuel_system_status_code);
/**
 * Service 0104
 * @return calculated engine load (%) [0;100]
 */
double saej1979_get_current_engine_load(final OBDIFACE iface);
#define SAEJ1979_ENGINE_LOAD_ERROR -1
/**
 * Service 0105
 * @return °C [-40;215] or INTEGER_MIN in case of error
 */
int saej1979_get_current_engine_coolant_temperature(final OBDIFACE iface);
#define SAEJ1979_ENGINE_COOLANT_TEMPERATURE_MIN -40
#define SAEJ1979_ENGINE_COOLANT_TEMPERATURE_MAX 215
#define SAEJ1979_ENGINE_COOLANT_TEMPERATURE_ERROR INT_MIN
/**
 * Service 0106-0109
 * Fuel trim status
 */
/**
 * Fuel trim balance
 * -100 reduce fuel: too rich, 99 add fuel: too lean
 * @return percent [-100;99.2]
 */
int saej1979_get_fuel_trim_bank_generic(final OBDIFACE iface, final const char *cmd);
int saej1979_get_current_short_term_fuel_trim_bank_1(final OBDIFACE iface);
int saej1979_get_current_long_term_fuel_trim_bank_1(final OBDIFACE iface);
int saej1979_get_current_short_term_fuel_trim_bank_2(final OBDIFACE iface);
int saej1979_get_current_long_term_fuel_trim_bank_2(final OBDIFACE iface);
#define SAEJ1979_FUEL_TRIM_MIN -100
#define SAEJ1979_FUEL_TRIM_MAX 99.2
#define SAEJ1979_FUEL_TRIM_ERROR INT_MIN
/**
 * Service 010A
 * @return kPa [0;765] (gauge)
 */
int saej1979_get_current_fuel_pressure(final OBDIFACE iface);
#define SAEJ1979_FUEL_PRESSURE_MIN 0
#define SAEJ1979_FUEL_PRESSURE_MAX 765
#define SAEJ1979_FUEL_PRESSURE_ERROR -1
/**
 * Service 010B
 * @return kPa [0;255] (absolute)
 */
int saej1979_get_current_intake_manifold_pressure(final OBDIFACE iface);
#define SAEJ1979_INTAKE_MANIFOLD_PRESSURE_MIN 0
#define SAEJ1979_INTAKE_MANIFOLD_PRESSURE_MAX 255
#define SAEJ1979_INTAKE_MANIFOLD_PRESSURE_ERROR -1
/**
 * Service 010C
 * @return rpm [0;16,383.75] or -1 on error
 */
double saej1979_get_current_engine_speed(final OBDIFACE iface);
#define SAEJ1979_ENGINE_SPEED_MIN 0
#define SAEJ1979_ENGINE_SPEED_MAX 16383.75
#define SAEJ1979_ENGINE_SPEED_ERROR -1
/**
 * Service 010D
 * @return km/h [0;255] or -1 on error
 */
int saej1979_get_current_vehicle_speed(final OBDIFACE iface);
#define SAEJ1979_VEHICULE_SPEED_MIN 0
#define SAEJ1979_VEHICULE_SPEED_MAX 255
#define SAEJ1979_VEHICULE_SPEED_ERROR -1
/**
 * Service 010E
 * Timing Advance (Cyl. #1)
 * @return [-64;63.5] ° before TDC or -65 in case of error
 */
double saej1979_get_current_timing_advance_cycle_1(final OBDIFACE iface);
#define SAEJ1979_TIMING_ADVANCE_CYCLE_1_MIN -64
#define SAEJ1979_TIMING_ADVANCE_CYCLE_1_MAX 63.5
#define SAEJ1979_TIMING_ADVANCE_CYCLE_1_ERROR -65
/**
 * Service 010F
 * @return °C [-40;215]
 */
int saej1979_get_current_intake_air_temperature(final OBDIFACE iface);
#define SAEJ1979_INTAKE_AIR_TEMPERATURE_MIN -40
#define SAEJ1979_INTAKE_AIR_TEMPERATURE_MAX 215
#define SAEJ1979_INTAKE_AIR_TEMPERATURE_ERROR INT_MIN
/**
 * Service 0110
 * @return g/s [0;655.35]
 */
double saej1979_get_current_maf_air_flow_rate(final OBDIFACE iface);
#define SAEJ1979_VEHICULE_MAF_AIR_FLOW_RATE_MIN 0
#define SAEJ1979_VEHICULE_MAF_AIR_FLOW_RATE_MAX 655.35
#define SAEJ1979_VEHICULE_MAF_AIR_FLOW_RATE_ERROR -1
/**
 * Service 0111
 * @return throttle position (%) [0;100]
 */
double saej1979_get_current_throttle_position(final OBDIFACE iface);
#define SAEJ1979_OBD_STANDARD int
#define SAEJ1979_THROTTLE_POSITION_ERROR -1
/**
 * Service 0112
 */
typedef int SAEJ1979_SECONDARY_AIR_STATUS;
SAEJ1979_SECONDARY_AIR_STATUS saej1979_get_current_secondary_air_status(final OBDIFACE iface);
#define SAEJ1979_SECONDARY_AIR_STATUS_UPSTREAM 1
#define SAEJ1979_SECONDARY_AIR_STATUS_DOWNSTREAM 2
#define SAEJ1979_SECONDARY_AIR_STATUS_OUTSIDE_OR_OFF 4
#define SAEJ1979_SECONDARY_AIR_STATUS_PUMP 8
#define SAEJ1979_SECONDARY_AIR_STATUS_ERROR 0x80
char * saej1979_secondary_air_status_to_string(SAEJ1979_SECONDARY_AIR_STATUS data);
/**
 * Service 0112
 */
byte saej1979_current_oxygen_sensors_present_bank1(final OBDIFACE iface);
byte saej1979_current_oxygen_sensors_present_bank2(final OBDIFACE iface);
byte saej1979_current_oxygen_sensors_present_generic(final OBDIFACE iface);
char* saej1979_current_oxygen_sensors_present_to_string(byte b);
/**
 * Service 011C
 * @return obd standard
 */
SAEJ1979_OBD_STANDARD saej1979_get_current_obd_standard(final OBDIFACE iface);
#define SAEJ1979_OBD_STANDARD_OBD_UNKNOWN -1
#define SAEJ1979_OBD_STANDARD_OBD_II_CARB 1
#define SAEJ1979_OBD_STANDARD_OBD_EPA 2
#define SAEJ1979_OBD_STANDARD_OBD_AND_OBD_II 3
#define SAEJ1979_OBD_STANDARD_OBD_I 4
#define SAEJ1979_OBD_STANDARD_NOT_OBD 5
#define SAEJ1979_OBD_STANDARD_EOBD 6
#define SAEJ1979_OBD_STANDARD_EOBD_AND_OBD_II 7
#define SAEJ1979_OBD_STANDARD_EOBD_AND_OBD 8
#define SAEJ1979_OBD_STANDARD_EOBD_AND_OBD_AND_OBD_II 9
#define SAEJ1979_OBD_STANDARD_JOBD 10
#define SAEJ1979_OBD_STANDARD_JOBD_AND_OBD_II 11
#define SAEJ1979_OBD_STANDARD_JOBD_AND_EOBD 12
#define SAEJ1979_OBD_STANDARD_JOBD_AND_EOBD_AND_OBD_II 13
#define SAEJ1979_OBD_STANDARD_EMD 17
#define SAEJ1979_OBD_STANDARD_EMD_ENHANCED 18
#define SAEJ1979_OBD_STANDARD_HEAVY_DUTY_OBD_C 19
#define SAEJ1979_OBD_STANDARD_HEAVY_DUTY_OBD 20
#define SAEJ1979_OBD_STANDARD_WWH_OBD 21
#define SAEJ1979_OBD_STANDARD_HD_EOBD 23
#define SAEJ1979_OBD_STANDARD_HD_EOBD_N 24
#define SAEJ1979_OBD_STANDARD_HD_EOBD_II 25
#define SAEJ1979_OBD_STANDARD_HD_EOBD_II_N 26
#define SAEJ1979_OBD_STANDARD_OBDBR_1 28
#define SAEJ1979_OBD_STANDARD_OBDBR_2 29
#define SAEJ1979_OBD_STANDARD_KOBD 30
#define SAEJ1979_OBD_STANDARD_IOBD 31
#define SAEJ1979_OBD_STANDARD_IOBD_II 32
#define SAEJ1979_OBD_STANDARD_HD_EOBD_VI 33
char * saej1979_get_current_obd_standard_str(final OBDIFACE iface);
/**
 * Service 011F
 * @return seconds since engine start or -1 on error
 */
int saej1979_get_current_seconds_since_engine_start(final OBDIFACE iface);
#define SAEJ1979_SECONDS_SINCE_ENGINE_START_ERROR -1
/**
 * @return string describing time since or null on error
 */
char * saej1979_get_current_time_since_engine_start(final OBDIFACE iface);
/**
 * Service 0121
 * @return km or -1 on error
 */
int saej1979_get_current_distance_since_mil_activated(final OBDIFACE iface);
#define SAEJ1979_DISTANCE_SINCE_MIL_ACTIVATED_ERROR -1
/**
 * Service 0122
 * fuel rail pressure relative to manifold vacuum
 * @return [0;5177.265] kPa or -1 on error
 */
double saej1979_get_current_frp_relative(final OBDIFACE iface);
#define SAEJ1979_FRP_RELATIVE -1
/**
 * Service 0123
 * fuel rail pressure (gauge), wide range
 * @return [0;655350] kPa or -1 on error
 */
int saej1979_get_current_frp_widerange(final OBDIFACE iface);
#define SAEJ1979_FRP_WIDERANGE_ERROR -1
/**
 * Service 012C
 * Commanded EGR : percent of exhaust gas that is routed back in the intake
 * @return [0;100] or -1 on error
 */
double saej1979_get_current_commanded_egr(final OBDIFACE iface);
#define SAEJ1979_COMMANDED_EGR_ERROR -1
/**
 * Service 012D
 * @return [-100;99.2] or -1
 */
double saej1979_get_current_egr_error(final OBDIFACE iface);
#define SAEJ1979_EGR_ERROR_ERROR -1
/**
 * Service 012E
 */
double saej1979_get_current_commanded_evap_purge(final OBDIFACE iface);
#define SAEJ1979_COMMANDED_EVAP_PURGE_ERROR -1
/**
 * Service 012F
 */
double saej1979_get_current_fuel_tank_level_input(final OBDIFACE iface);
#define SAEJ1979_FUEL_TANK_LEVEL_INPUT_MIN 0
#define SAEJ1979_FUEL_TANK_LEVEL_INPUT_MAX 100
#define SAEJ1979_FUEL_TANK_LEVEL_INPUT_ERROR -1
/**
 * Service 0130
 * warm ups since ECU reset
 * @return count or -1 on error
 */
int saej1979_get_current_warm_ups_since_ecu_reset(final OBDIFACE iface);
#define SAEJ1979_WARM_UPS_SINCE_ECU_RESET_ERROR -1
/**
 * Service 0131
 * Distance since ECU reset
 * @return km or -1 on error
 */
int saej1979_get_current_distance_since_ecu_reset(final OBDIFACE iface);
#define SAEJ1979_DISTANCE_SINCE_ECU_RESET_ERROR -1
/**
 * Service 0132
 * Evap System Vapor Pressure
 * @return Pa or -1 on error
 */
double saej1979_get_current_evap_system_vapor_pressure(final OBDIFACE iface);
#define SAEJ1979_EVAP_SYSTEM_VAPOR_PRESSURE_ERROR -1
/**
 * Service 0133
 * @return kPa or -1 on error
 */
int saej1979_get_current_baro_pressure_absolute(final OBDIFACE iface);
#define SAEJ1979_BARO_PRESSURE_ABSOLUTE_ERROR -1
/**
 * Service 0140
 * @return V or -1 on error
 */
double saej1979_get_current_ecu_voltage(final OBDIFACE iface);
#define SAEJ1979_ECU_VOLTAGE_ERROR -1
/**
 * Service 04
 */
bool saej1979_clear_dtc_and_stored_values(final OBDIFACE iface);
/**
 * Convenience include
 */
#include "saej1979_show_dtc.h"

#endif
